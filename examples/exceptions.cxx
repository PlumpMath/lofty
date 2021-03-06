﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015, 2017 Raffaello D. Di Napoli

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

/*! @file
Exceptions and stack traces example

This program showcases Lofty’s ability to display stack traces when an exception is thrown, as
well as its support for catching null pointer access and similar invalid operations. See the source
code for more comments. */

#include <lofty.hxx>
#include <lofty/app.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/io/text.hxx>

using namespace lofty;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Application class for this program.
class exceptions_app : public app {
public:
   /*! Main function of the program.

   @param args
      Arguments that were provided to this program via command line.
   @return
      Return value of this program.
   */
   virtual int main(collections::vector<str> & args) override {
      LOFTY_TRACE_FUNC(this, args);

      LOFTY_UNUSED_ARG(args);
      str s(LOFTY_SL("Test String"));

      collections::vector<int> ints;
      ints.push_back(101);
      ints.push_back(102);

      io::text::stdout->print(LOFTY_SL("Populated ints with {} and {}\n"), ints[0], ints[1]);

      io::text::stdout->write_line(LOFTY_SL("Before calling first_function()"));
      io::text::stdout->write_line();
      first_function(s, ints);

      // This will never happen.
      io::text::stdout->write_line(LOFTY_SL("After calling first_function()"));

      return 0;
   }

   /*! Sample enumeration. Used to demonstrate Lofty’s support for automatic translation of enumerated
   values into strings. */
   LOFTY_ENUM(numbers_enum,
      (zero,  0),
      (one,   1),
      (two,   2),
      (three, 3),
      (four,  4)
   );

   void first_function(str const & s, collections::vector<int> const & ints) const {
      LOFTY_TRACE_FUNC(this, s, ints);

      exception::write_with_scope_trace(io::text::stdout.get());
      io::text::stdout->write_line();

      io::text::stdout->write_line(LOFTY_SL("Before calling is_zero()"));
      io::text::stdout->write_line();

      // Passing a null pointer!
      is_zero(numbers_enum::two, nullptr);

      // This will never happen.
      io::text::stdout->write_line(LOFTY_SL("After calling is_zero()"));
   }

   void is_zero(numbers_enum number, bool * ret) const {
      LOFTY_TRACE_FUNC(this, number, ret);

      *ret = (number == numbers_enum::zero);
   }
};

LOFTY_APP_CLASS(exceptions_app)
