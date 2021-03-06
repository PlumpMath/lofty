﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2017 Raffaello D. Di Napoli

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

#include <lofty.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/process.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   this_process_env,
   "lofty::this_process – retrieving environment variables"
) {
   LOFTY_TRACE_FUNC(this);

   LOFTY_TESTING_ASSERT_NOT_EQUAL(_std::get<0>(this_process::env_var(LOFTY_SL("PATH"))), str::empty);
   LOFTY_TESTING_ASSERT_TRUE(_std::get<1>(this_process::env_var(LOFTY_SL("PATH"))));
   LOFTY_TESTING_ASSERT_EQUAL(_std::get<0>(this_process::env_var(LOFTY_SL("?UNSET?"))), str::empty);
   LOFTY_TESTING_ASSERT_FALSE(_std::get<1>(this_process::env_var(LOFTY_SL("?UNSET?"))));
}

}} //namespace lofty::test
