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

#include <abaclade.hxx>

ABCMK_CMP_BEGIN


// FIXME: ABC_CPP_LIST_COUNT() returns 1 instead of 0.
ABC_CPP_LIST_COUNT(a)
ABC_CPP_LIST_COUNT((a, 1))
ABC_CPP_LIST_COUNT(abc, cde)
ABC_CPP_LIST_COUNT((a, 1), (b, 2), (c, 3), (d, 4))

#define SCALAR_WALKER(x) x
ABC_CPP_LIST_WALK(SCALAR_WALKER, a)
ABC_CPP_LIST_WALK(SCALAR_WALKER, a, b, c, d)

#define TUPLE_WALKER(x, y) x = y,
ABC_CPP_TUPLELIST_WALK(TUPLE_WALKER, (a, 1))
ABC_CPP_TUPLELIST_WALK(TUPLE_WALKER, (a, 1), (b, 2), (c, 3), (d, 4))

ABCMK_CMP_END