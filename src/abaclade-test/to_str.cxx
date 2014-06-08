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
#include <abaclade/testing/test_case.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::to_str_test_case_base

namespace abc {
namespace test {

class to_str_test_case_base :
   public testing::test_case {
protected:

   /** Constructor.
   */
   to_str_test_case_base() :
      m_stw(&m_sWriterBuffer) {
   }


   /** Same as abc::to_str(), except it uses an writer with a static string for higher speed.

   t
      Value to output.
   sFormatSpec
      Format specification for t.
   return
      The resulting contents of the internal writer.
   */
   template <typename T>
   istr const & get_to_str_output(T const & t, istr const & sFormatSpec) {
      ABC_TRACE_FUNC(t, sFormatSpec);

      to_str_backend<T> tsb(sFormatSpec);
      m_stw.clear();
      tsb.write(t, &m_stw);
      return m_stw.get_str();
   }


protected:

   /** Buffer for m_tw, to avoid performance impacts from memory (re)allocation. */
   smstr<128> m_sWriterBuffer;
   /** Destination for to_str() writes. */
   io::text::str_writer m_stw;
};

} //namespace test
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::to_str_int

namespace abc {
namespace test {

class to_str_int :
   public to_str_test_case_base {
public:

   /** See to_str_test_case_base::title().
   */
   virtual istr title() {
      return istr(SL("abc::to_str – int"));
   }


   /** See to_str_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      // Test zero, decimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(0, SL("")), SL("0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(0, SL(" 1")), SL(" 0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(0, SL("01")), SL("0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(0, SL(" 2")), SL(" 0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(0, SL("02")), SL("00"));

      // Test positive values, decimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(1, SL("")), SL("1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(1, SL(" 1")), SL(" 1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(1, SL("01")), SL("1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(1, SL(" 2")), SL(" 1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(1, SL("02")), SL("01"));

      // Test negative values, decimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(-1, SL("")), SL("-1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(-1, SL(" 1")), SL("-1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(-1, SL("01")), SL("-1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(-1, SL(" 2")), SL("-1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(-1, SL("02")), SL("-1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(-1, SL(" 3")), SL(" -1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(-1, SL("03")), SL("-01"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::to_str_int)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::to_str_int8

namespace abc {
namespace test {

class to_str_int8 :
   public to_str_test_case_base {
public:

   /** See to_str_test_case_base::title().
   */
   virtual istr title() {
      return istr(SL("abc::to_str – int8_t"));
   }


   /** See to_str_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      // Test zero, hexadecimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(0), SL("x")), SL("0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(0), SL(" 1x")), SL("0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(0), SL("01x")), SL("0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(0), SL(" 2x")), SL(" 0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(0), SL("02x")), SL("00"));

      // Test positive values, hexadecimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(1), SL("x")), SL("1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(1), SL(" 1x")), SL("1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(1), SL("01x")), SL("1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(1), SL(" 2x")), SL(" 1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(1), SL("02x")), SL("01"));

      // Test negative values, hexadecimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(-1), SL("x")), SL("ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(-1), SL(" 1x")), SL("ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(-1), SL("01x")), SL("ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(-1), SL(" 2x")), SL("ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(-1), SL("02x")), SL("ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(-1), SL(" 3x")), SL(" ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_output(int8_t(-1), SL("03x")), SL("0ff"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::to_str_int8)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::to_str_raw_pointers

namespace abc {
namespace test {

class to_str_raw_pointers :
   public to_str_test_case_base {
public:

   /** See to_str_test_case_base::title().
   */
   virtual istr title() {
      return istr(SL("abc::to_str – raw pointers"));
   }


   /** See to_str_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      uintptr_t iBad(0xbad);

      // Test nullptr.
      ABC_TESTING_ASSERT_EQUAL(
         get_to_str_output(static_cast<void *>(nullptr), SL("")), SL("nullptr")
      );

      // Test void pointer.
      ABC_TESTING_ASSERT_EQUAL(
         get_to_str_output(reinterpret_cast<void *>(iBad), SL("")), SL("0xbad")
      );

      // Test void const volatile pointer.
      ABC_TESTING_ASSERT_EQUAL(
         get_to_str_output(reinterpret_cast<void const volatile *>(iBad), SL("")), SL("0xbad")
      );

      // Test function pointer.
      ABC_TESTING_ASSERT_EQUAL(
         get_to_str_output(reinterpret_cast<void (*)(int)>(iBad), SL("")), SL("0xbad")
      );

      // Test char_t const pointer. Also confirms that pointers-to-char are NOT treated as strings
      // by abc::to_str().
      ABC_TESTING_ASSERT_EQUAL(
         get_to_str_output(reinterpret_cast<char_t const *>(iBad), SL("")), SL("0xbad")
      );
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::to_str_raw_pointers)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::to_str_smart_pointers

namespace abc {
namespace test {

class to_str_smart_pointers :
   public to_str_test_case_base {
public:

   /** See to_str_test_case_base::title().
   */
   virtual istr title() {
      return istr(SL("abc::to_str – smart pointers"));
   }


   /** See to_str_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      int * pi(new int);
      istr sPtr(to_str(pi));

      {
         std::unique_ptr<int> upi(pi);
         // Test non-nullptr std::unique_ptr.
         ABC_TESTING_ASSERT_EQUAL(get_to_str_output(upi, SL("")), sPtr);

         upi.release();
         // Test nullptr std::unique_ptr.
         ABC_TESTING_ASSERT_EQUAL(get_to_str_output(upi, SL("")), SL("nullptr"));
      }
      {
         std::shared_ptr<int> spi(pi);
         // Test non-nullptr std::shared_ptr.
         ABC_TESTING_ASSERT_EQUAL(get_to_str_output(spi, SL("")), sPtr);
         std::weak_ptr<int> wpi(spi);
         // Test non-nullptr std::weak_ptr.
         ABC_TESTING_ASSERT_EQUAL(get_to_str_output(wpi, SL("")), sPtr);

         spi.reset();
         // Test nullptr std::shared_ptr.
         ABC_TESTING_ASSERT_EQUAL(get_to_str_output(spi, SL("")), SL("nullptr"));
         // Test expired non-nullptr std::weak_ptr.
         ABC_TESTING_ASSERT_EQUAL(get_to_str_output(wpi, SL("")), SL("nullptr"));

         wpi.reset();
         // Test nullptr std::weak_ptr.
         ABC_TESTING_ASSERT_EQUAL(get_to_str_output(wpi, SL("")), SL("nullptr"));
      }
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::to_str_smart_pointers)


////////////////////////////////////////////////////////////////////////////////////////////////////
