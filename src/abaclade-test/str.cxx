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
#include <abaclade/testing/utility.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test globals


namespace abc {
namespace test {

char32_t const gc_chP0(ABC_CHAR('\x20ac'));
char32_t const gc_chP2(0x024b62);
// The string “acabaabca” has the following properties:
// •  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s
//    “aba” instead), to catch incorrect skip-last comparisons;
// •  first and last characters match 'a', but other inner ones do too;
// •  would match “abcd” were it not for the last character;
// •  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
// The only thing though is that we replace ‘b’ with the Unicode Plane 2 character defined
// above and ‘c’ with the BMP (Plane 0) character above.
istr const gc_sAcabaabca(
   istr() + 'a' + gc_chP0 + 'a' + gc_chP2 + SL("aa") + gc_chP2 + gc_chP0 + 'a'
);

} //namespace test
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_basic


namespace abc {
namespace test {

class str_basic :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*str classes – basic operations"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      dmstr s;
      auto cdpt(testing::utility::make_container_data_ptr_tracker(s));

      s += SL("ä");
      // true: operator+= must have created an item array (there was none).
      ABC_TESTING_ASSERT_TRUE(cdpt.changed());
      ABC_TESTING_ASSERT_THROWS(index_error, s[-1]);
      ABC_TESTING_ASSERT_DOES_NOT_THROW(s[0]);
      ABC_TESTING_ASSERT_THROWS(index_error, s[1]);
      ABC_TESTING_ASSERT_THROWS(iterator_error, --s.cbegin());
      ABC_TESTING_ASSERT_DOES_NOT_THROW(++s.cbegin());
      ABC_TESTING_ASSERT_DOES_NOT_THROW(--s.cend());
      ABC_TESTING_ASSERT_THROWS(iterator_error, ++s.cend());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 1u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1u);
      ABC_TESTING_ASSERT_EQUAL(s[0], ABC_CHAR('ä'));

      s = s + 'b' + s;
      // true: a new string is created by operator+, which replaces s by operator=.
      ABC_TESTING_ASSERT_TRUE(cdpt.changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 3u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("äbä"));

      s = s.substr(1, 3);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(cdpt.changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 2u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("bä"));

      s += 'c';
      // false: there should’ve been enough space for 'c'.
      ABC_TESTING_ASSERT_FALSE(cdpt.changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 3u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("bäc"));

      s = s.substr(0, -1);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(cdpt.changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 2u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2u);
      ABC_TESTING_ASSERT_EQUAL(s[0], 'b');
      ABC_TESTING_ASSERT_EQUAL(s[1], ABC_CHAR('ä'));

      s += s;
      // false: there should’ve been enough space for “baba”.
      ABC_TESTING_ASSERT_FALSE(cdpt.changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 4u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4u);
      ABC_TESTING_ASSERT_EQUAL(s[0], 'b');
      ABC_TESTING_ASSERT_EQUAL(s[1], ABC_CHAR('ä'));
      ABC_TESTING_ASSERT_EQUAL(s[2], 'b');
      ABC_TESTING_ASSERT_EQUAL(s[3], ABC_CHAR('ä'));

      s = s.substr(-3, -2);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(cdpt.changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 1u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1u);
      ABC_TESTING_ASSERT_EQUAL(s[0], ABC_CHAR('ä'));

