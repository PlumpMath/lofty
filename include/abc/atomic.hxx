﻿/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2010, 2011, 2012, 2013
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef ABC_ATOMIC_HXX
#define ABC_ATOMIC_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/core.hxx>
#if ABC_HOST_API_POSIX
	#include <pthread.h>
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

namespace atomic {

/// Integer type of optimal size for atomic operations (usually the machine’s word size).
#if ABC_HOST_API_POSIX
	// No preference really, since we use don’t use atomic intrinsics.
	typedef int int_t;
#elif ABC_HOST_API_WIN32
	// Win32 uses long to mean 32 bits, always.
	typedef long int_t;
#else
	#error TODO-PORT: HOST_API
#endif

#if ABC_HOST_API_POSIX
extern pthread_mutex_t g_mtx;
#endif

/// Atomically add the second argument to the number pointed to by the first argument, storing the
// result in *pi and returning it.
template <typename I>
I add(I volatile * piDst, I iAddend);

/// Atomically add the second argument to the number pointed to by the first argument, storing the
// result in *pi and returning it.
template <typename I>
I compare_exchange(I volatile * piDst, I iNewValue, I iComparand);

/// Atomically decrements the number pointed to by the argument, storing the result in *pi and
// returning it.
template <typename I>
I decrement(I volatile * pi);

/// Atomically increments the number pointed to by the argument, storing the result in *pi and
// returning it.
template <typename I>
I increment(I volatile * pi);

/// Atomically subtracts the second argument from the number pointed to by the first argument,
// storing the result in *pi and returning it.
template <typename I>
I subtract(I volatile * piDst, I iSubtrahend);

} //namespace atomic

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::atomic globals


namespace abc {

namespace atomic {

template <typename I>
I add(I volatile * piDst, I iAddend) {
#if ABC_HOST_API_POSIX
	I iRet;
	pthread_mutex_lock(&g_mtx);
	iRet = (*piDst += iAddend);
	pthread_mutex_unlock(&g_mtx);
	return iRet;
#elif ABC_HOST_API_WIN32
	switch (sizeof(I)) {
		case sizeof(long):
			return ::InterlockedAdd(reinterpret_cast<long volatile *>(piDst), iAddend);
#if _WIN32_WINNT >= 0x0502
		case sizeof(long long):
			return ::InterlockedAdd64(reinterpret_cast<long long volatile *>(piDst), iAddend);
#endif //if _WIN32_WINNT >= 0x0502
	}
#else
	#error TODO-PORT: HOST_API
#endif
}


template <typename I>
I compare_and_swap(I volatile * piDst, I iNewValue, I iComparand) {
#if ABC_HOST_API_POSIX
	I iOldValue;
	pthread_mutex_lock(&g_mtx);
	if ((iOldValue = *piDst) == iComparand) {
		*piDst = iNewValue;
	}
	pthread_mutex_unlock(&g_mtx);
	return iOldValue;
#elif ABC_HOST_API_WIN32
	switch (sizeof(I)) {
		case sizeof(long):
			return ::InterlockedCompareExchange(
				reinterpret_cast<long volatile *>(piDst), iNewValue, iComparand
			);
#if _WIN32_WINNT >= 0x0502
		case sizeof(long long):
			return ::InterlockedCompareExchange64(
				reinterpret_cast<long long volatile *>(piDst), iNewValue, iComparand
			);
#endif //if _WIN32_WINNT >= 0x0502
	}
#else
	#error TODO-PORT: HOST_API
#endif
}


template <typename I>
inline I decrement(I volatile * pi) {
#if ABC_HOST_API_POSIX
	I iRet;
	pthread_mutex_lock(&g_mtx);
	iRet = --*pi;
	pthread_mutex_unlock(&g_mtx);
	return iRet;
#elif ABC_HOST_API_WIN32
	switch (sizeof(I)) {
		case sizeof(long):
			return ::InterlockedDecrement(reinterpret_cast<long volatile *>(pi));
#if _WIN32_WINNT >= 0x0502
		case sizeof(long long):
			return ::InterlockedDecrement64(reinterpret_cast<long long volatile *>(pi));
#endif //if _WIN32_WINNT >= 0x0502
	}
#else
	#error TODO-PORT: HOST_API
#endif
}


template <typename I>
inline I increment(I volatile * pi) {
#if ABC_HOST_API_POSIX
	I iRet;
	pthread_mutex_lock(&g_mtx);
	iRet = ++*pi;
	pthread_mutex_unlock(&g_mtx);
	return iRet;
#elif ABC_HOST_API_WIN32
	switch (sizeof(I)) {
		case sizeof(long):
			return ::InterlockedIncrement(reinterpret_cast<long volatile *>(pi));
#if _WIN32_WINNT >= 0x0502
		case sizeof(long long):
			return ::InterlockedIncrement64(reinterpret_cast<long long volatile *>(pi));
#endif //if _WIN32_WINNT >= 0x0502
	}
#else
	#error TODO-PORT: HOST_API
#endif
}


template <typename I>
I subtract(I volatile * piDst, I iSubtrahend) {
#if ABC_HOST_API_POSIX
	I iRet;
	pthread_mutex_lock(&g_mtx);
	iRet = (*piDst -= iSubtrahend);
	pthread_mutex_unlock(&g_mtx);
	return iRet;
#elif ABC_HOST_API_WIN32
	switch (sizeof(I)) {
		case sizeof(long):
			return ::InterlockedAdd(
				reinterpret_cast<long volatile *>(piDst), -long(iSubtrahend)
			);
#if _WIN32_WINNT >= 0x0502
		case sizeof(long long):
			return ::InterlockedAdd64(
				reinterpret_cast<long long volatile *>(piDst), -long long(iSubtrahend)
			);
#endif //if _WIN32_WINNT >= 0x0502
	}
#else
	#error TODO-PORT: HOST_API
#endif
}

} //namespace atomic

} //namespace abc


//////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_ATOMIC_HXX

