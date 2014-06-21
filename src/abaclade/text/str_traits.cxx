﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf8_str_traits


namespace abc {
namespace text {

uint8_t const utf8_str_traits::smc_aiOverlongDetectionMasks[] = {
   // Leading byte = 110zzzzz, first continuation byte must have at least one K=1 in ccKyyyyy.
   0x20,
   // Leading byte = 1110zzzz, first continuation byte must have at least one K=1 in ccKKyyyy.
   0x30,
   // Leading byte = 11110zzz, first continuation byte must have at least one K=1 in ccKKKyyy.
   0x38,
   // Leading byte = 111110zz, first continuation byte must have at least one K=1 in ccKKKKyy.
   0x3c,
   // Leading byte = 1111110z, first continuation byte must have at least one K=1 in ccKKKKKy.
   0x3e
};


/*static*/ bool utf8_str_traits::is_valid(char8_t const * psz) {
   ABC_TRACE_FUNC(psz);

   unsigned cbCont(0);
   bool bCheckFirstContByteForOverlongs;
   while (char8_t ch = *psz++) {
      if (cbCont) {
         // Ensure that the lead byte is really followed by cbCont trailing bytes.
         if (!utf8_char_traits::is_trail_char(ch)) {
            return false;
         }
         --cbCont;
         if (bCheckFirstContByteForOverlongs) {
            // Detect an overlong due to unused bits in the lead byte and first continuation bytes.
            // See smc_aiOverlongDetectionMasks for more information on how this check works.
            if (!(ch & smc_aiOverlongDetectionMasks[cbCont])) {
               return false;
            }
            bCheckFirstContByteForOverlongs = false;
         }
      } else {
         // This should be a lead byte, and not the invalid 1111111x.
         if (utf8_char_traits::is_trail_char(ch) || uint8_t(ch) >= 0xfe) {
            return false;
         }
         // Detect an overlong that would fit in a single character: 11000001 10yyyyyy should have
         // been encoded as 01yyyyyy.
         if (ch == char8_t(0xc1)) {
            return false;
         }
         cbCont = utf8_char_traits::lead_char_to_codepoint_size(ch) - 1;
         // If no code point bits are used (1) in the lead byte, enable overlong detection for
         // the first continuation byte.
         bCheckFirstContByteForOverlongs = (
            utf8_char_traits::get_lead_char_codepoint_bits(ch, cbCont) == 0
         );
      }
   }
   return cbCont == 0;
}
/*static*/ bool utf8_str_traits::is_valid(char8_t const * pchBegin, char8_t const * pchEnd) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   unsigned cbCont(0);
   bool bCheckFirstContByteForOverlongs;
   for (char8_t const * pch(pchBegin); pch < pchEnd; ++pch) {
      char8_t ch(*pch);
      if (cbCont) {
         // Ensure that the lead byte is really followed by cbCont trailing bytes.
         if (!utf8_char_traits::is_trail_char(ch)) {
            return false;
         }
         --cbCont;
         if (bCheckFirstContByteForOverlongs) {
            // Detect an overlong due to unused bits in the lead byte and first continuation bytes.
            // See smc_aiOverlongDetectionMasks for more information on how this check works.
            if (!(ch & smc_aiOverlongDetectionMasks[cbCont])) {
               return false;
            }
            bCheckFirstContByteForOverlongs = false;
         }
      } else {
         // This should be a lead byte, and not the invalid 1111111x.
         if (utf8_char_traits::is_trail_char(ch) || uint8_t(ch) >= 0xfe) {
            return false;
         }
         // Detect an overlong that would fit in a single character: 11000001 10yyyyyy should have
         // been encoded as 01yyyyyy.
         if (ch == char8_t(0xc1)) {
            return false;
         }
         cbCont = utf8_char_traits::lead_char_to_codepoint_size(ch) - 1;
         // If no code point bits are used (1) in the lead byte, enable overlong detection for
         // the first continuation byte.
         bCheckFirstContByteForOverlongs = (
            utf8_char_traits::get_lead_char_codepoint_bits(ch, cbCont) == 0
         );
      }
   }
   return cbCont == 0;
}


/*static*/ size_t utf8_str_traits::size_in_chars(char8_t const * psz) {
   ABC_TRACE_FUNC(psz);

   char8_t const * pch(psz);
   while (*pch) {
      ++pch;
   }
   return size_t(pch - psz);
}


/*static*/ size_t utf8_str_traits::size_in_codepoints(
   char8_t const * pchBegin, char8_t const * pchEnd
) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   size_t ccp(0);
   // Count a single code point for each lead byte, skipping over trailing bytes.
   for (
      char8_t const * pch(pchBegin);
      pch < pchEnd;
      pch += utf8_char_traits::lead_char_to_codepoint_size(*pch)
   ) {
      ++ccp;
   }
   return ccp;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf16_str_traits


namespace abc {
namespace text {

/*static*/ bool utf16_str_traits::is_valid(char16_t const * psz) {
   ABC_TRACE_FUNC(psz);

   bool bExpectTrailSurrogate(false);
   while (char16_t ch = *psz++) {
      bool bSurrogate(utf16_char_traits::is_surrogate(ch));
      if (bSurrogate) {
         bool bTrailSurrogate(utf16_char_traits::is_trail_char(ch));
         // If this is a lead surrogate and we were expecting a trail, or this is a trail surrogate
         // but we’re not in a surrogate, this character is invalid.
         if (bTrailSurrogate != bExpectTrailSurrogate) {
            return false;
         }
         bExpectTrailSurrogate = !bTrailSurrogate;
      } else if (bExpectTrailSurrogate) {
         // We were expecting a trail surrogate, but this is not a surrogate at all.
         return false;
      }
   }
   // Cannot end in the middle of a surrogate.
   return !bExpectTrailSurrogate;
}
/*static*/ bool utf16_str_traits::is_valid(char16_t const * pchBegin, char16_t const * pchEnd) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   bool bExpectTrailSurrogate(false);
   for (char16_t const * pch(pchBegin); pch < pchEnd; ++pch) {
      char16_t ch(*pch);
      bool bSurrogate(utf16_char_traits::is_surrogate(ch));
      if (bSurrogate) {
         bool bTrailSurrogate(utf16_char_traits::is_trail_char(ch));
         // If this is a lead surrogate and we were expecting a trail, or this is a trail surrogate
         // but we’re not in a surrogate, this character is invalid.
         if (bTrailSurrogate != bExpectTrailSurrogate) {
            return false;
         }
         bExpectTrailSurrogate = !bTrailSurrogate;
      } else if (bExpectTrailSurrogate) {
         // We were expecting a trail surrogate, but this is not a surrogate at all.
         return false;
      }
   }
   // Cannot end in the middle of a surrogate.
   return !bExpectTrailSurrogate;
}


/*static*/ size_t utf16_str_traits::size_in_chars(char16_t const * psz) {
   ABC_TRACE_FUNC(psz);

   char16_t const * pch(psz);
   while (*pch) {
      ++pch;
   }
   return size_t(pch - psz);
}


/*static*/ size_t utf16_str_traits::size_in_codepoints(
   char16_t const * pchBegin, char16_t const * pchEnd
) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   size_t ccp(0);
   for (
      char16_t const * pch(pchBegin);
      pch < pchEnd;
      pch += utf16_char_traits::lead_char_to_codepoint_size(*pch)
   ) {
      ++ccp;
   }
   return ccp;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::host_str_traits


namespace abc {
namespace text {

/*static*/ int host_str_traits::compare(
   char_t const * pch1Begin, char_t const * pch1End,
   char_t const * pch2Begin, char_t const * pch2End
) {
   ABC_TRACE_FUNC(pch1Begin, pch1End, pch2Begin, pch2End);

   char_t const * pch1(pch1Begin), * pch2(pch2Begin);
   while (pch1 < pch1End && pch2 < pch2End) {
      char_t ch1(*pch1++), ch2(*pch2++);
#if ABC_HOST_UTF == 8
      // Note: not only don’t sequences matter when scanning for the first differing bytes, but once
      // a pair of differing bytes is found, if they are part of a sequence, its start must have
      // been the same, so only their absolute value matters; if they started a sequence, the first
      // byte of a longer encoding (greater code point value) if greater than that of a shorter one.
#elif ABC_HOST_UTF == 16 //if ABC_HOST_UTF == 8
      // Surrogates mess with the ability to just compare the absolute char16_t value.
      bool bIsSurrogate1(text::host_char_traits::is_surrogate(ch1));
      bool bIsSurrogate2(text::host_char_traits::is_surrogate(ch2));
      if (bIsSurrogate1 != bIsSurrogate2) {
         if (bIsSurrogate1) {
            // If ch1 is a surrogate and ch2 is not, ch1 > ch2.
            return +1;
         } else /*if (bIsSurrogate2)*/ {
            // If ch2 is a surrogate and ch1 is not, ch1 < ch2.
            return -1;
         }
      }
      // The characters are both regular or surrogates. Since a difference in lead surrogate
      // generates bias, we only get to compare trails if the leads were equal.
#endif //if ABC_HOST_UTF == 8 … elif ABC_HOST_UTF == 16
      if (ch1 > ch2) {
         return +1;
      } else if (ch1 < ch2) {
         return -1;
      }
   }
   // If we’re still here, the string that didn’t run out of characters wins.
   if (pch1 < pch1End) {
      return +1;
   } else if (pch2 < pch2End) {
      return -1;
   } else {
      return 0;
   }
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