      s = dmstr(SL("ab")) + 'c';
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(cdpt.changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 3u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
      ABC_TESTING_ASSERT_EQUAL(s[0], 'a');
      ABC_TESTING_ASSERT_EQUAL(s[1], 'b');
      ABC_TESTING_ASSERT_EQUAL(s[2], 'c');

      s += 'd';
      // false: there should’ve been enough space for “abcd”.
      ABC_TESTING_ASSERT_FALSE(cdpt.changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 4u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4u);
      ABC_TESTING_ASSERT_EQUAL(s[0], 'a');
      ABC_TESTING_ASSERT_EQUAL(s[1], 'b');
      ABC_TESTING_ASSERT_EQUAL(s[2], 'c');
      ABC_TESTING_ASSERT_EQUAL(s[3], 'd');

      s += SL("efghijklmnopqrstuvwxyz");
      // Cannot assert (ABC_TESTING_ASSERT_*) on this to behave in any specific way, since the
      // character array may or may not change depending on heap reallocation strategy.
      cdpt.changed();
      ABC_TESTING_ASSERT_EQUAL(s.size(), 26u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 26u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("abcdefghijklmnopqrstuvwxyz"));

      s = SL("a\0b");
      s += SL("\0ç");
      // false: there should have been plenty of storage allocated.
      ABC_TESTING_ASSERT_FALSE(cdpt.changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 5u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 5u);
      // Test both ways to make sure that the char_t[] overload is always chosen over char *.
      ABC_TESTING_ASSERT_EQUAL(s, SL("a\0b\0ç"));
      ABC_TESTING_ASSERT_EQUAL(SL("a\0b\0ç"), s);

      {
         // Note: all string operations here must involve as few characters as possible to avoid
         // triggering a reallocation, which would break these tests.

         dmstr s1, s2(SL("a"));
         char_t const * pchCheck(s2.cbegin().base());
         // Verify that the compiler selects operator+(dmstr &&, …) when possible.
         s1 = std::move(s2) + SL("b");
         ABC_TESTING_ASSERT_EQUAL(s1.cbegin().base(), pchCheck);

         istr s3(std::move(s1));
         // Verify that the compiler selects operator+(istr &&, …) when possible.
         s1 = std::move(s3) + SL("c");
         ABC_TESTING_ASSERT_EQUAL(s1.cbegin().base(), pchCheck);
      }

      // While we’re at it, let’s also validate gc_sAcabaabca.
      ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[0], 'a');
      ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[1], gc_chP0);
      ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[2], 'a');
      ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[3], gc_chP2);
      ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[4], 'a');
      ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[5], 'a');
      ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[6], gc_chP2);
      ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[7], gc_chP0);
      ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[8], 'a');
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_basic)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_iterators


namespace abc {
namespace test {

class str_iterators :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*str classes – iterator-based character access"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      dmstr s;
      auto cdpt(testing::utility::make_container_data_ptr_tracker(s));

      // No accessible characters.
      ABC_TESTING_ASSERT_THROWS(index_error, s[-1]);
      ABC_TESTING_ASSERT_THROWS(index_error, s[0]);

      // Should not allow to move an iterator to outside [begin, end].
      ABC_TESTING_ASSERT_DOES_NOT_THROW(s.cbegin());
      ABC_TESTING_ASSERT_DOES_NOT_THROW(s.cend());
      ABC_TESTING_ASSERT_THROWS(iterator_error, --s.cbegin());
      ABC_TESTING_ASSERT_THROWS(iterator_error, ++s.cbegin());
      ABC_TESTING_ASSERT_THROWS(iterator_error, --s.cend());
      ABC_TESTING_ASSERT_THROWS(iterator_error, ++s.cend());

      // Should not allow to dereference end().
      ABC_TESTING_ASSERT_THROWS(iterator_error, *s.cend());
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_iterators)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_encode


namespace abc {
namespace test {

class str_encode :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(
         SL("abc::*str classes – conversion to different encodings")
      );
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      smstr<32> s;
      s += char32_t(0x000024);
      s += char32_t(0x0000a2);
      s += char32_t(0x0020ac);
      s += char32_t(0x024b62);
      dmvector<uint8_t> vb;

      vb = s.encode(text::encoding::utf8, false);
      {
         smvector<uint8_t, 16> vbUtf8;
         vbUtf8.append(0x24);
         vbUtf8.append(0xc2);
         vbUtf8.append(0xa2);
         vbUtf8.append(0xe2);
         vbUtf8.append(0x82);
         vbUtf8.append(0xac);
         vbUtf8.append(0xf0);
         vbUtf8.append(0xa4);
         vbUtf8.append(0xad);
         vbUtf8.append(0xa2);
         ABC_TESTING_ASSERT_EQUAL(vb, vbUtf8);
      }

      vb = s.encode(text::encoding::utf16be, false);
      {
         smvector<uint8_t, 16> vbUtf16;
         vbUtf16.append(0x00);
         vbUtf16.append(0x24);
         vbUtf16.append(0x00);
         vbUtf16.append(0xa2);
         vbUtf16.append(0x20);
         vbUtf16.append(0xac);
         vbUtf16.append(0xd8);
         vbUtf16.append(0x52);
         vbUtf16.append(0xdf);
         vbUtf16.append(0x62);
         ABC_TESTING_ASSERT_EQUAL(vb, vbUtf16);
      }

