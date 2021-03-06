﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2017 Raffaello D. Di Napoli

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
#include <lofty/to_str.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test { namespace {

class type_with_member_ttos {
public:
   type_with_member_ttos(str s_) :
      s(_std::move(s_)) {
   }

   str const & get() const {
      return s;
   }

   void to_text_ostream(io::text::ostream * dst) const {
      dst->write(s);
   }

private:
   str s;
};

class type_with_nonmember_ttos {
public:
   type_with_nonmember_ttos(str s_) :
      s(_std::move(s_)) {
   }

   str const & get() const {
      return s;
   }

private:
   str s;
};

}}} //namespace lofty::test::

namespace lofty {

template <>
class to_text_ostream<test::type_with_nonmember_ttos> {
public:
   void set_format(str const & format) {
      LOFTY_UNUSED_ARG(format);
   }

   void write(test::type_with_nonmember_ttos const & src, io::text::ostream * dst) {
      dst->write(src.get());
   }
};

} //namespace lofty

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   to_text_ostream_member_nonmember,
   "lofty::to_text_ostream – member and non-member to_text_ostream"
) {
   LOFTY_TRACE_FUNC(this);

   type_with_member_ttos    twmt(LOFTY_SL("TWMT"));
   type_with_nonmember_ttos twnt(LOFTY_SL("TWNT"));

   /* These assertions are more important at compile time than at run time; if the to_str() calls compile,
   they won’t return the wrong value. */
   LOFTY_TESTING_ASSERT_EQUAL(to_str(twmt), twmt.get());
   LOFTY_TESTING_ASSERT_EQUAL(to_str(twnt), twnt.get());
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   to_text_ostream_bool,
   "lofty::to_text_ostream – bool"
) {
   LOFTY_TRACE_FUNC(this);

   LOFTY_TESTING_ASSERT_EQUAL(to_str(false), LOFTY_SL("false"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(true), LOFTY_SL("true"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   to_text_ostream_int,
   "lofty::to_text_ostream – int"
) {
   LOFTY_TRACE_FUNC(this);

   // Test zero, decimal base.
   LOFTY_TESTING_ASSERT_EQUAL(to_str(0, str::empty), LOFTY_SL("0"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(0, LOFTY_SL(" 1")), LOFTY_SL(" 0"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(0, LOFTY_SL("01")), LOFTY_SL("0"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(0, LOFTY_SL(" 2")), LOFTY_SL(" 0"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(0, LOFTY_SL("02")), LOFTY_SL("00"));

   // Test positive values, decimal base.
   LOFTY_TESTING_ASSERT_EQUAL(to_str(1, str::empty), LOFTY_SL("1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(1, LOFTY_SL(" 1")), LOFTY_SL(" 1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(1, LOFTY_SL("01")), LOFTY_SL("1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(1, LOFTY_SL(" 2")), LOFTY_SL(" 1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(1, LOFTY_SL("02")), LOFTY_SL("01"));

   // Test negative values, decimal base.
   LOFTY_TESTING_ASSERT_EQUAL(to_str(-1, str::empty), LOFTY_SL("-1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(-1, LOFTY_SL(" 1")), LOFTY_SL("-1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(-1, LOFTY_SL("01")), LOFTY_SL("-1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(-1, LOFTY_SL(" 2")), LOFTY_SL("-1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(-1, LOFTY_SL("02")), LOFTY_SL("-1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(-1, LOFTY_SL(" 3")), LOFTY_SL(" -1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(-1, LOFTY_SL("03")), LOFTY_SL("-01"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   to_text_ostream_std_int8_t,
   "lofty::to_text_ostream – std::int8_t"
) {
   LOFTY_TRACE_FUNC(this);

   // Test zero, hexadecimal base.
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(0), LOFTY_SL("x")), LOFTY_SL("0"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(0), LOFTY_SL(" 1x")), LOFTY_SL("0"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(0), LOFTY_SL("01x")), LOFTY_SL("0"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(0), LOFTY_SL(" 2x")), LOFTY_SL(" 0"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(0), LOFTY_SL("02x")), LOFTY_SL("00"));

   // Test positive values, hexadecimal base.
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(1), LOFTY_SL("x")), LOFTY_SL("1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(1), LOFTY_SL(" 1x")), LOFTY_SL("1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(1), LOFTY_SL("01x")), LOFTY_SL("1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(1), LOFTY_SL(" 2x")), LOFTY_SL(" 1"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(1), LOFTY_SL("02x")), LOFTY_SL("01"));

   // Test negative values, hexadecimal base.
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), LOFTY_SL("x")), LOFTY_SL("ff"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), LOFTY_SL(" 1x")), LOFTY_SL("ff"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), LOFTY_SL("01x")), LOFTY_SL("ff"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), LOFTY_SL(" 2x")), LOFTY_SL("ff"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), LOFTY_SL("02x")), LOFTY_SL("ff"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), LOFTY_SL(" 3x")), LOFTY_SL(" ff"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), LOFTY_SL("03x")), LOFTY_SL("0ff"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   to_text_ostream_raw_ptr,
   "lofty::to_text_ostream – raw pointers"
) {
   LOFTY_TRACE_FUNC(this);

   std::uintptr_t bad = 0xbad;

   // Test nullptr.
   LOFTY_TESTING_ASSERT_EQUAL(to_str(static_cast<void *>(nullptr), str::empty), LOFTY_SL("nullptr"));

   // Test void pointer.
   LOFTY_TESTING_ASSERT_EQUAL(to_str(reinterpret_cast<void *>(bad), str::empty), LOFTY_SL("0xbad"));

   // Test void const volatile pointer.
   LOFTY_TESTING_ASSERT_EQUAL(
      to_str(reinterpret_cast<void const volatile *>(bad), str::empty), LOFTY_SL("0xbad")
   );

   // Test function pointer.
   LOFTY_TESTING_ASSERT_EQUAL(to_str(reinterpret_cast<void (*)(int)>(bad), str::empty), LOFTY_SL("0xbad"));

   /* Test char_t const pointer. Also confirms that pointers-to-char are NOT treated as strings by
   lofty::to_text_ostream(). */
   LOFTY_TESTING_ASSERT_EQUAL(to_str(reinterpret_cast<char_t const *>(bad), str::empty), LOFTY_SL("0xbad"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   to_text_ostream_smart_ptr,
   "lofty::to_text_ostream – smart pointers"
) {
   LOFTY_TRACE_FUNC(this);

   int * raw_ptr = new int;
   str ptr_str(to_str(raw_ptr));

   {
      _std::unique_ptr<int> u_ptr(raw_ptr);
      // Test non-nullptr _std::unique_ptr.
      LOFTY_TESTING_ASSERT_EQUAL(to_str(u_ptr, str::empty), ptr_str);

      u_ptr.release();
      // Test nullptr _std::unique_ptr.
      LOFTY_TESTING_ASSERT_EQUAL(to_str(u_ptr, str::empty), LOFTY_SL("nullptr"));
   }
   {
      _std::shared_ptr<int> sh_ptr(raw_ptr);
      // Test non-nullptr _std::shared_ptr.
      LOFTY_TESTING_ASSERT_EQUAL(to_str(sh_ptr, str::empty), ptr_str);
      _std::weak_ptr<int> wk_ptr(sh_ptr);
      // Test non-nullptr _std::weak_ptr.
      LOFTY_TESTING_ASSERT_EQUAL(to_str(wk_ptr, str::empty), ptr_str);

      sh_ptr.reset();
      // Test nullptr _std::shared_ptr.
      LOFTY_TESTING_ASSERT_EQUAL(to_str(sh_ptr, str::empty), LOFTY_SL("nullptr"));
      // Test expired non-nullptr _std::weak_ptr.
      LOFTY_TESTING_ASSERT_EQUAL(to_str(wk_ptr, str::empty), LOFTY_SL("nullptr"));

      wk_ptr.reset();
      // Test nullptr _std::weak_ptr.
      LOFTY_TESTING_ASSERT_EQUAL(to_str(wk_ptr, str::empty), LOFTY_SL("nullptr"));
   }
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   to_text_ostream_tuple,
   "lofty::to_text_ostream – STL tuple types"
) {
   LOFTY_TRACE_FUNC(this);

   // Test _std::tuple.
   LOFTY_TESTING_ASSERT_EQUAL(to_str(_std::tuple<>()), LOFTY_SL("()"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(_std::tuple<int>(1)), LOFTY_SL("(1)"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(_std::tuple<int, int>(1, 2)), LOFTY_SL("(1, 2)"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(_std::tuple<str, int>(LOFTY_SL("abc"), 42)), LOFTY_SL("(abc, 42)"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

union union_type {
   int i;
   char ch;
};

struct struct_type {
   int i;
   char ch;
};

class class_type {
public:
   int i;
   char ch;
};

LOFTY_TESTING_TEST_CASE_FUNC(
   to_text_ostream_std_type_info,
   "lofty::to_text_ostream – std::type_info"
) {
   LOFTY_TRACE_FUNC(this);

   // Test std::type_info.
   LOFTY_TESTING_ASSERT_EQUAL(to_str(typeid(1)), LOFTY_SL("int"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(typeid(double)), LOFTY_SL("double"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(typeid(bool)), LOFTY_SL("bool"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(typeid(union_type )), LOFTY_SL("lofty::test::union_type" ));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(typeid(struct_type)), LOFTY_SL("lofty::test::struct_type"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(typeid(class_type )), LOFTY_SL("lofty::test::class_type" ));
}

}} //namespace lofty::test
