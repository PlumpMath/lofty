﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_STL_EXCEPTION_HXX
#define _ABACLADE_STL_EXCEPTION_HXX

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// std::exception


namespace std {

/** Base class for standard exceptions (18.8.1).
*/
class ABACLADE_SYM exception {
public:

   /** Constructor.
   */
   exception();


   /** Destructor.
   */
   virtual ~exception();


   /** Returns information on the exception.

   return
      Description of the exception.
   */
   virtual char const * what() const;
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABACLADE_STL_EXCEPTION_HXX
