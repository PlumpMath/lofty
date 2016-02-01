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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

template <class T>
inline str enum_impl<T>::name() const {
   _pvt::enum_member const * pem = _member();
   return str(external_buffer, pem->pszName, pem->cchName);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _pvt {

//! Implementation of the specializations of to_text_ostream for enum_impl specializations.
class ABACLADE_SYM enum_to_text_ostream_impl {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

protected:
   /*! Writes an enumeration value, applying the formatting options.

   @param i
      Value of the enumeration member to write.
   @param pem
      Pointer to the enumeration members map.
   @param ptos
      Pointer to the stream to output to.
   */
   void write_impl(int i, enum_member const * pem, io::text::ostream * ptos);
};

}} //namespace abc::_pvt

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc {

template <class T>
class to_text_ostream<enum_impl<T>> : public _pvt::enum_to_text_ostream_impl {
public:
   //! See abc::_pvt::enum_to_text_ostream_impl::write().
   void write(enum_impl<T> e, io::text::ostream * ptos) {
      _pvt::enum_to_text_ostream_impl::write_impl(e.base(), e._get_map(), ptos);
   }
};

} //namespace abc
//! @endcond