      vb = s.encode(text::encoding::utf32le, false);
      {
         smvector<uint8_t, 16> vbUtf32;
         vbUtf32.append(0x24);
         vbUtf32.append(0x00);
         vbUtf32.append(0x00);
         vbUtf32.append(0x00);
         vbUtf32.append(0xa2);
         vbUtf32.append(0x00);
         vbUtf32.append(0x00);
         vbUtf32.append(0x00);
         vbUtf32.append(0xac);
         vbUtf32.append(0x20);
         vbUtf32.append(0x00);
         vbUtf32.append(0x00);
         vbUtf32.append(0x62);
         vbUtf32.append(0x4b);
         vbUtf32.append(0x02);
         vbUtf32.append(0x00);
         ABC_TESTING_ASSERT_EQUAL(vb, vbUtf32);
      }
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_encode)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_replace


namespace abc {
namespace test {

class str_replace :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*str classes – character replacement"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      smstr<8> s;

      // No replacements to be made.
      ABC_TESTING_ASSERT_EQUAL(((s = SL("aaa")).replace('b', 'c'), s), SL("aaa"));
      // Simple ASCII-to-ASCII replacement: no size change.
      ABC_TESTING_ASSERT_EQUAL(((s = SL("aaa")).replace('a', 'b'), s), SL("bbb"));
      // Complex ASCII-to-char32_t replacement: size will increase beyond the embedded capacity, so
      // the iterator used in abc::mstr::replace() must be intelligent enough to self-refresh with
      // the new descriptor.
      ABC_TESTING_ASSERT_EQUAL(
         ((s = SL("aaaaa")).replace(char32_t('a'), gc_chP2), s),
         istr() + gc_chP2 + gc_chP2 + gc_chP2 + gc_chP2 + gc_chP2
      );
      // Less-complex char32_t-to-ASCII replacement: size will decrease.
      ABC_TESTING_ASSERT_EQUAL(
         ((s = istr() + gc_chP2 + gc_chP2 + gc_chP2 + gc_chP2 + gc_chP2).
            replace(gc_chP2, char32_t('a')), s),
         SL("aaaaa")
      );
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_replace)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_substr


namespace abc {
namespace test {

class str_substr_range_permutations :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*str classes – range permutations"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      istr sEmpty, sAB(SL("äb"));

