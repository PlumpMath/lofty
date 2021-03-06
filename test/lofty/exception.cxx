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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class exception_polymorphism : public testing::test_case {
protected:
   //! First-level lofty::generic_error subclass.
   class derived1_error : public generic_error {
   public:
      //! Default constructor.
      derived1_error() {
      }
   };

   //! Second-level lofty::generic_error subclass.
   class derived2_error : public derived1_error {
   public:
      //! Default constructor.
      derived2_error() {
      }
   };

public:
   //! See testing::test_case::title().
   virtual str title() override {
      return str(LOFTY_SL("lofty::exception – polymorphism"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      LOFTY_TRACE_FUNC(this);

      LOFTY_TESTING_ASSERT_THROWS(exception, throw_exception());
      LOFTY_TESTING_ASSERT_THROWS(generic_error, throw_generic_error());
      LOFTY_TESTING_ASSERT_THROWS(derived1_error, throw_derived1_error());
      LOFTY_TESTING_ASSERT_THROWS(derived1_error, throw_derived2_error());
      LOFTY_TESTING_ASSERT_THROWS(derived2_error, throw_derived2_error());
   }

   void throw_exception() {
      LOFTY_TRACE_FUNC(this);

      LOFTY_THROW(exception, ());
   }

   void throw_generic_error() {
      LOFTY_TRACE_FUNC(this);

      LOFTY_THROW(generic_error, ());
   }

   void throw_derived1_error() {
      LOFTY_TRACE_FUNC(this);

      LOFTY_THROW(derived1_error, ());
   }

   void throw_derived2_error() {
      LOFTY_TRACE_FUNC(this);

      LOFTY_THROW(derived2_error, ());
   }
};

}} //namespace lofty::test

LOFTY_TESTING_REGISTER_TEST_CASE(lofty::test::exception_polymorphism)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class exception_scope_trace : public testing::test_case {
public:
   //! See testing::test_case::title().
   virtual str title() override {
      return str(LOFTY_SL("lofty::exception – scope/stack trace generation"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      std::uint32_t local_int_test = 3141592654;

      LOFTY_TRACE_FUNC(this, local_int_test);

      str scope_trace;

      // Verify that the current scope trace contains this function.

      scope_trace = get_scope_trace();
      LOFTY_TESTING_ASSERT_NOT_EQUAL(scope_trace.find(LOFTY_SL("3141592654")), scope_trace.cend());

      // Verify that an exception in run_sub_*() generates a scope trace with run_sub_*().

      try {
         run_sub_1(12345678u);
      } catch (_std::exception const & x) {
         scope_trace = get_scope_trace(&x);
      }
      LOFTY_TESTING_ASSERT_NOT_EQUAL(
         scope_trace.find(LOFTY_SL("exception_scope_trace::run_sub_2")), scope_trace.cend()
      );
      LOFTY_TESTING_ASSERT_NOT_EQUAL(scope_trace.find(LOFTY_SL("spam and eggs")), scope_trace.cend());
      LOFTY_TESTING_ASSERT_NOT_EQUAL(
         scope_trace.find(LOFTY_SL("exception_scope_trace::run_sub_1")), scope_trace.cend()
      );
      LOFTY_TESTING_ASSERT_NOT_EQUAL(scope_trace.find(LOFTY_SL("12345678")), scope_trace.cend());
      // This method is invoked via the polymorphic lofty::testing::runner class.
      LOFTY_TESTING_ASSERT_NOT_EQUAL(scope_trace.find(LOFTY_SL("runner::run")), scope_trace.cend());
      LOFTY_TESTING_ASSERT_NOT_EQUAL(scope_trace.find(LOFTY_SL("3141592654")), scope_trace.cend());

      // Verify that now the scope trace does not contain run_sub_*().

      scope_trace = get_scope_trace();
      LOFTY_TESTING_ASSERT_EQUAL(
         scope_trace.find(LOFTY_SL("exception_scope_trace::run_sub_2")), scope_trace.cend()
      );
      LOFTY_TESTING_ASSERT_EQUAL(scope_trace.find(LOFTY_SL("spam and eggs")), scope_trace.cend());
      LOFTY_TESTING_ASSERT_EQUAL(
         scope_trace.find(LOFTY_SL("exception_scope_trace::run_sub_1")), scope_trace.cend()
      );
      LOFTY_TESTING_ASSERT_EQUAL(scope_trace.find(LOFTY_SL("12345678")), scope_trace.cend());
      // This method is invoked via the polymorphic lofty::testing::runner class.
      LOFTY_TESTING_ASSERT_NOT_EQUAL(scope_trace.find(LOFTY_SL("runner::run")), scope_trace.cend());
      LOFTY_TESTING_ASSERT_NOT_EQUAL(scope_trace.find(LOFTY_SL("3141592654")), scope_trace.cend());
   }

   static str get_scope_trace(_std::exception const * x = nullptr) {
      LOFTY_TRACE_FUNC(x);

      io::text::str_ostream ostream;
      exception::write_with_scope_trace(&ostream, x);
      return ostream.release_content();
   }

   void run_sub_1(std::uint32_t int_arg) {
      LOFTY_TRACE_FUNC(this, int_arg);

      run_sub_2(LOFTY_SL("spam and eggs"));
   }

   void run_sub_2(str const & str_arg) {
      LOFTY_TRACE_FUNC(this, str_arg);

      throw_exception();
   }

   void throw_exception() {
      LOFTY_TRACE_FUNC(this);

      LOFTY_THROW(exception, ());
   }
};

}} //namespace lofty::test

LOFTY_TESTING_REGISTER_TEST_CASE(lofty::test::exception_scope_trace)
