﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABC_CORE_HXX
   #error Please #include <abc/core.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_enum_to_str_backend_impl


namespace abc {

/** Implementation of the specializations of to_str_backend for enum_impl specializations.
*/
class ABCAPI _enum_to_str_backend_impl {
public:

   /** Constructor.

   crFormat
      Formatting options.
   */
   _enum_to_str_backend_impl(char_range const & crFormat);


protected:

   /** Writes an enumeration value, applying the formatting options.

   e
      Enumeration value to write.
   posOut
      Pointer to the output stream to write to.
   */
   void write_impl(int i, enum_member const * pem, io::ostream * posOut);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::enum_impl


namespace abc {

// Specialization of to_str_backend.
template <class T>
class to_str_backend<enum_impl<T>> :
   public _enum_to_str_backend_impl {
public:

   /** Constructor. See abc::_enum_to_str_backend_impl::_enum_to_str_backend_impl().
   */
   to_str_backend(char_range const & crFormat = char_range()) :
      _enum_to_str_backend_impl(crFormat) {
   }


   /** See abc::_enum_to_str_backend_impl::write().
   */
   void write(enum_impl<T> e, io::ostream * posOut) {
      _enum_to_str_backend_impl::write_impl(e.base(), e._get_map(), posOut);
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