      // Substring of empty string.
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(-1, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(-1, 0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(-1, 1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(0, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(0, 0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(0, 1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(1, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(1, 0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(1, 1), SL(""));

      // Substring of a 2-characer string.
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, -1), SL("ä"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, 0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, 1), SL("ä"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, 2), SL("äb"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, -1), SL("ä"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, 0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, 1), SL("ä"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, 2), SL("äb"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, 0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, 1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, 2), SL("b"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, -1), SL("ä"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, 0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, 1), SL("ä"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, 2), SL("äb"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, 0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, 1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, 2), SL("b"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, 0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, 1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, 2), SL(""));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_substr_range_permutations)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::istr_c_str


namespace abc {
namespace test {

class istr_c_str :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::istr – C string extraction"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      istr s;
      auto psz(s.c_str());
      // s has no character array, so it should have returned the static NUL character.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

      s = SL("");
      psz = s.c_str();
      // s should have adopted the literal and therefore have a trailing NUL, so it should have
      // returned its own character array.
      ABC_TESTING_ASSERT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

      s = SL("a");
      psz = s.c_str();
      // s should have adopted the literal and therefore have a trailing NUL, so it should have
      // returned its own character array.
      ABC_TESTING_ASSERT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 1u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
      ABC_TESTING_ASSERT_EQUAL(psz[1], '\0');
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::istr_c_str)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::mstr_c_str


namespace abc {
namespace test {

class mstr_c_str :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::mstr – C string extraction"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      dmstr s;
      auto psz(s.c_str());
      // s has no character array, so it should have returned the static NUL character.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

      s = SL("");
      psz = s.c_str();
      // s still has no character array, so it should have returned the static NUL character again.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

      s = SL("a");
      psz = s.c_str();
      // s should have copied the literal but dropped its trailing NUL, so it must’ve returned a
      // distinct character array.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_TRUE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 1u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
      ABC_TESTING_ASSERT_EQUAL(psz[1], '\0');

      s += SL("b");
      psz = s.c_str();
      // The character array should have grown, but still lack the trailing NUL.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_TRUE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 2u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
      ABC_TESTING_ASSERT_EQUAL(psz[1], 'b');
      ABC_TESTING_ASSERT_EQUAL(psz[2], '\0');
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::mstr_c_str)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_substr


namespace abc {
namespace test {

class str_find :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*str classes – character and substring search"));
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      // Special characters.
      char32_t ch0(gc_chP0);
      char32_t ch2(gc_chP2);
      // See gc_sAcabaabca for more information on its pattern. To make it more interesting, here we
      // also duplicate it.
      istr const s(gc_sAcabaabca + gc_sAcabaabca);

      ABC_TESTING_ASSERT_EQUAL(s.find(ch0), s.cbegin() + 1);
      ABC_TESTING_ASSERT_EQUAL(s.find('d'), s.cend());
      ABC_TESTING_ASSERT_EQUAL(s.find(istr() + 'a' + ch2), s.cbegin() + 2);
      ABC_TESTING_ASSERT_EQUAL(s.find(istr() + 'a' + ch2 + ch0 + 'a'), s.cbegin() + 5);
      ABC_TESTING_ASSERT_EQUAL(s.find(istr() + 'a' + ch2 + ch0 + 'd'), s.cend());
      ABC_TESTING_ASSERT_EQUAL(s.find(istr() + 'a' + ch2 + SL("aa") + ch2 + ch0), s.cbegin() + 2);
      ABC_TESTING_ASSERT_EQUAL(s.find(istr() + 'a' + ch2 + SL("aa") + ch2 + ch0 + 'd'), s.cend());
      ABC_TESTING_ASSERT_EQUAL(s.find_last('a'), s.cend() - 1);
#if 0
      ABC_TESTING_ASSERT_EQUAL(s.find_last(ch2), s.cend() - 3);
      ABC_TESTING_ASSERT_EQUAL(s.find_last(SL("ab")), s.cend() - 4);
      ABC_TESTING_ASSERT_EQUAL(s.find_last(SL("ac")), s.cend() - 9);
      ABC_TESTING_ASSERT_EQUAL(s.find_last(SL("ca")), s.cend() - 2);
#endif
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_find)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_substr_starts_with


namespace abc {
namespace test {

class str_substr_starts_with :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(
         SL("abc::*str classes – initial matching")
      );
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      // Special characters.
      char32_t ch0(gc_chP0);
      char32_t ch2(gc_chP2);
      // See gc_sAcabaabca for more information on its pattern.
      istr const & s(gc_sAcabaabca);

      ABC_TESTING_ASSERT_TRUE(s.starts_with(istr()));
      ABC_TESTING_ASSERT_TRUE(s.starts_with(istr() + 'a'));
      ABC_TESTING_ASSERT_TRUE(s.starts_with(istr() + 'a' + ch0));
      ABC_TESTING_ASSERT_FALSE(s.starts_with(istr() + 'a' + ch2));
      ABC_TESTING_ASSERT_FALSE(s.starts_with(istr() + ch0));
      ABC_TESTING_ASSERT_FALSE(s.starts_with(istr() + ch2));
      ABC_TESTING_ASSERT_TRUE(s.starts_with(s));
      ABC_TESTING_ASSERT_FALSE(s.starts_with(s + '-'));
      ABC_TESTING_ASSERT_FALSE(s.starts_with('-' + s));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_substr_starts_with)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_substr_ends_with


namespace abc {
namespace test {

class str_substr_ends_with :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(
         SL("abc::*str classes – final matching")
      );
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      // Special characters.
      char32_t ch0(gc_chP0);
      char32_t ch2(gc_chP2);
      // See gc_sAcabaabca for more information on its pattern.
      istr const & s(gc_sAcabaabca);

      ABC_TESTING_ASSERT_TRUE(s.ends_with(istr()));
      ABC_TESTING_ASSERT_TRUE(s.ends_with(istr() + 'a'));
      ABC_TESTING_ASSERT_TRUE(s.ends_with(istr() + ch0 + 'a'));
      ABC_TESTING_ASSERT_FALSE(s.ends_with(istr() + ch2 + 'a'));
      ABC_TESTING_ASSERT_FALSE(s.ends_with(istr() + ch0));
      ABC_TESTING_ASSERT_FALSE(s.ends_with(istr() + ch2));
      ABC_TESTING_ASSERT_TRUE(s.ends_with(s));
      ABC_TESTING_ASSERT_FALSE(s.ends_with(s + '-'));
      ABC_TESTING_ASSERT_FALSE(s.ends_with('-' + s));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_substr_ends_with)


////////////////////////////////////////////////////////////////////////////////////////////////////

