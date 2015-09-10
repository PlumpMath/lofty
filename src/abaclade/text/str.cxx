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
#include <abaclade/collections.hxx>
#include <abaclade/collections/vector.hxx>
#include <abaclade/text.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

external_buffer_t const external_buffer;

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text {

//! Single NUL terminator.
static char_t const gc_chNul('\0');

//! Vextr referencing a static, empty, NUL-terminated raw C string.
static collections::detail::raw_vextr_impl_data const gc_rvidEmpty = {
   /*m_pBegin                      =*/ const_cast<char_t *>(&gc_chNul),
   /*m_pEnd                        =*/ const_cast<char_t *>(&gc_chNul),
   /*mc_bEmbeddedPrefixedItemArray =*/ false,
   /*m_bPrefixedItemArray          =*/ false,
   /*m_bDynamic                    =*/ false,
   /*m_bNulT                       =*/ true
};

str const & str::empty = static_cast<str const &>(gc_rvidEmpty);


std::ptrdiff_t str::const_iterator::distance(std::size_t ich) const {
   ABC_TRACE_FUNC(this, ich);

   if (ich == m_ich) {
      return 0;
   } else {
      char_t const * pchBegin = m_ps->data();
      if (ich < m_ich) {
         return static_cast<std::ptrdiff_t>(
            str_traits::size_in_codepoints(pchBegin + ich, pchBegin + m_ich)
         );
      } else {
         return -static_cast<std::ptrdiff_t>(
            str_traits::size_in_codepoints(pchBegin + m_ich, pchBegin + ich)
         );
      }
   }
}

std::size_t str::advance_index_by_codepoint_delta(
   std::size_t ich, std::ptrdiff_t iDelta, bool bAllowEnd
) const {
   char_t const * pchBegin = data(), * pch = pchBegin + ich, * pchEnd = data_end();

   // If i is positive, move forward.
   for (; iDelta > 0 && pch < pchEnd; --iDelta) {
      // Find the next code point start, skipping any trail characters.
      pch += host_char_traits::lead_char_to_codepoint_size(*pch);
   }
   // If i is negative, move backwards.
   for (; iDelta < 0 && pch > pchBegin; ++iDelta) {
      // Moving to the previous code point requires finding the previous non-trail character.
      while (host_char_traits::is_trail_char(*--pch)) {
         ;
      }
   }
   // Use the remainder of iDelta. If it was non-zero, this will bring pch out of range.
   pch += iDelta;
   /* Verify that pch is still within range: that’s not the case if we left either for loop before
   iDelta reached 0, or if the pointer was invalid on entry (e.g. accessing str()[0]). */
   if (bAllowEnd) {
      ++pchEnd;
   }
   if (pch < pchBegin || pch >= pchEnd) {
      ABC_THROW(collections::out_of_range, (pch, pchBegin, data_end()));
   }

   // Return the resulting index.
   return static_cast<std::size_t>(pch - pchBegin);
}

/*static*/ std::size_t str::advance_index_by_codepoint_delta(
   str const * ps, std::size_t ich, std::ptrdiff_t iDelta, bool bAllowEnd
) {
   if (!ps) {
      ABC_THROW(collections::out_of_range, ());
   }
   return ps->advance_index_by_codepoint_delta(ich, iDelta, bAllowEnd);
}

str::c_str_ptr str::c_str() {
   ABC_TRACE_FUNC(this);

   if (m_bNulT) {
      // The string already includes a NUL terminator, so we can simply return the same array.
   } else if (std::size_t cch = size_in_chars()) {
      // The string is not empty but lacks a NUL terminator: enlarge the string to include one.
      prepare_for_writing();
      set_capacity(cch + 1, true);
      *data_end() = '\0';
      m_bNulT = true;
   } else {
      // The string is empty, so a static NUL character will suffice.
      return c_str_ptr(&gc_chNul, false);
   }
   return c_str_ptr(data(), false);
}

str::c_str_ptr str::c_str() const {
   ABC_TRACE_FUNC(this);

   if (m_bNulT) {
      // The string already includes a NUL terminator, so we can simply return the same array.
      return c_str_ptr(data(), false);
   } else if (std::size_t cch = size_in_chars()) {
      /* The string is not empty but lacks a NUL terminator: create a temporary copy that includes a
      NUL, and return it. */
      auto psz(memory::alloc_unique<char_t[]>(cch + 1 /*NUL*/));
      memory::copy(psz.get(), data(), cch);
      psz[cch] = '\0';
      return c_str_ptr(psz.release(), true);
   } else {
      // The string is empty, so a static NUL character will suffice.
      return c_str_ptr(&gc_chNul, false);
   }
}

collections::vector<std::uint8_t> str::encode(encoding enc, bool bNulT) const {
   ABC_TRACE_FUNC(this, enc, bNulT);

   collections::vector<std::uint8_t> vb;
   std::size_t cbChar, cbUsed, cbStr = size_in_bytes();
   if (enc == encoding::host) {
      // Optimal case: no transcoding necessary.
      cbChar = sizeof(char_t);
      // Enlarge vb as necessary, then copy to it the contents of the string buffer.
      vb.set_capacity(cbStr + (bNulT ? sizeof(char_t) : 0), false);
      memory::copy(vb.data(), vextr_impl::begin<std::uint8_t>(), cbStr);
      cbUsed = cbStr;
   } else {
      cbChar = get_encoding_size(enc);
      void const * pStr = data();
      // Calculate the size required, then resize vb accorgingly.
      cbUsed = transcode(true, encoding::host, &pStr, &cbStr, enc);
      vb.set_capacity(cbUsed + (bNulT ? cbChar : 0), false);
      // Transcode the string into vb.
      void * pBuf = vb.data();
      // Re-assign to cbUsed because transcode() will set *(&cbUsed) to 0.
      cbUsed = transcode(true, encoding::host, &pStr, &cbStr, enc, &pBuf, &cbUsed);
   }
   if (bNulT) {
      memory::clear(vb.data() + cbUsed, cbChar);
      cbUsed += cbChar;
   }
   // Assign the vector its size, and return it.
   vb.set_size(cbUsed);
   return _std::move(vb);
}

bool str::ends_with(str const & s) const {
   ABC_TRACE_FUNC(this, s);

   char_t const * pchStart = data_end() - s.size_in_chars();
   return pchStart >= data() && str_traits::compare(
      pchStart, data_end(), s.data(), s.data_end()
   ) == 0;
}

str::const_iterator str::find(char_t chNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, chNeedle, itWhence);

   char_t const * pchWhence = data() + itWhence.m_ich;
   validate_pointer(pchWhence, true);
   auto pch = str_traits::find_char(pchWhence, data_end(), chNeedle);
   return const_iterator(this, static_cast<std::size_t>(pch - data()));
}
str::const_iterator str::find(char32_t cpNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, cpNeedle, itWhence);

   char_t const * pchWhence = data() + itWhence.m_ich;
   validate_pointer(pchWhence, true);
   auto pch = str_traits::find_char(pchWhence, data_end(), cpNeedle);
   return const_iterator(this, static_cast<std::size_t>(pch - data()));
}
str::const_iterator str::find(str const & sNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, sNeedle, itWhence);

   char_t const * pchWhence = data() + itWhence.m_ich;
   validate_pointer(pchWhence, true);
   auto pch = str_traits::find_substr(pchWhence, data_end(), sNeedle.data(), sNeedle.data_end());
   return const_iterator(this, static_cast<std::size_t>(pch - data()));
}

