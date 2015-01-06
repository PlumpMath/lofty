﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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

#if ABC_HOST_API_POSIX
   #include <sys/stat.h> // stat
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary globals

namespace abc {
namespace io {
namespace binary {
namespace detail {

struct file_init_data {
#if ABC_HOST_API_POSIX
   //! Set by _construct().
   struct ::stat statFile;
#endif
   //! See file_base::m_fd. To be set before calling _construct().
   filedesc fd;
   /*! Determines what type of I/O object will be instantiated. To be set before calling
   _construct(). */
   access_mode am;
   /*! If true, causes the file to be opened with flags to the effect of disabling OS cache for the
   file. To be set before calling _construct(). */
   bool bBypassCache:1;
};

} //namespace detail
} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
