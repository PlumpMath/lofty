﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
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

#ifndef _ABC_STL_ALGORITHM_HXX
#define _ABC_STL_ALGORITHM_HXX

#ifndef _ABC_HXX
   #error Please #include <abc.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// std globals


namespace std {

/** Returns the greatest of two objects (C++11 § 25.4.7 “Minimum and maximum”).

t1
   First value.
t2
   Second value.
return
   Greatest of t1 and t2.
*/
template <typename T>
inline T const & max(T const & t1, T const & t2) {
   return t1 > t2 ? t1 : t2;
}
template <typename T, class TCompare>
inline T const & max(T const & t1, T const & t2, TCompare fnComp) {
   return fnComp(t1, t2) > 0 ? t1 : t2;
}


/** Returns the least of two objects (C++11 § 25.4.7 “Minimum and maximum”).

t1
   First value.
t2
   Second value.
return
   Least of t1 and t2.
*/
template <typename T>
inline T const & min(T const & t1, T const & t2) {
   return t1 < t2 ? t1 : t2;
}
template <typename T, class TCompare>
inline T const & min(T const & t1, T const & t2, TCompare fnComp) {
   return fnComp(t1, t2) < 0 ? t1 : t2;
}

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC_STL_ALGORITHM_HXX