str::const_iterator str::find_last(char_t chNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, chNeedle, itWhence);

   char_t const * pchWhence = data() + itWhence.m_ich;
   validate_pointer(pchWhence, true);
   auto pch = str_traits::find_char_last(data(), pchWhence, chNeedle);
   return const_iterator(this, static_cast<std::size_t>(pch - data()));
}
str::const_iterator str::find_last(char32_t cpNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, cpNeedle, itWhence);

   char_t const * pchWhence = data() + itWhence.m_ich;
   validate_pointer(pchWhence, true);
   auto pch = str_traits::find_char_last(data(), pchWhence, cpNeedle);
   return const_iterator(this, static_cast<std::size_t>(pch - data()));
}
str::const_iterator str::find_last(str const & sNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, sNeedle, itWhence);

   char_t const * pchWhence = data() + itWhence.m_ich;
   validate_pointer(pchWhence, true);
   auto pch = str_traits::find_substr_last(data(), pchWhence, sNeedle.data(), sNeedle.data_end());
   return const_iterator(this, static_cast<std::size_t>(pch - data()));
}

void str::prepare_for_writing() {
   if (!m_bPrefixedItemArray) {
      /* Copying from itself is safe because the new character array will necessarily not overlap
      the old, non-prefixed one. */
      assign_copy(data(), data_end());
   }
}

void str::replace(char_t chSearch, char_t chReplacement) {
   ABC_TRACE_FUNC(this, chSearch, chReplacement);

   prepare_for_writing();
   for (char_t * pch = data(), * pchEnd = data_end(); pch != pchEnd; ++pch) {
      if (*pch == chSearch) {
         *pch = chReplacement;
      }
   }
}

void str::replace(char32_t cpSearch, char32_t cpReplacement) {
   ABC_TRACE_FUNC(this, cpSearch, cpReplacement);

   prepare_for_writing();
   // TODO: optimize this. Using iterators requires little code but it’s not that efficient.
   for (auto it(begin()); it != end(); ++it) {
      if (*it == cpSearch) {
         *it = cpReplacement;
      }
   }
}

void str::replace_codepoint(std::size_t ich, char_t chNew) {
   ABC_TRACE_FUNC(this, ich, chNew);

   std::size_t cbRemove = sizeof(char_t) * host_char_traits::lead_char_to_codepoint_size(
      data()[ich]
   );
   // Note: either of these two calls may change data().
   prepare_for_writing();
   vextr_impl::insert_remove(ich, nullptr, sizeof(char_t), cbRemove);
   data()[ich] = chNew;
}

