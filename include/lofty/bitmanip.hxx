﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

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

#ifndef _LOFTY_BITMANIP_HXX
#define _LOFTY_BITMANIP_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <climits> // CHAR_BIT


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Bit manipulation functions.
namespace bitmanip {}

} //namespace lofty

/* Intrinsics in MSC16:
•  _BitScanForward
•  _BitScanReverse
•  _bittest
•  _bittestandcomplement
•  _bittestandreset
•  _bittestandset
•  _InterlockedCompareExchange
•  _InterlockedCompareExchange64
*/

namespace lofty { namespace bitmanip { namespace _pvt {

/*! Helper for ceiling_to_pow2(), to unify specializations based on sizeof(T). See 
lofty::bitmanip::ceiling_to_pow2(). */
LOFTY_SYM std::uint8_t  ceiling_to_pow2(std::uint8_t i);
LOFTY_SYM std::uint16_t ceiling_to_pow2(std::uint16_t i);
LOFTY_SYM std::uint32_t ceiling_to_pow2(std::uint32_t i);
LOFTY_SYM std::uint64_t ceiling_to_pow2(std::uint64_t i);

}}} //namespace lofty::bitmanip::_pvt

namespace lofty { namespace bitmanip {

/*! Returns the argument rounded up to the closest power of 2.

@param i
   Integer to round up.
@return
   Smallest power of 2 that’s not smaller than i.
*/
template <typename T>
inline T ceiling_to_pow2(T i) {
   static_assert(
      sizeof(T) == sizeof(std::uint8_t ) || sizeof(T) == sizeof(std::uint16_t) ||
      sizeof(T) == sizeof(std::uint32_t) || sizeof(T) == sizeof(std::uint64_t),
      "sizeof(T) is not a supported power of 2"
   );
   switch (sizeof(T)) {
      case sizeof(std::uint8_t):
         return _pvt::ceiling_to_pow2(static_cast<std::uint8_t>(i));
      case sizeof(std::uint16_t):
         return _pvt::ceiling_to_pow2(static_cast<std::uint16_t>(i));
      case sizeof(std::uint32_t):
         return _pvt::ceiling_to_pow2(static_cast<std::uint32_t>(i));
      case sizeof(std::uint64_t):
         return _pvt::ceiling_to_pow2(static_cast<std::uint64_t>(i));
   }
}

/*! Returns the first argument rounded up to a multiple of the second, which has to be a power of 2.

@param i
   Integer to round up.
@param step
   Power of 2 to use as step to increment i.
@return
   Smallest multiple of step that’s not smaller than i.
*/
template <typename T>
inline /*constexpr*/ T ceiling_to_pow2_multiple(T i, T step) {
   --step;
   return (i + step) & ~step;
}

/*! Rotates bits to the left (most significant bits shifted out, and back in to become least significant).

@param i
   Integer the bits of which are to be rotated.
@param bits
   Count of positions the bits in i will be shifted.
@return
   Rotated bits of i.
*/
template <typename T>
inline /*constexpr*/ T rotate_l(T i, unsigned bits) {
   return (i << bits) | (i >> (sizeof(T) * CHAR_BIT - bits));
}

/*! Rotates bits to the right (least significant bits shifted out, and back in to become most significant).

@param i
   Integer the bits of which are to be rotated.
@param bits
   Count of positions the bits in i will be shifted.
@return
   Rotated bits of i.
*/
template <typename T>
inline /*constexpr*/ T rotate_r(T i, unsigned bits) {
   return (i >> bits) | (i << (sizeof(T) * CHAR_BIT - bits));
}

}} //namespace lofty::bitmanip

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_BITMANIP_HXX
