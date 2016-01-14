﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/app.hxx>
#include <abaclade/collections/vector.hxx>
#include <abaclade/io/text.hxx>
#include <abaclade/io/binary.hxx>
#include "detail/signal_dispatcher.hxx"


////////////////////////////////////////////////////////////////////////////////////////////////////

#if ABC_HOST_API_WIN32
/*! Entry point for abaclade.dll.

@param hinst
   Module’s instance handle.
@param iReason
   Reason why this function was invoked; one of DLL_{PROCESS,THREAD}_{ATTACH,DETACH}.
@param pReserved
   Legacy.
@return
   true in case of success, or false otherwise.
*/
extern "C" ::BOOL WINAPI DllMain(::HINSTANCE hinst, ::DWORD iReason, void * pReserved) {
   ABC_UNUSED_ARG(hinst);
   ABC_UNUSED_ARG(pReserved);
   if (!abc::detail::thread_local_storage::dllmain_hook(static_cast<unsigned>(iReason))) {
      return false;
   }
   return true;
}
#endif //if ABC_HOST_API_WIN32

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*static*/ app * app::sm_papp = nullptr;

app::app() {
   /* Asserting here is okay because if the assertion is true nothing will happen, and if it’s not
   that means that there’s already an app instance with all the infrastructure needed to handle a
   failed assertion. */
   ABC_ASSERT(!sm_papp, ABC_SL("multiple instantiation of abc::app singleton class"));
   sm_papp = this;
}

/*virtual*/ app::~app() {
   sm_papp = nullptr;
}

/*static*/ int app::call_main(app * papp, _args_t * pargs) {
   ABC_TRACE_FUNC(papp, pargs);

   collections::vector<str, 8> vsArgs;
// TODO: find a way to define ABC_HOST_API_WIN32_GUI, and maybe come up with a better name.
#if ABC_HOST_API_WIN32 && defined(ABC_HOST_API_WIN32_GUI)
   // TODO: call ::GetCommandLine() and parse its result.
#else
   vsArgs.set_capacity(static_cast<std::size_t>(pargs->cArgs), false);
   // Make each string not allocate a new character array.
   for (int i = 0; i < pargs->cArgs; ++i) {
      vsArgs.push_back(str(external_buffer, pargs->ppszArgs[i]));
   }
#endif

   // Invoke the program-defined main().
   // TODO: find a way to avoid this (mis)use of vector0_ptr().
   return papp->main(*vsArgs.vector0_ptr());
}

/*static*/ bool app::deinitialize_stdio() {
   ABC_TRACE_FUNC();

   bool bErrors = false;
   io::text::stdin.reset();
   io::binary::stdin.reset();
   try {
      io::text::stdout->finalize();
   } catch (_std::exception const & x) {
      if (io::text::stderr) {
         try {
            exception::write_with_scope_trace(nullptr, &x);
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
      }
      bErrors = true;
   } catch (...) {
      if (io::text::stderr) {
         try {
            exception::write_with_scope_trace();
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
      }
      bErrors = true;
   }
   io::text::stdout.reset();
   try {
      io::binary::stdout->finalize();
   } catch (_std::exception const & x) {
      if (io::text::stderr) {
         try {
            exception::write_with_scope_trace(nullptr, &x);
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
      }
      bErrors = true;
   } catch (...) {
      if (io::text::stderr) {
         try {
            exception::write_with_scope_trace();
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
      }
      bErrors = true;
   }
   io::binary::stdout.reset();
   try {
      io::text::stderr->finalize();
   } catch (_std::exception const &) {
      // FIXME: EXC-SWALLOW
      bErrors = true;
   } catch (...) {
      // FIXME: EXC-SWALLOW
      bErrors = true;
   }
   io::text::stderr.reset();
   try {
      io::binary::stderr->finalize();
   } catch (_std::exception const &) {
      // FIXME: EXC-SWALLOW
      bErrors = true;
   } catch (...) {
      // FIXME: EXC-SWALLOW
      bErrors = true;
   }
   io::binary::stderr.reset();
   return !bErrors;
}

/*static*/ bool app::initialize_stdio() {
   try {
      io::binary::stderr = io::binary::detail::make_stderr();
      io::binary::stdin  = io::binary::detail::make_stdin ();
      io::binary::stdout = io::binary::detail::make_stdout();
      io::text::stderr = io::text::detail::make_stderr();
      io::text::stdin  = io::text::detail::make_stdin ();
      io::text::stdout = io::text::detail::make_stdout();
      return true;
   } catch (_std::exception const &) {
      // Exceptions can’t be reported at this point.
      return false;
   } catch (...) {
      // Exceptions can’t be reported at this point.
      return false;
   }
}

/*static*/ int app::run(int (* pfnInstantiateAppAndCallMain)(_args_t *), _args_t * pargs) {
   // Establish these as early as possible.
   detail::thread_local_storage tls;
   detail::signal_dispatcher sd;

   int iRet;
   if (initialize_stdio()) {
      /* Assume for now that main() will return without exceptions, in which case
      abc::app_exit_interruption will be thrown in any coroutine/thread still running. */
      exception::common_type xct = exception::common_type::app_exit_interruption;
      try {
         sd.main_thread_started();
         iRet = pfnInstantiateAppAndCallMain(pargs);
      } catch (_std::exception const & x) {
         try {
            exception::write_with_scope_trace(nullptr, &x);
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
         iRet = 123;
         xct = exception::execution_interruption_to_common_type(&x);
      } catch (...) {
         try {
            exception::write_with_scope_trace();
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
         iRet = 123;
         xct = exception::execution_interruption_to_common_type();
      }
      sd.main_thread_terminated(xct);
      if (!deinitialize_stdio()) {
         iRet = 124;
      }
   } else {
      iRet = 122;
   }
   return iRet;
}

} //namespace abc
