﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals

#if ABC_HOST_API_WIN32
/*! Entry point for abaclade.dll.

hinst
   Module’s instance handle.
iReason
   Reason why the DLL entry point was invoked; one of DLL_{PROCESS,THREAD}_{ATTACH,DETACH}.
return
   true in case of success, or false otherwise.
*/
extern "C" BOOL WINAPI DllMain(HINSTANCE hinst, DWORD iReason, void * pReserved) {
   ABC_UNUSED_ARG(hinst);
   ABC_UNUSED_ARG(pReserved);
   if (iReason == DLL_PROCESS_ATTACH || iReason == DLL_THREAD_ATTACH) {
      if (iReason == DLL_PROCESS_ATTACH) {
         abc::detail::thread_local_storage::alloc_slot();
      }
      // Not calling abc::detail::thread_local_storage::construct() since initialization of TLS
      // is lazy.
   } else if (iReason == DLL_THREAD_DETACH || iReason == DLL_PROCESS_DETACH) {
      abc::detail::thread_local_storage::destruct();
      if (iReason == DLL_PROCESS_DETACH) {
         abc::detail::thread_local_storage::free_slot();
      }
   }
   return true;
}
#endif //if ABC_HOST_API_WIN32

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_explob_helper

namespace abc {

#ifndef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
void _explob_helper::bool_true() const {
}
#endif

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

