﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/coroutine.hxx>
#include <lofty/defer_to_scope_end.hxx>
#include <lofty/numeric.hxx>
#include "coroutine-scheduler.hxx"

#if LOFTY_HOST_API_POSIX
   #if LOFTY_HOST_API_DARWIN
      #define _XOPEN_SOURCE
   #endif
   #include <errno.h> // EINTR errno
   #include <signal.h> // SIGSTKSZ
   #include <ucontext.h>
   #if LOFTY_HOST_API_BSD
      #include <sys/types.h>
      #include <sys/event.h>
      #include <sys/time.h>
   #elif LOFTY_HOST_API_LINUX
      #include <sys/epoll.h>
      #include <sys/timerfd.h>
   #endif
#endif
#ifdef COMPLEMAKE_USING_VALGRIND
   #include <valgrind/valgrind.h>
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

class coroutine::impl : public noncopyable {
private:
   friend class coroutine;

public:
   /*! Constructor

   @param main_fn
      Initial value for inner_main_fn.
   */
   impl(_std::function<void ()> main_fn) :
#if LOFTY_HOST_API_POSIX
      stack(SIGSTKSZ),
#elif LOFTY_HOST_API_WIN32
      fiber_(nullptr),
#endif
#ifdef COMPLEMAKE_USING_VALGRIND
      valgrind_stack_id(VALGRIND_STACK_REGISTER(
         stack.get(), static_cast<std::int8_t *>(stack.get()) + stack.size()
      )),
#endif
      pending_x_type(exception::common_type::none),
      inner_main_fn(_std::move(main_fn)) {
#if LOFTY_HOST_API_POSIX
      // TODO: use ::mprotect() to setup a guard page for the stack.
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
      if (::getcontext(&uctx) < 0) {
         exception::throw_os_error();
      }
      uctx.uc_stack.ss_sp = static_cast<char *>(stack.get());
      uctx.uc_stack.ss_size = stack.size();
      uctx.uc_link = nullptr;
      ::makecontext(&uctx, reinterpret_cast<void (*)()>(&outer_main), 1, this);
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
#elif LOFTY_HOST_API_WIN32
      fiber_ = ::CreateFiber(0, &outer_main, this);
#endif
   }

   //! Destructor.
   ~impl() {
#ifdef COMPLEMAKE_USING_VALGRIND
      VALGRIND_STACK_DEREGISTER(valgrind_stack_id);
#endif
#if LOFTY_HOST_API_WIN32
      if (fiber_) {
         ::DeleteFiber(fiber_);
      }
#endif
   }

#if LOFTY_HOST_API_WIN32
   /*! Returns the internal fiber pointer.

   @return
      Pointer to the coroutine’s fiber.
   */
   void * fiber() {
      return fiber_;
   }
#endif

   /*! Injects the requested type of exception in the coroutine.

   @param this_pimpl
      Shared pointer to *this.
   @param x_type
      Type of exception to inject.
   */
   void inject_exception(_std::shared_ptr<impl> const & this_pimpl, exception::common_type x_type) {
      LOFTY_TRACE_FUNC(this, this_pimpl, x_type);

      /* Avoid interrupting the coroutine if there’s already a pending interruption (expected_x_type != none).
      This is not meant to prevent multiple concurrent interruptions (@see interruption-points); this is
      analogous to lofty::thread::interrupt() not trying to prevent multiple concurrent interruptions. In this
      scenario, the compare-and-swap below would succeed, but the coroutine might terminate before
      find_coroutine_to_activate() got to running it (and it would, eventually, since we call add_ready() for
      that), which would be bad. */
      auto expected_x_type = exception::common_type::none;
      if (pending_x_type.compare_exchange_strong(expected_x_type, x_type.base())) {
         /* Mark this coroutine as ready, so it will be scheduler before the scheduler tries to wait for it to
         be unblocked. */
         // TODO: sanity check to avoid scheduling a coroutine twice!
         this_thread::coroutine_scheduler()->add_ready(this_pimpl);
      }
   }

   /*! Called right after each time the coroutine resumes execution and on each interruption point defined by
   this_coroutine::interruption_point(), this will throw an exception of the type specified by
   pending_x_type. */
   void interruption_point() {
      /* This load/store is multithread-safe: the coroutine can only be executing on one thread at a time, and
      the “if” condition being true means that coroutine::interrupt() is preventing other threads from
      changing pending_x_type until we reset it to none. */
      auto x_type = pending_x_type.load();
      if (x_type != exception::common_type::none) {
         pending_x_type.store(exception::common_type::none/*, _std::memory_order_relaxed*/);
         exception::throw_common_type(x_type, 0, 0);
      }
   }