void str::replace_codepoint(std::size_t ich, char32_t cpNew) {
   ABC_TRACE_FUNC(this, ich, cpNew);

   std::size_t cbInsert = sizeof(char_t) * host_char_traits::codepoint_size(cpNew);
   std::size_t cbRemove = sizeof(char_t) * host_char_traits::lead_char_to_codepoint_size(
      data()[ich]
   );
   // Note: either of these two calls may change data().
   prepare_for_writing();
   vextr_impl::insert_remove(sizeof(char_t) * ich, nullptr, cbInsert, cbRemove);
   // codepoint_size() validated cpNew, so nothing can go wrong here.
   host_char_traits::traits_base::codepoint_to_chars(cpNew, data() + ich);
}

void str::set_from(_std::function<std::size_t (char_t * pch, std::size_t cchMax)> const & fnRead) {
   ABC_TRACE_FUNC(this/*, fnRead*/);

   prepare_for_writing();
   /* The initial size avoids a few reallocations (* smc_iGrowthRate ** 2). Multiplying by
   smc_iGrowthRate should guarantee that set_capacity() will allocate exactly the requested number
   of characters, eliminating the need to query back with capacity(). */
   std::size_t cchRet, cchMax = smc_cbCapacityMin * smc_iGrowthRate;
   do {
      cchMax *= smc_iGrowthRate;
      set_capacity(cchMax, false);
      cchRet = fnRead(data(), cchMax);
   } while (cchRet >= cchMax);
   // Finalize the length.
   set_size_in_chars(cchRet);
}

bool str::starts_with(str const & s) const {
   ABC_TRACE_FUNC(this, s);

   char_t const * pchEnd = data() + s.size_in_chars();
   return pchEnd <= data_end() && str_traits::compare(data(), pchEnd, s.data(), s.data_end()) == 0;
}

char_t const * str::validate_index_to_pointer(std::size_t ich, bool bAllowEnd) const {
   auto pchBegin = data(), pchEnd = data_end(), pch = pchBegin + ich;
   if (bAllowEnd) {
      ++pchEnd;
   }
   if (pch < pchBegin || pch >= pchEnd) {
      ABC_THROW(collections::out_of_range, (pch, pchBegin, m_pEnd));
   }
   return pch;
}

/*static*/ char_t const * str::validate_index_to_pointer(
   str const * ps, std::size_t ich, bool bAllowEnd
) {
   if (!ps) {
      ABC_THROW(collections::out_of_range, ());
   }
   return ps->validate_index_to_pointer(ich, bAllowEnd);
}

}} //namespace abc::text

namespace std {

/* Implementation based on the Fowler/Noll/Vo variant 1a (FNV-1a) algorithm. See
<http://www.isthe.com/chongo/tech/comp/fnv/> for details.

The bases are calculated by src/fnv_hash_basis.py. */
std::size_t hash<abc::text::str>::operator()(abc::text::str const & s) const {
   ABC_TRACE_FUNC(this, s);

   static_assert(
      sizeof(std::size_t) * 8 == ABC_HOST_WORD_SIZE,
      "unexpected sizeof(std::size_t) will break FNV prime/basis selection"
   );
#if ABC_HOST_WORD_SIZE == 16
   static std::size_t const sc_iFNVPrime = 0x1135;
   static std::size_t const sc_iFNVBasis = 16635u;
#elif ABC_HOST_WORD_SIZE == 32
   static std::size_t const sc_iFNVPrime = 0x01000193;
   static std::size_t const sc_iFNVBasis = 2166136261u;
#elif ABC_HOST_WORD_SIZE == 64
   static std::size_t const sc_iFNVPrime = 0x00000100000001b3;
   static std::size_t const sc_iFNVBasis = 14695981039346656037u;
#endif
   std::size_t iHash = sc_iFNVBasis;
   for (auto it(s.cbegin()), itEnd(s.cend()); it != itEnd; ++it) {
      iHash ^= static_cast<std::size_t>(*it);
      iHash *= sc_iFNVPrime;
   }
   return iHash;
}

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text { namespace detail {

void str_to_str_backend::set_format(str const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   auto it(sFormat.cbegin());

   // Add parsing of the format string here.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         ABC_SL("unexpected character"), sFormat, static_cast<unsigned>(it - sFormat.cend())
      ));
   }
}

void str_to_str_backend::write(
   void const * p, std::size_t cb, encoding enc, io::text::writer * ptwOut
) {
   ABC_TRACE_FUNC(this, p, cb, enc, ptwOut);

   ptwOut->write_binary(p, cb, enc);
}

}}} //namespace abc::text::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

void to_str_backend<text::str>::write(text::str const & s, io::text::writer * ptwOut) {
   text::detail::str_to_str_backend::write(s.data(), static_cast<std::size_t>(
      reinterpret_cast<std::uintptr_t>(s.data_end()) - reinterpret_cast<std::uintptr_t>(s.data())
   ), text::encoding::host, ptwOut);
}

} //namespace abc