   /*! Returns a pointer to the coroutine’s coroutine_local_storage object.

   @return
      Pointer to the coroutine’s crls member.
   */
   _pvt::coroutine_local_storage * local_storage_ptr() {
      return &crls;
   }

#if LOFTY_HOST_API_POSIX
   /*! Returns a pointer to the coroutine’s context.

   @return
      Pointer to the context.
   */
   ::ucontext_t * ucontext_ptr() {
      return &uctx;
   }
#endif

private:
   /*! Lower-level wrapper for the coroutine function passed to coroutine::coroutine().

   @param p
      this.
   */
   static void
#if LOFTY_HOST_API_WIN32
      WINAPI
#endif
   outer_main(void * p);

private:
#if LOFTY_HOST_API_POSIX
   //! Context for the coroutine.
   ::ucontext_t uctx;
   //! Pointer to the memory chunk used as stack.
   memory::pages_ptr stack;
#elif LOFTY_HOST_API_WIN32
   //! Fiber for the coroutine.
   void * fiber_;
#else
   #error "TODO: HOST_API"
#endif
#ifdef COMPLEMAKE_USING_VALGRIND
   //! Identifier assigned by Valgrind to this coroutine’s stack.
   unsigned valgrind_stack_id;
#endif
   /*! Every time the coroutine is scheduled or returns from an interruption point, this is checked for
   pending exceptions to be injected. */
   _std::atomic<exception::common_type::enum_type> pending_x_type;
   //! Function to be executed in the coroutine.
   _std::function<void ()> inner_main_fn;
   //! Local storage for the coroutine.
   _pvt::coroutine_local_storage crls;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

coroutine::coroutine() {
}
/*explicit*/ coroutine::coroutine(_std::function<void ()> main_fn) :
   pimpl(_std::make_shared<impl>(_std::move(main_fn))) {
   this_thread::attach_coroutine_scheduler()->add_ready(pimpl);
}

coroutine::~coroutine() {
}

coroutine::id_type coroutine::id() const {
   return reinterpret_cast<id_type>(pimpl.get());
}

void coroutine::interrupt() {
   LOFTY_TRACE_FUNC(this);

   pimpl->inject_exception(pimpl, exception::common_type::execution_interruption);
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

void to_text_ostream<coroutine>::set_format(str const & format) {
   LOFTY_TRACE_FUNC(this, format);

   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void to_text_ostream<coroutine>::write(coroutine const & src, io::text::ostream * dst) {
   LOFTY_TRACE_FUNC(this/*, src*/, dst);

   dst->write(LOFTY_SL("CRID:"));
   if (auto id = src.id()) {
      to_text_ostream<decltype(id)>::write(id, dst);
   } else {
      dst->write(LOFTY_SL("-"));
   }
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

thread_local_value<_std::shared_ptr<coroutine::impl>> coroutine::scheduler::active_coro_pimpl;
#if LOFTY_HOST_API_POSIX
thread_local_value< ::ucontext_t *> coroutine::scheduler::default_return_uctx /*= nullptr*/;
#elif LOFTY_HOST_API_WIN32
thread_local_value<void *> coroutine::scheduler::return_fiber /*= nullptr*/;
#endif

coroutine::scheduler::scheduler() :
#if LOFTY_HOST_API_BSD
   kqueue_fd(::kqueue()),
#elif LOFTY_HOST_API_LINUX
   epoll_fd(::epoll_create1(EPOLL_CLOEXEC)),
#elif LOFTY_HOST_API_WIN32
   iocp_fd(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0)),
   timer_thread_handle(nullptr),
   stop_thread_timer(false),
#endif
   interruption_reason_x_type(exception::common_type::none) {
#if LOFTY_HOST_API_BSD
   if (!kqueue_fd) {
      exception::throw_os_error();
   }
   /* Note that at this point there’s no hack that will ensure a fork()/exec() from another thread won’t leak
   the file descriptor. That’s the whole point of NetBSD’s kqueue1(). */
   kqueue_fd.set_close_on_exec(true);
#elif LOFTY_HOST_API_LINUX
   if (!epoll_fd) {
      exception::throw_os_error();
   }
#elif LOFTY_HOST_API_WIN32
   if (!iocp_fd) {
      exception::throw_os_error();
   }
#endif
}

coroutine::scheduler::~scheduler() {
   // TODO: verify that ready_coros_queue and coros_blocked_by_fd (and coros_blocked_by_timer_ke…) are empty.
#if LOFTY_HOST_API_WIN32
   if (timer_thread_handle) {
      stop_thread_timer.store(true);
      // Wake the thread up one last time to let it know that it’s over.
      arm_timer(0);
      ::WaitForSingleObject(timer_thread_handle, INFINITE);
      ::CloseHandle(timer_thread_handle);
   }
#endif
}

void coroutine::scheduler::add_ready(_std::shared_ptr<impl> coro_pimpl) {
   LOFTY_TRACE_FUNC(this, coro_pimpl);

//   _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
   ready_coros_queue.push_back(_std::move(coro_pimpl));
}

#if LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
void coroutine::scheduler::arm_timer(time_duration_t millisecs) const {
   /* Since setting the timeout to 0 disables the timer, we’ll set it to the smallest delay possible instead.
   The resolution of the timer is much greater than milliseconds, so the requested sleep duration will be
   essentially honored. */
   #if LOFTY_HOST_API_LINUX
      ::itimerspec sleep_end;
      if (millisecs == 0) {
         // See comment above.
         sleep_end.it_value.tv_sec  = 0;
         sleep_end.it_value.tv_nsec = 1;
      } else {
         sleep_end.it_value.tv_sec  =  millisecs / 1000;
         sleep_end.it_value.tv_nsec = (millisecs % 1000) * 1000000;
      }
      sleep_end.it_interval.tv_sec  = 0;
      sleep_end.it_interval.tv_nsec = 0;
      if (::timerfd_settime(timer_fd.get(), 0, &sleep_end, nullptr) < 0) {
         exception::throw_os_error();
      }
   #elif LOFTY_HOST_API_WIN32
      ::LARGE_INTEGER nanosec_hundreds;
      // Set a relative time (negative) to make the time counting monotonic.
      if (millisecs == 0) {
         // See comment in the beginning of this method.
         nanosec_hundreds.QuadPart = -1;
      } else {
         nanosec_hundreds.QuadPart = static_cast<std::int64_t>(
            static_cast<std::uint64_t>(millisecs)
         ) * -10000;
      }
      if (!::SetWaitableTimer(timer_fd.get(), &nanosec_hundreds, 0, nullptr, nullptr, false)) {
         exception::throw_os_error();
      }
   #endif
}

void coroutine::scheduler::arm_timer_for_next_sleep_end() const {
   if (coros_blocked_by_timer_fd) {
      // Calculate the time at which the earliest sleep end should occur.
      time_point_t now = current_time(), sleep_end = coros_blocked_by_timer_fd.front().key;
      time_duration_t sleep;
      if (now < sleep_end) {
         sleep = static_cast<time_duration_t>(sleep_end - now);
      } else {
         // The timer should’ve already fired by now.
         sleep = 0;
      }
      arm_timer(sleep);
   } else {
      // Stop the timer.
   #if LOFTY_HOST_API_LINUX
      ::itimerspec sleep_end;
      memory::clear(&sleep_end);
      if (::timerfd_settime(timer_fd.get(), 0, &sleep_end, nullptr) < 0) {
         exception::throw_os_error();
      }
   #elif LOFTY_HOST_API_WIN32
      if (!::CancelWaitableTimer(timer_fd.get())) {
         exception::throw_os_error();
      }
   #endif
   }
}
#endif

void coroutine::scheduler::block_active_for_ms(unsigned millisecs) {
   LOFTY_TRACE_FUNC(this, millisecs);

   // TODO: handle millisecs == 0 as a timer-less yield.

#if LOFTY_HOST_API_BSD
   impl * coro_pimpl = active_coro_pimpl.get();
   struct ::kevent ke;
   ke.ident = reinterpret_cast<std::uintptr_t>(coro_pimpl);
   // Use EV_ONESHOT to avoid waking up multiple threads for the same fd becoming ready.
   ke.flags = EV_ADD | EV_ONESHOT;
   ke.filter = EVFILT_TIMER;
#if LOFTY_HOST_API_DARWIN
   ke.fflags = NOTE_USECONDS;
   ke.data = millisecs * 1000u;
#else
   ke.data = millisecs;
#endif
   ::timespec ts = { 0, 0 };
   if (::kevent(kqueue_fd.get(), &ke, 1, nullptr, 0, &ts) < 0) {
      exception::throw_os_error();
   }
   // Deactivate the current coroutine and find one to activate instead.
   {
//      _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
      coros_blocked_by_timer_ke.add_or_assign(ke.ident, _std::move(active_coro_pimpl));
   }
   try {
      // Switch back to the thread’s own context and have it wait for a ready coroutine.
      switch_to_scheduler(coro_pimpl);
   } catch (...) {
      // If anything went wrong or the coroutine was terminated, remove the timer.
      {
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coros_blocked_by_timer_ke.remove(ke.ident);
      }
      ke.flags = EV_DELETE;
      ::kevent(kqueue_fd.get(), &ke, 1, nullptr, 0, &ts);
      throw;
   }
#elif LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
   if (!timer_fd) {
      // No timer infrastructure yet; set it up now.
   #if LOFTY_HOST_API_LINUX
      timer_fd = io::filedesc(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC));
      if (!timer_fd) {
         exception::throw_os_error();
      }
      ::epoll_event ee;
      memory::clear(&ee.data);
      ee.data.fd = timer_fd.get();
      /* Use EPOLLET to avoid waking up multiple threads for each firing of the timer. If when the timer fires
      there will be multiple coroutines to activate (unlikely), we’ll manually rearm the timer to wake up more
      threads (or wake up the same threads repeatedly) until all coroutines are activated. */
      ee.events = EPOLLET | EPOLLIN;
      if (::epoll_ctl(epoll_fd.get(), EPOLL_CTL_ADD, timer_fd.get(), &ee) < 0) {
         exception::throw_os_error();
      }
   #elif LOFTY_HOST_API_WIN32
      timer_fd = io::filedesc(::CreateWaitableTimer(nullptr, false, nullptr));
      if (!timer_fd) {
         exception::throw_os_error();
      }
      /* Create a thread that will wait for the timer to fire and post each firing to the IOCP, effectively
      emulating a timerfd. */
      timer_thread_handle = ::CreateThread(nullptr, 0, &timer_thread_static, this, 0, nullptr);
      if (!timer_thread_handle) {
         exception::throw_os_error();
      }
   #endif
   }
   // Calculate the time at which this timer should fire.
   time_point_t sleep_end_millisecs = current_time() + millisecs;

   // Extract the next timeout from the map.
   time_point_t next_sleep_end;
   if (coros_blocked_by_timer_fd) {
      next_sleep_end = coros_blocked_by_timer_fd.front().key;
   } else {
      next_sleep_end = numeric::max<time_point_t>::value;
   }

   // Move the active coroutine to the map.
   auto itr(coros_blocked_by_timer_fd.add(sleep_end_millisecs, _std::move(active_coro_pimpl)));
   try {
      // If the calculated time is sooner than the next timeout, rearm the timer.
      if (sleep_end_millisecs < next_sleep_end) {
         arm_timer(millisecs);
      }
      // Switch back to the thread’s own context and have it wait for a ready coroutine.
      switch_to_scheduler(itr->value.get());
   } catch (...) {
      // Remove the coroutine from the map of blocked ones and rearm the timer if there are sleepers left.
      coros_blocked_by_timer_fd.remove(itr);
      arm_timer_for_next_sleep_end();
      throw;
   }
#else
   #error "TODO: HOST_API"
#endif
}

void coroutine::scheduler::block_active_until_fd_ready(
   io::filedesc_t fd, bool write
#if LOFTY_HOST_API_WIN32
   , io::overlapped * ovl
#endif
) {
#if LOFTY_HOST_API_WIN32
   LOFTY_TRACE_FUNC(this, fd, write, ovl);
#else
   LOFTY_TRACE_FUNC(this, fd, write);
#endif

   // Add fd as a new event source.
#if LOFTY_HOST_API_BSD
   struct ::kevent ke;
   ke.ident = static_cast<std::uintptr_t>(fd);
   // Use EV_ONESHOT to avoid waking up multiple threads for the same fd becoming ready.
   ke.flags = EV_ADD | EV_ONESHOT | EV_EOF;
   ke.filter = write ? EVFILT_WRITE : EVFILT_READ;
   ::timespec ts = { 0, 0 };
   if (::kevent(kqueue_fd.get(), &ke, 1, nullptr, 0, &ts) < 0) {
      exception::throw_os_error();
   }
#elif LOFTY_HOST_API_LINUX
   ::epoll_event ee;
   memory::clear(&ee.data);
   ee.data.fd = fd;
   /* Use EPOLLONESHOT to avoid waking up multiple threads for the same fd becoming ready. This means we’d
   need to then rearm it in find_coroutine_to_activate() when it becomes ready, but we’ll remove it
   instead. */
   ee.events = EPOLLONESHOT | EPOLLPRI | (write ? EPOLLOUT : EPOLLIN);
   if (::epoll_ctl(epoll_fd.get(), EPOLL_CTL_ADD, fd, &ee) < 0) {
      exception::throw_os_error();
   }
   // Afterwards, remove fd from the epoll. Ignore errors since we wouldn’t know what to do about them.
   LOFTY_DEFER_TO_SCOPE_END(::epoll_ctl(epoll_fd.get(), EPOLL_CTL_DEL, fd, nullptr));
#elif LOFTY_HOST_API_WIN32
   // TODO: ensure bind_to_this_coroutine_scheduler_iocp() has been called on fd.
   // This may repeat in case of spurious notifications by the IOCP for fd (WIN32 BUG?).
   do {
#else
   #error "TODO: HOST_API"
#endif
      // Deactivate the current coroutine and find one to activate instead.
      impl * coro_pimpl;
      {
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coro_pimpl = _std::get<0>(coros_blocked_by_fd.add_or_assign(
            fd, _std::move(active_coro_pimpl)
         ))->value.get();
      }
      try {
         // Switch back to the thread’s own context and have it wait for a ready coroutine.
         switch_to_scheduler(coro_pimpl);
      } catch (...) {
#if LOFTY_HOST_API_WIN32
         /* Cancel the pending I/O operation. Note that this will cancel ALL pending I/O on the file, not just
         this one. */
         ::CancelIo(fd);
#endif
         // Remove the coroutine from the map of blocked ones.
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coros_blocked_by_fd.remove(fd);
         throw;
      }
#if LOFTY_HOST_API_WIN32
   } while (ovl->get_result() == ERROR_IO_INCOMPLETE);
#endif
}

void coroutine::scheduler::coroutine_scheduling_loop(bool interrupting_all /*= false*/) {
   _std::shared_ptr<impl> & active_coro_pimpl_ = active_coro_pimpl;
   _pvt::coroutine_local_storage * default_crls, ** current_crls;
   _pvt::coroutine_local_storage::get_default_and_current_pointers(&default_crls, &current_crls);
#if LOFTY_HOST_API_POSIX
   ::ucontext_t * return_uctx = default_return_uctx.get();
#endif
   while ((active_coro_pimpl_ = find_coroutine_to_activate())) {
      // Swap the coroutine_local_storage pointer for this thread with that of the active coroutine.
      *current_crls = active_coro_pimpl_->local_storage_ptr();
#if LOFTY_HOST_API_POSIX
      int ret;
#endif
      {
         // Afterwards, restore the coroutine_local_storage pointer for this thread.
         LOFTY_DEFER_TO_SCOPE_END(*current_crls = default_crls);
         // Switch the current thread’s context to the active coroutine’s.
#if LOFTY_HOST_API_POSIX
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
         ret = ::swapcontext(return_uctx, active_coro_pimpl_->ucontext_ptr());
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
#elif LOFTY_HOST_API_WIN32
         ::SwitchToFiber(active_coro_pimpl_->fiber());
#else
   #error "TODO: HOST_API"
#endif
      }
#if LOFTY_HOST_API_POSIX
      if (ret < 0) {
         /* TODO: only a stack-related ENOMEM is possible, so throw a stack overflow exception
         (*active_coro_pimpl has a problem, not return_uctx). */
      }
#endif
      /* If a coroutine (in this or another thread) leaked an uncaught exception, terminate all coroutines and
      eventually this very thread. */
      if (!interrupting_all && interruption_reason_x_type.load() != exception::common_type::none) {
         interrupt_all();
         break;
      }
   }
}

#if LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
/*static*/ coroutine::scheduler::time_point_t coroutine::scheduler::current_time() {
   time_point_t now;
   #if LOFTY_HOST_API_LINUX
      ::timespec now_ts;
      ::clock_gettime(CLOCK_MONOTONIC, &now_ts);
      now  = static_cast<time_point_t>(now_ts.tv_sec) * 1000;
      now += static_cast<time_point_t>(now_ts.tv_nsec / 1000000);
   #elif LOFTY_HOST_API_WIN32
      static ::LARGE_INTEGER frequency = {{0, 0}};
      if (frequency.QuadPart == 0) {
         ::QueryPerformanceFrequency(&frequency);
      }
      ::LARGE_INTEGER now_li;
      ::QueryPerformanceCounter(&now_li);
      // TODO: handle wrap-around by keeping a “last now” and adding something if now < “last now”.
      now = now_li.QuadPart * 1000ull / frequency.QuadPart;
   #endif
   return now;
}
#endif

_std::shared_ptr<coroutine::impl> coroutine::scheduler::find_coroutine_to_activate() {
   LOFTY_TRACE_FUNC(this);

   // This loop will only repeat in case of EINTR from the blocking-wait API.
   /* TODO: if the epoll/kqueue/IOCP is shared by several threads and one thread receives and removes the last
   event source from it, what happens to the remaining threads?
   a) We could send a no-op signal (SIGCONT?) to all threads using this scheduler, to make the wait function
      return EINTR;
   b) We could have a single event source for each scheduler with the semantics of “event sources changed”, in
      edge-triggered mode so it wakes all waiting threads once, at once. */
   for (;;) {
      {
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         if (ready_coros_queue) {
            // There are coroutines that are ready to run; remove and return the first.
            return ready_coros_queue.pop_front();
         } else if (
            !coros_blocked_by_fd
#if LOFTY_HOST_API_BSD
            && !coros_blocked_by_timer_ke
#elif LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
            && !coros_blocked_by_timer_fd
#endif
         ) {
            this_thread::interruption_point();
            return nullptr;
         }
      }
      /* TODO: FIXME: coros_add_remove_mutex does not protect against race conditions for the “any coroutines
      left?” case. */

      // There are blocked coroutines; wait for the first one to become ready again.
#if LOFTY_HOST_API_BSD
      struct ::kevent ke;
      if (::kevent(kqueue_fd.get(), nullptr, 0, &ke, 1, nullptr) < 0) {
         int err = errno;
         /* TODO: EINTR is not a reliable way to interrupt a thread’s ::kevent() call when multiple threads
         share the same coroutine::scheduler. */
         if (err == EINTR) {
            this_thread::interruption_point();
            continue;
         }
         exception::throw_os_error(err);
      }
      // TODO: understand how EV_ERROR works.
      /*if (ke.flags & EV_ERROR) {
         exception::throw_os_error(ke.data);
      }*/
//      _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
      if (ke.filter == EVFILT_TIMER) {
         // Remove and return the coroutine that was waiting for this timer.
         return coros_blocked_by_timer_ke.pop(ke.ident);
      }
      io::filedesc_t fd = static_cast<io::filedesc_t>(ke.ident);
#elif LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
   #if LOFTY_HOST_API_LINUX
      ::epoll_event ee;
      if (::epoll_wait(epoll_fd.get(), &ee, 1, -1) < 0) {
         int err = errno;
         /* TODO: EINTR is not a reliable way to interrupt a thread’s ::epoll_wait() call when multiple
         threads share the same coroutine::scheduler. This is a problem for Win32 as well (see below), so it
         probably needs a shared solution. */
         if (err == EINTR) {
            this_thread::interruption_point();
            continue;
         }
         exception::throw_os_error(err);
      }
      io::filedesc_t fd = ee.data.fd;
   #elif LOFTY_HOST_API_WIN32
      ::DWORD transferred_byte_size;
      ::ULONG_PTR completion_key;
      ::OVERLAPPED * ovl;
      if (!::GetQueuedCompletionStatus(
         iocp_fd.get(), &transferred_byte_size, &completion_key, &ovl, INFINITE)
      ) {
         /* Distinguish between IOCP failures and I/O failures by also checking whether an OVERLAPPED pointer
         was returned. */
         if (!ovl) {
            exception::throw_os_error();
         }
      }
      io::filedesc_t fd = reinterpret_cast< ::HANDLE>(completion_key);
      /* Note (WIN32 BUG?)
      Empirical evidence shows that at this point, ovl might not be a valid pointer, even if the completion
      key (fd) returned was a valid Lofty-owned handle. I could not find any explanation for this, but at
      least the caller of sleep_until_fd_ready() will be able to detect the spurious notification due to
      GetOverlappedResult() setting the last error to ERROR_IO_INCOMPLETE.
      Spurious notifications seem to occur predictably with sockets when, after a completed overlapped read, a
      new overlapped read is requested and ReadFile() returns ERROR_IO_PENDING. */

      /* A completion reported on the IOCP itself is used by Lofty to emulate EINTR; see
      lofty::thread::impl::inject_exception(). While we could use a dedicated handle for this purpose, the
      IOCP reporting a completion about itself kind of makes sense. */
      /* TODO: this is not a reliable way to interrupt a thread’s ::GetQueuedCompletionStatus() call when
      multiple threads share the same coroutine::scheduler. This is a problem for POSIX as well (see above),
      so it probably needs a shared solution. */
      if (fd == iocp_fd.get()) {
         this_thread::interruption_point();
         continue;
      }
   #endif
//      _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
      if (fd == timer_fd.get()) {
         // Pop the coroutine that should run now, and rearm the timer if necessary.
         auto kv(coros_blocked_by_timer_fd.pop_front());
         if (coros_blocked_by_timer_fd) {
            arm_timer_for_next_sleep_end();
         }
         // Return the coroutine that was waiting for the timer.
         return _std::move(kv.value);
      }
#else
   #error "TODO: HOST_API"
#endif
      // Remove and return the coroutine that was waiting for this file descriptor.
      auto blocked_coro_itr(coros_blocked_by_fd.find(fd));
      if (blocked_coro_itr != coros_blocked_by_fd.cend()) {
         return coros_blocked_by_fd.pop(blocked_coro_itr);
      }
      // Else ignore this notification for an event that nobody was waiting for.
      /* TODO: in a Win32 multithreaded scenario, the IOCP notification might arrive to a thread before the
      coroutine blocked itself (on another thread) for the event due, to associating the fd with the IOCP
      before blocking, which is unavoidable and necessary. To address this, requeue the event so it gets
      another chance at being processed. It may be necessary to keep a list of handles that should be held
      until a coroutine blocks for them. */
   }
}

void coroutine::scheduler::interrupt_all() {
   // Interrupt all coroutines using pending_x_type.
   auto x_type = interruption_reason_x_type.load();
   {
      /* TODO: using a different locking pattern, this work could be split across multiple threads, in case
      multiple are associated to this scheduler. */
//      _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
      LOFTY_FOR_EACH(auto kv, coros_blocked_by_fd) {
         kv.value->inject_exception(kv.value, x_type);
      }
#if LOFTY_HOST_API_BSD
      LOFTY_FOR_EACH(auto kv, coros_blocked_by_timer_ke) {
         kv.value->inject_exception(kv.value, x_type);
      }
#elif LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
      LOFTY_FOR_EACH(auto kv, coros_blocked_by_timer_fd) {
         kv.value->inject_exception(kv.value, x_type);
      }
#endif
      /* TODO: coroutines currently running on other threads associated to this scheduler won’t have been
      interrupted by the above loops; they need to be stopped by interrupting the threads that are running
      them. */
   }
   /* Run all coroutines. Since they’ve all just been added to ready_coros_queue, they’ll all run and handle
   the interruption request, leaving the epoll/kqueue/IOCP empty, so the latter won’t be checked at all. */
   /* TODO: document that scheduling a new coroutine at this point should be avoided because it breaks the
   interruption guarantee. Maybe actively prevent new coroutines from being scheduled? */
   coroutine_scheduling_loop(true);
}

void coroutine::scheduler::interrupt_all(exception::common_type reason_x_type) {
   /* Try to set interruption_reason_x_type; if that doesn’t happen, it’s because it was already set to none,
   in which case we still go ahead and get to interrupting all coroutines. */
   auto expected_x_type = exception::common_type::none;
   interruption_reason_x_type.compare_exchange_strong(expected_x_type, reason_x_type.base());
   interrupt_all();
}

void coroutine::scheduler::return_to_scheduler(exception::common_type x_type) {
   /* Only the first uncaught exception in a coroutine can succeed at triggering termination of all
   coroutines. */
   auto expected_x_type = exception::common_type::none;
   interruption_reason_x_type.compare_exchange_strong(expected_x_type, x_type.base());

#if LOFTY_HOST_API_POSIX
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
   ::setcontext(default_return_uctx.get());
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
   // Assume ::setcontext() is always successful, in which case it never returns.
   // TODO: maybe issue warning/abort in case ::setcontext() does return?
#elif LOFTY_HOST_API_WIN32
   ::SwitchToFiber(return_fiber.get());
#else
   #error "TODO: HOST_API"
#endif
}

void coroutine::scheduler::run() {
   LOFTY_TRACE_FUNC(this);

#if LOFTY_HOST_API_POSIX
   ::ucontext_t thread_uctx;
   default_return_uctx = &thread_uctx;
   LOFTY_DEFER_TO_SCOPE_END(default_return_uctx = nullptr);
#elif LOFTY_HOST_API_WIN32
   void * pfbr = ::ConvertThreadToFiber(nullptr);
   if (!pfbr) {
      exception::throw_os_error();
   }
   LOFTY_DEFER_TO_SCOPE_END(::ConvertFiberToThread());
   return_fiber = pfbr;
#else
   #error "TODO: HOST_API"
#endif
   try {
      coroutine_scheduling_loop();
   } catch (_std::exception const & x) {
      interrupt_all(exception::execution_interruption_to_common_type(&x));
      throw;
   } catch (...) {
      interrupt_all(exception::execution_interruption_to_common_type());
      throw;
   }
}

void coroutine::scheduler::switch_to_scheduler(impl * last_active_coro_pimpl) {
#if LOFTY_HOST_API_POSIX
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
   if (::swapcontext(last_active_coro_pimpl->ucontext_ptr(), default_return_uctx.get()) < 0) {
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
      /* TODO: only a stack-related ENOMEM is possible, so throw a stack overflow exception
      (*default_return_uctx has a problem, not *active_coro_pimpl). */
   }
#elif LOFTY_HOST_API_WIN32
   ::SwitchToFiber(return_fiber.get());
#else
   #error "TODO: HOST_API"
#endif
   // Now that we’re back to the coroutine, check for any pending interruptions.
   last_active_coro_pimpl->interruption_point();
}

#if LOFTY_HOST_API_WIN32
void coroutine::scheduler::timer_thread() {
   do {
      if (::WaitForSingleObject(timer_fd.get(), INFINITE) == WAIT_OBJECT_0) {
         ::PostQueuedCompletionStatus(
            iocp_fd.get(), 0, reinterpret_cast< ::ULONG_PTR>(timer_fd.get()), nullptr
         );
      }
   } while (!stop_thread_timer.load());
}

/*static*/ ::DWORD WINAPI coroutine::scheduler::timer_thread_static(void * coro_sched) {
   try {
      static_cast<scheduler *>(coro_sched)->timer_thread();
   } catch (...) {
      return 1;
   }
   return 0;
}
#endif


// Now this can be defined.

/*static*/ void coroutine::impl::outer_main(void * p) {
   auto this_pimpl = static_cast<impl *>(p);
   // Assume for now that inner_main_fn will return without exceptions.
   exception::common_type x_type = exception::common_type::none;
   try {
      this_pimpl->inner_main_fn();
   } catch (_std::exception const & x) {
      exception::write_with_scope_trace(nullptr, &x);
      x_type = exception::execution_interruption_to_common_type(&x);
   } catch (...) {
      exception::write_with_scope_trace();
      x_type = exception::execution_interruption_to_common_type();
   }
   this_thread::coroutine_scheduler()->return_to_scheduler(x_type);
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace this_coroutine {

coroutine::id_type id() {
   return reinterpret_cast<coroutine::id_type>(coroutine::scheduler::active_coro_pimpl.get());
}

void interruption_point() {
   if (
      _std::shared_ptr<coroutine::impl> const & active_coro_pimpl_ = coroutine::scheduler::active_coro_pimpl
   ) {
      active_coro_pimpl_->interruption_point();
   }
   this_thread::interruption_point();
}

void sleep_for_ms(unsigned millisecs) {
   if (auto & pcorosched = this_thread::coroutine_scheduler()) {
      pcorosched->block_active_for_ms(millisecs);
   } else {
      this_thread::sleep_for_ms(millisecs);
   }
}

void sleep_until_fd_ready(
   io::filedesc_t fd, bool write
#if LOFTY_HOST_API_WIN32
   , io::overlapped * ovl
#endif
) {
   if (auto & pcorosched = this_thread::coroutine_scheduler()) {
      pcorosched->block_active_until_fd_ready(
         fd, write
#if LOFTY_HOST_API_WIN32
         , ovl
#endif
      );
   } else {
      this_thread::sleep_until_fd_ready(
         fd, write
#if LOFTY_HOST_API_WIN32
         , ovl
#endif
      );
   }
}

}} //namespace lofty::this_coroutine

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

coroutine_local_storage_registrar::data_members coroutine_local_storage_registrar::data_members_ =
   LOFTY__PVT_CONTEXT_LOCAL_STORAGE_REGISTRAR_INITIALIZER;

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

coroutine_local_storage::coroutine_local_storage() :
   context_local_storage_impl(&coroutine_local_storage_registrar::instance()) {
}

coroutine_local_storage::~coroutine_local_storage() {
   unsigned remaining_attempts = 10;
   bool any_destructed;
   do {
      any_destructed = destruct_vars(coroutine_local_storage_registrar::instance());
   } while (--remaining_attempts > 0 && any_destructed);
}

}} //namespace lofty::_pvt
