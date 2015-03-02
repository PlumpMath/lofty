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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::detail::c_str_ptr

namespace abc {
namespace text {
namespace detail {

/*! Pointer to a C-style, NUL-terminated character array that may or may not share memory with an
abc::text::*str instance. */
class c_str_ptr {
private:
   //! Internal conditionally-deleting pointer type.
   typedef std::unique_ptr<
      char_t const [], memory::conditional_deleter<char_t const [], memory::freeing_deleter>
   > pointer;

public:
   /*! Constructor.

   @param pch
      Pointer to the character array.
   @param p
      Source object.
   @param bOwn
      If true, the pointer will own the character array; if false, it won’t try to deallocate it.
   */
   c_str_ptr(char_t const * pch, bool bOwn) :
      m_p(pch, pointer::deleter_type(bOwn)) {
   }
   c_str_ptr(c_str_ptr && p) :
      m_p(std::move(p.m_p)) {
   }

   /*! Assignment operator.

   @param p
      Source object.
   @return
      *this.
   */
   c_str_ptr & operator=(c_str_ptr && p) {
      m_p = std::move(p.m_p);
      return *this;
   }

   /*! Implicit conversion to char_t const *.

   @return
      Pointer to the character array.
   */
   operator char_t const *() const {
      return m_p.get();
   }

   /*! Enables access to the internal pointer.

   @return
      Reference to the internal pointer.
   */
   pointer const & _get() const {
      return m_p;
   }

private:
   //! Conditionally-deleting pointer.
   pointer m_p;
};

} //namespace detail
} //namespace text
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::external_buffer

namespace abc {

//! See abc::external_buffer.
struct external_buffer_t {
   //! Constructor. Required to instantiate a const instance.
   external_buffer_t() {
   }
};

/*! Constant similar in use to std::nothrow; when specified as extra argument for abc::text::*str
constructors, it indicates that the string should use an external buffer that is guaranteed by the
caller to have a scope lifetime equal or longer than that of the string. */
extern ABACLADE_SYM external_buffer_t const external_buffer;

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::detail::str_base

namespace abc {
namespace text {

// Forward declarations.
class istr;
class dmstr;

namespace detail {

/*! Base class for strings. Unlike C or STL strings, instances do not implcitly have an accessible
trailing NUL character.

See [DOC:4019 abc::text::*str and abc::collections::*vector design] for implementation details for
this and all the abc::text::*str classes. */
class ABACLADE_SYM str_base :
   protected collections::detail::raw_trivial_vextr_impl,
   public support_explicit_operator_bool<str_base> {
public:
   typedef char_t value_type;
   typedef char_t * pointer;
   typedef char_t const * const_pointer;
   typedef char_t & reference;
   typedef char_t const & const_reference;
   typedef std::size_t size_type;
   typedef std::ptrdiff_t difference_type;
   typedef codepoint_iterator<false> iterator;
   typedef codepoint_iterator<true> const_iterator;
   typedef std::reverse_iterator<iterator> reverse_iterator;
   typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
   /*! Allows automatic cross-class-hierarchy casts.

   @return
      Const reference to *this as an immutable string.
   */
   operator istr const &() const;

   /*! Character access operator.

   @param i
      Character index. If outside of the [begin, end) range, an  index_error exception will be
      thrown.
   @return
      Character at index i.
   */
   detail::codepoint_proxy<true> operator[](std::ptrdiff_t i) const {
      return detail::codepoint_proxy<true>(_advance_char_ptr(chars_begin(), i, true), this);
   }

   /*! Returns true if the length is greater than 0.

   @return
      true if the string is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      /* Use std::int8_t to avoid multiplying by sizeof(char_t) when all we need is a greater-than
      check. */
      return collections::detail::raw_vextr_impl_base::end<std::int8_t>() >
         collections::detail::raw_vextr_impl_base::begin<std::int8_t>();
   }

   /*! Advances or backs up a pointer by the specified number of code points, returning the
   resulting pointer. If the pointer is moved outside of the buffer, an index_error or
   iterator_error exception (depending on bIndex) is thrown.

   @param pch
      Initial pointer.
   @param i
      Count of code points to move from pch by.
   @param bIndex
      If true, a movement to outside of [begin, end) will cause an index_error to be thrown; if
      false, a movement to outside of [begin, end] will cause an iterator_error to be thrown.
   @return
      Resulting pointer.
   */
   char_t const * _advance_char_ptr(char_t const * pch, std::ptrdiff_t i, bool bIndex) const;

   /*! Returns a forward iterator set to the first element.

   @return
      Forward iterator to the first element.
   */
   const_iterator begin() const {
      return cbegin();
   }

   /*! Returns a pointer to a NUL-terminated version of the string.

   If the string already includes a NUL terminator, the returned pointer will refer to the same
   character array, and it will not own it; if the string does not include a NUL terminator, the
   returned pointer will own a NUL-terminated copy of *this.

   The returned pointer should be thought of as having a very short lifetime, and it should never be
   stored of manipulated.

   TODO: provide non-immutable version mstr::to_c_str().

   @return
      NUL-terminated version of the string.
   */
   detail::c_str_ptr c_str() const;

   /*! Returns the maximum number of characters the string buffer can currently hold.

   @return
      Size of the string buffer, in characters.
   */
   std::size_t capacity() const {
      return collections::detail::raw_vextr_impl_base::capacity<char_t>();
   }

   /*! Returns a const forward iterator set to the first element.

   @return
      Forward iterator to the first element.
   */
   const_iterator cbegin() const {
      return const_iterator(chars_begin(), this);
   }

   /*! Returns a const forward iterator set beyond the last element.

   @return
      Forward iterator to beyond the last element.
   */
   const_iterator cend() const {
      return const_iterator(chars_end(), this);
   }

   //! See collections::detail::raw_trivial_vextr_impl::begin().
   char_t * chars_begin() {
      return collections::detail::raw_trivial_vextr_impl::begin<char_t>();
   }
   char_t const * chars_begin() const {
      return collections::detail::raw_trivial_vextr_impl::begin<char_t>();
   }

   //! See collections::detail::raw_trivial_vextr_impl::end().
   char_t * chars_end() {
      return collections::detail::raw_trivial_vextr_impl::end<char_t>();
   }
   char_t const * chars_end() const {
      return collections::detail::raw_trivial_vextr_impl::end<char_t>();
   }

   /*! Returns a const reverse iterator set to the last element.

   @return
      Reverse iterator to the last element.
   */
   const_reverse_iterator crbegin() const {
      return const_reverse_iterator(cend());
   }

   /*! Returns a const reverse iterator set to before the first element.

   @return
      Reverse iterator to before the first element.
   */
   const_reverse_iterator crend() const {
      return const_reverse_iterator(cbegin());
   }

   /*! Returns the string, encoded as requested, into a byte vector.

   @param enc
      Requested encoding.
   @param bNulT
      If true, the resulting vector will contain an additional NUL terminator (using as many vector
      elements as the destination encoding’s character size); if false, no NUL terminator will be
      present.
   @return
      Resulting byte vector.
   */
   collections::dmvector<std::uint8_t> encode(encoding enc, bool bNulT) const;

   /*! Returns a forward iterator set beyond the last element.

   @return
      Forward iterator to the first element.
   */
   const_iterator end() const {
      return cend();
   }

   /*! Returns true if the string ends with a specified suffix.

   @param s
      String that *this should end with.
   @return
      true if *this ends with the specified suffix, or false otherwise.
   */
   bool ends_with(istr const & s) const;

   /*! Searches for and returns the first occurrence of the specified character or substring.

   @param chNeedle
      Character to search for.
   @param sNeedle
      String to search for.
   @param itWhence
      Iterator to the first character whence the search should start. When not specified, it
      defaults to cbegin().
   @return
      Iterator to the first occurrence of the character/string, or cend() when no matches are found.
   */
   const_iterator find(char_t chNeedle) const {
      return find(chNeedle, cbegin());
   }
#if ABC_HOST_UTF > 8
   const_iterator find(char chNeedle) const {
      return find(host_char(chNeedle));
   }
#endif
   const_iterator find(char32_t chNeedle) const {
      return find(chNeedle, cbegin());
   }
   const_iterator find(char_t chNeedle, const_iterator itWhence) const;
#if ABC_HOST_UTF > 8
   const_iterator find(char chNeedle, const_iterator itWhence) const {
      return find(host_char(chNeedle), itWhence);
   }
#endif
   const_iterator find(char32_t chNeedle, const_iterator itWhence) const;
   const_iterator find(istr const & sNeedle) const {
      return find(sNeedle, cbegin());
   }
   const_iterator find(istr const & sNeedle, const_iterator itWhence) const;

   /*! Searches for and returns the last occurrence of the specified character or substring.

   @param chNeedle
      Character to search for.
   @param sNeedle
      String to search for.
   @param itWhence
      Iterator to the last character whence the search should start. When not specified, it
      defaults to cend().
   @return
      Iterator to the first occurrence of the character/string, or cend() when no matches are found.
   */
   const_iterator find_last(char_t chNeedle) const {
      return find_last(chNeedle, cend());
   }
#if ABC_HOST_UTF > 8
   const_iterator find_last(char chNeedle) const {
      return find_last(host_char(chNeedle));
   }
#endif
   const_iterator find_last(char32_t chNeedle) const {
      return find_last(chNeedle, cend());
   }
   const_iterator find_last(char_t chNeedle, const_iterator itWhence) const;
#if ABC_HOST_UTF > 8
   const_iterator find_last(char chNeedle, const_iterator itWhence) const {
      return find_last(host_char(chNeedle), itWhence);
   }
#endif
   const_iterator find_last(char32_t chNeedle, const_iterator itWhence) const;
   const_iterator find_last(istr const & sNeedle) const {
      return find_last(sNeedle, cend());
   }
   const_iterator find_last(istr const & sNeedle, const_iterator itWhence) const;

   /*! Uses the current content of the string to generate a new one using io::text::writer::print().

   @param ts
      Replacement values.
   @return
      Resulting string.
   */
#ifdef ABC_CXX_VARIADIC_TEMPLATES
   template <typename... Ts>
   dmstr format(Ts const &... ts) const;
#else //ifdef ABC_CXX_VARIADIC_TEMPLATES
   dmstr format() const;
   template <typename T0>
   dmstr format(T0 const & t0) const;
   template <typename T0, typename T1>
   dmstr format(T0 const & t0, T1 const & t1) const;
   template <typename T0, typename T1, typename T2>
   dmstr format(T0 const & t0, T1 const & t1, T2 const & t2) const;
   template <typename T0, typename T1, typename T2, typename T3>
   dmstr format(T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3) const;
   template <typename T0, typename T1, typename T2, typename T3, typename T4>
   dmstr format(T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4) const;
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
   dmstr format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
   >
   dmstr format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7
   >
   dmstr format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8
   >
   dmstr format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8, typename T9
   >
   dmstr format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
   ) const;
#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

   /*! Converts a character index into its corresponding code point index.

   @param ich
      Character index. No validation is performed on it.
   @return
      Code point index. If ich is not a valid character index for the string, the return value is
      undefined.
   */
   std::size_t index_from_char_index(std::size_t ich) const {
      return str_traits::size_in_codepoints(chars_begin(), chars_begin() + ich);
   }

   /*! Returns a reverse iterator set to the last element.

   @return
      Reverse iterator to the last element.
   */
   const_reverse_iterator rbegin() const {
      return crbegin();
   }

   /*! Returns a reverse iterator set to before the first element.

   @return
      Reverse iterator to before the first element.
   */
   const_reverse_iterator rend() const {
      return crend();
   }

   /*! Returns size of the string, in code points.

   @return
      Size of the string.
   */
   std::size_t size() const {
      return str_traits::size_in_codepoints(chars_begin(), chars_end());
   }

   /*! Returns size of the string, in bytes.

   @return
      Size of the string.
   */
   std::size_t size_in_bytes() const {
      return collections::detail::raw_trivial_vextr_impl::size<std::int8_t>();
   }

   /*! Returns size of the string, in characters.

   @return
      Size of the string.
   */
   std::size_t size_in_chars() const {
      return collections::detail::raw_trivial_vextr_impl::size<char_t>();
   }

   /*! Returns true if the string starts with a specified prefix.

   @param s
      String that *this should start with.
   @return
      true if *this starts with the specified suffix, or false otherwise.
   */
   bool starts_with(istr const & s) const;

   /*! Returns a portion of the string.

   @param ichBegin
      Index of the first character of the substring. See str_base::translate_range() for allowed
      begin index values.
   @param ichEnd
      Index of the last character of the substring, exclusive. See str_base::translate_range() for
      allowed end index values.
   @param itBegin
      Iterator to the first character of the substring.
   @param itEnd
      Iterator to past the end of the substring.
   @return
      Substring of *this.
   */
   dmstr substr(std::ptrdiff_t ichBegin) const;
   dmstr substr(std::ptrdiff_t ichBegin, std::ptrdiff_t ichEnd) const;
   dmstr substr(const_iterator itBegin) const;
   dmstr substr(const_iterator itBegin, const_iterator itEnd) const;

protected:
   /*! Constructor.

   @param cbEmbeddedCapacity
      Size of the embedded character array, in bytes, or 0 if no embedded array is present.
   @param pchConstSrc
      Pointer to a string that will be adopted by the str_base as read-only.
   @param cchSrc
      Count of characters in the string pointed to by pchConstSrc.
   @param bNulT
      true if the array pointed to by pchConstSrc is a NUL-terminated string, or false otherwise.
   */
   str_base(std::size_t cbEmbeddedCapacity) :
      collections::detail::raw_trivial_vextr_impl(cbEmbeddedCapacity) {
   }
   str_base(char_t const * pchConstSrc, std::size_t cchSrc, bool bNulT) :
      collections::detail::raw_trivial_vextr_impl(pchConstSrc, pchConstSrc + cchSrc, bNulT) {
   }

   /*! See collections::detail::raw_trivial_vextr_impl::assign_copy().

   @param pchBegin
      Pointer to the start of the source string.
   @param pchEnd
      Pointer to the end of the source string.
   */
   void assign_copy(char_t const * pchBegin, char_t const * pchEnd) {
      collections::detail::raw_trivial_vextr_impl::assign_copy(pchBegin, pchEnd);
   }

   /*! See collections::detail::raw_trivial_vextr_impl::assign_concat().

   @param pch1Begin
      Pointer to the start of the first source string.
   @param pch1End
      Pointer to the end of the first source string.
   @param pch2Begin
      Pointer to the start of the second source string.
   @param pch2End
      Pointer to the end of the second source string.
   */
   void assign_concat(
      char_t const * pch1Begin, char_t const * pch1End,
      char_t const * pch2Begin, char_t const * pch2End
   ) {
      collections::detail::raw_trivial_vextr_impl::assign_concat(
         pch1Begin, pch1End, pch2Begin, pch2End
      );
   }

   /*! See collections::detail::raw_trivial_vextr_impl::assign_move().

   @param s
      Source string.
   */
   void assign_move(str_base && s) {
      collections::detail::raw_trivial_vextr_impl::assign_move(
         static_cast<collections::detail::raw_trivial_vextr_impl &&>(s)
      );
   }

   /*! See collections::detail::raw_trivial_vextr_impl::assign_move_dynamic_or_move_items().

   @param s
      Source string.
   */
   void assign_move_dynamic_or_move_items(str_base && s) {
      collections::detail::raw_trivial_vextr_impl::assign_move_dynamic_or_move_items(
         static_cast<collections::detail::raw_trivial_vextr_impl &&>(s)
      );
   }

   /*! See collections::detail::raw_trivial_vextr_impl::assign_share_raw_or_copy_desc().

   @param s
      Source string.
   */
   void assign_share_raw_or_copy_desc(str_base const & s) {
      collections::detail::raw_trivial_vextr_impl::assign_share_raw_or_copy_desc(s);
   }

   /*! Converts a possibly negative character index into an iterator.

   @param ich
      If positive, this is interpreted as a 0-based index; if negative, it’s interpreted as a
      1-based index from the end of the character array by adding this->size() to it.
   @return
      Resulting iterator.
   */
   const_iterator translate_index(std::ptrdiff_t ich) const;

   /*! Converts a left-closed, right-open interval with possibly negative character indices into one
   consisting of two iterators.

   @param ichBegin
      Left endpoint of the interval, inclusive. If positive, this is interpreted as a 0-based index;
      if negative, it’s interpreted as a 1-based index from the end of the character array by adding
      this->size() to it.
   @param ichEnd
      Right endpoint of the interval, exclusive. If positive, this is interpreted as a 0-based
      index; if negative, it’s interpreted as a 1-based index from the end of the character array by
      adding this->size() to it.
   @return
      Left-closed, right-open interval such that return.first <= i < return.second, or the empty
      interval [end(), end()) if the indices represent an empty interval after being adjusted.
   */
   std::pair<const_iterator, const_iterator> translate_range(
      std::ptrdiff_t ichBegin, std::ptrdiff_t ichEnd
   ) const;
};

// Relational operators.
#define ABC_RELOP_IMPL(op) \
   inline bool operator op(str_base const & s1, str_base const & s2) { \
      return str_traits::compare( \
         s1.chars_begin(), s1.chars_end(), s2.chars_begin(), s2.chars_end() \
      ) op 0; \
   } \
   template <std::size_t t_cch> \
   inline bool operator op(str_base const & s, char_t const (& ach)[t_cch]) { \
      char_t const * pchEnd = ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'); \
      return str_traits::compare(s.chars_begin(), s.chars_end(), ach, pchEnd) op 0; \
   } \
   template <std::size_t t_cch> \
   inline bool operator op(char_t const (& ach)[t_cch], str_base const & s) { \
      char_t const * pchEnd = ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'); \
      return str_traits::compare(ach, pchEnd, s.chars_begin(), s.chars_end()) op 0; \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL

} //namespace detail
} //namespace text
} //namespace abc

namespace std {

// Specialization of std::hash.
template <>
struct ABACLADE_SYM hash<abc::text::detail::str_base> {
   std::size_t operator()(abc::text::detail::str_base const & s) const;
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::istr

namespace abc {
namespace text {

// Forward declaration.
class mstr;

/*! Class to be used as “the” string class in most cases. It cannot be modified in-place, which
means that it shouldn’t be used in code performing intensive string manipulations.
*/
class ABACLADE_SYM istr : public detail::str_base {
public:
   /*! Constructor.

   @param s
      Source string.
   @param ach
      Source NUL-terminated string literal.
   @param pchBegin
      Pointer to the beginning of the source stirng.
   @param pchEnd
      Pointer to the end of the source stirng.
   @param psz
      Pointer to the source NUL-terminated string literal.
   @param cch
      Count of characters in the array pointed to be psz.
   */
   istr() :
      detail::str_base(0) {
   }
   istr(istr const & s) :
      detail::str_base(0) {
      assign_share_raw_or_copy_desc(s);
   }
   istr(istr && s) :
      detail::str_base(0) {
      // Non-const, so it can’t be anything but a real istr, so it owns its character array.
      assign_move(std::move(s));
   }
   // This can throw exceptions, but it’s allowed to since it’s not the istr && overload.
   istr(mstr && s);
   istr(dmstr && s);
   template <std::size_t t_cch>
   istr(char_t const (& ach)[t_cch]) :
      detail::str_base(
         ach, t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'), ach[t_cch - 1 /*NUL*/] == '\0'
      ) {
   }
   istr(char_t const * pchBegin, char_t const * pchEnd) :
      detail::str_base(0) {
      assign_copy(pchBegin, pchEnd);
   }
   istr(external_buffer_t const &, char_t const * psz) :
      detail::str_base(psz, text::size_in_chars(psz), true) {
   }
   istr(external_buffer_t const &, char_t const * psz, std::size_t cch) :
      detail::str_base(psz, cch, false) {
   }

   /*! Assignment operator.

   @param s
      Source string.
   @param ach
      Source NUL-terminated string literal.
   @return
      *this.
   */
   istr & operator=(istr const & s) {
      assign_share_raw_or_copy_desc(s);
      return *this;
   }
   istr & operator=(istr && s) {
      // Non-const, so it can’t be anything but a real istr, so it owns its character array.
      assign_move(std::move(s));
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the istr && overload.
   istr & operator=(mstr && s);
   istr & operator=(dmstr && s);
   template <std::size_t t_cch>
   istr & operator=(char_t const (& ach)[t_cch]) {
      // This order is safe, because the constructor invoked on the next line won’t throw.
      this->~istr();
      ::new(this) istr(ach);
      return *this;
   }

public:
   //! Empty string constant.
   static istr const & empty;
};


// Now this can be defined.

namespace detail {

inline str_base::operator istr const &() const {
   return *static_cast<istr const *>(this);
}

} //namespace detail

} //namespace text
} //namespace abc

namespace std {

// Specialization of std::hash.
template <>
struct hash<abc::text::istr> : public hash<abc::text::detail::str_base> {};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::mstr

namespace abc {
namespace text {

/*! Class to be used as “the“ string class for arguments of functions that want to modify a string
argument, since unlike istr, it allows in-place alterations to the string. Both smstr and dmstr
are automatically converted to this. */
class ABACLADE_SYM mstr : public detail::str_base {
private:
   friend detail::codepoint_proxy<false> & detail::codepoint_proxy<false>::operator=(char_t ch);
   friend detail::codepoint_proxy<false> & detail::codepoint_proxy<false>::operator=(char32_t ch);

public:
   /*! Assignment operator.

   @param s
      Source string.
   @return
      *this.
   */
   mstr & operator=(mstr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   mstr & operator=(istr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the mstr && overload.
   mstr & operator=(istr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   mstr & operator=(dmstr && s);

   /*! Concatenation-assignment operator.

   @param ch
      Character to append.
   @param s
      String to append.
   @return
      *this.
   */
   mstr & operator+=(char_t ch) {
      append(&ch, 1);
      return *this;
   }
#if ABC_HOST_UTF > 8
   mstr & operator+=(char ch) {
      return operator+=(host_char(ch));
   }
#endif
   mstr & operator+=(char32_t ch) {
      char_t ach[host_char_traits::max_codepoint_length];
      append(ach, static_cast<std::size_t>(host_char_traits::codepoint_to_chars(ch, ach) - ach));
      return *this;
   }
   mstr & operator+=(istr const & s) {
      append(s.chars_begin(), s.size_in_chars());
      return *this;
   }

   /*! Same as operator+=(), but for multi-argument overloads.

   @param pchAdd
      Pointer to an array of characters to append.
   @param cchAdd
      Count of characters in the array pointed to by pchAdd.
   */
   void append(char_t const * pchAdd, std::size_t cchAdd) {
      collections::detail::raw_trivial_vextr_impl::insert_remove(
         collections::detail::raw_vextr_impl_base::size<std::int8_t>(),
         pchAdd, sizeof(char_t) * cchAdd, 0
      );
   }

   //! See detail::str_base::begin(). Here also available in non-const overload.
   iterator begin() {
      return iterator(chars_begin(), this);
   }
   using detail::str_base::begin;

   //! Truncates the string to zero length, without deallocating the internal buffer.
   void clear() {
      set_size(0);
   }

   //! See detail::str_base::end(). Here also available in non-const overload.
   iterator end() {
      return iterator(chars_end(), this);
   }
   using detail::str_base::end;

   /*! Inserts characters into the string at a specific character (not code point) offset.

   @param ichOffset
      0-based offset at which to insert the characters.
   @param ch
      Character to insert.
   @param s
      String to insert.
   @param pchInsert
      Pointer to an array of characters to insert.
   @param cchInsert
      Count of characters in the array pointed to by pchInsert.
   */
   void insert(std::size_t ichOffset, char_t ch) {
      insert(ichOffset, &ch, 1);
   }
#if ABC_HOST_UTF > 8
   void insert(std::size_t ichOffset, char ch) {
      insert(ichOffset, host_char(ch));
   }
#endif
   void insert(std::size_t ichOffset, char32_t ch) {
      char_t ach[host_char_traits::max_codepoint_length];
      insert(
         ichOffset, ach,
         static_cast<std::size_t>(host_char_traits::codepoint_to_chars(ch, ach) - ach)
      );
   }
   void insert(std::size_t ichOffset, istr const & s) {
      insert(ichOffset, s.chars_begin(), s.size_in_chars());
   }
   void insert(std::size_t ichOffset, char_t const * pchInsert, std::size_t cchInsert) {
      collections::detail::raw_trivial_vextr_impl::insert_remove(
         sizeof(char_t) * ichOffset, pchInsert, sizeof(char_t) * cchInsert, 0
      );
   }

   //! See detail::str_base::rbegin(). Here also available in non-const overload.
   reverse_iterator rbegin() {
      return reverse_iterator(iterator(chars_end(), this));
   }
   using detail::str_base::rbegin;

   //! See detail::str_base::rend(). Here also available in non-const overload.
   reverse_iterator rend() {
      return reverse_iterator(iterator(chars_begin(), this));
   }
   using detail::str_base::rend;

   /*! Replaces all occurrences of a character with another character.

   @param chSearch
      Character to search for.
   @param chReplacement
      Character to replace chSearch with.
   */
   void replace(char_t chSearch, char_t chReplacement);
#if ABC_HOST_UTF > 8
   void replace(char chSearch, char chReplacement) {
      replace(host_char(chSearch), host_char(chReplacement));
   }
#endif
   void replace(char32_t chSearch, char32_t chReplacement);

   /*! See collections::detail::raw_trivial_vextr_impl::set_capacity().

   @param cchMin
      Minimum count of characters requested.
   @param bPreserve
      If true, the previous contents of the string will be preserved even if the reallocation
      causes the string to switch to a different character array.
   */
   void set_capacity(std::size_t cchMin, bool bPreserve) {
      collections::detail::raw_trivial_vextr_impl::set_capacity(sizeof(char_t) * cchMin, bPreserve);
   }

   /*! Expands the character array until the specified callback succeeds in filling it and returns a
   number of needed characters that’s less than the size of the buffer. For example, for cchMax == 3
   (NUL terminator included), it must return <= 2 (NUL excluded).

   This method is not transaction-safe; if an exception is thrown in the callback or elsewhere,
   *this will not be restored to its previous state.

   TODO: maybe improve exception resilience? Check typical usage to see if it’s an issue.

   @param fnRead
      Callback that is invoked to fill up the string buffer.
      pch
         Pointer to the beginning of the buffer to be filled up by the callback.
      cchMax
         Size of the buffer pointed to by pch.
      return
         Count of characters written to the buffer pointed to by pch. If less than cchMax, this will
         be the final count of characters of *this; otherwise, fnRead will be called once more with
         a larger cchMax after the string buffer has been enlarged.
   */
   void set_from(std::function<std::size_t (char_t * pch, std::size_t cchMax)> const & fnRead);

   /*! Changes the length of the string. If the string needs to be lengthened, the added characters
   will be left uninitialized.

   @param cch
      New length of the string.
   @param bClear
      If true, the string will be cleared after being resized; if false, no characters will be
      changed.
   */
   void set_size_in_chars(std::size_t cch, bool bClear = false) {
      collections::detail::raw_trivial_vextr_impl::set_size(sizeof(char_t) * cch);
      if (bClear) {
         memory::clear(chars_begin(), cch);
      }
   }

protected:
   //! See detail::str_base::str_base().
   mstr(std::size_t cbEmbeddedCapacity) :
      detail::str_base(cbEmbeddedCapacity) {
   }

   /*! Replaces a single code point with another single code point.

   @param pch
      Pointer to the start of the code point to replace.
   @param chNew
      Character or code point that will be written at *pch.
   */
   void _replace_codepoint(char_t * pch, char_t chNew);
#if ABC_HOST_UTF > 8
   void _replace_codepoint(char_t * pch, char chNew) {
      _replace_codepoint(pch, host_char(chNew));
   }
#endif
   void _replace_codepoint(char_t * pch, char32_t chNew);
};


// Now these can be defined.

inline istr::istr(mstr && s) :
   detail::str_base(0) {
   assign_move_dynamic_or_move_items(std::move(s));
}

inline istr & istr::operator=(mstr && s) {
   assign_move_dynamic_or_move_items(std::move(s));
   return *this;
}

} //namespace text
} //namespace abc

namespace std {

// Specialization of std::hash.
template <>
struct hash<abc::text::mstr> : public hash<abc::text::detail::str_base> {};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::dmstr

namespace abc {
namespace text {

/*! mstr-derived class, good for clients that need in-place manipulation of strings whose length is
unknown at design time. */
class dmstr : public mstr {
public:
   /*! Constructor.

   @param s
      Source string.
   @param ach
      Source NUL-terminated string literal.
   @param pchBegin
      Pointer to the beginning of the source stirng.
   @param pchEnd
      Pointer to the end of the source stirng.
   @param pch1Begin
      Pointer to the beginning of the left source stirng to concatenate.
   @param pch1End
      Pointer to the end of the left source stirng.
   @param pch2Begin
      Pointer to the beginning of the right source stirng to concatenate.
   @param pch2End
      Pointer to the end of the right source stirng.
   */
   dmstr() :
      mstr(0) {
   }
   dmstr(dmstr const & s) :
      mstr(0) {
      assign_copy(s.chars_begin(), s.chars_end());
   }
   dmstr(dmstr && s) :
      mstr(0) {
      assign_move(std::move(s));
   }
   dmstr(istr const & s) :
      mstr(0) {
      assign_copy(s.chars_begin(), s.chars_end());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr(istr && s) :
      mstr(0) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   dmstr(mstr const & s) :
      mstr(0) {
      assign_copy(s.chars_begin(), s.chars_end());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr(mstr && s) :
      mstr(0) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   template <std::size_t t_cch>
   dmstr(char_t const (& ach)[t_cch]) :
      mstr(0) {
      assign_copy(ach, ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'));
   }
   dmstr(char_t const * pchBegin, char_t const * pchEnd) :
      mstr(0) {
      assign_copy(pchBegin, pchEnd);
   }
   dmstr(
      char_t const * pch1Begin, char_t const * pch1End,
      char_t const * pch2Begin, char_t const * pch2End
   ) :
      mstr(0) {
      assign_concat(pch1Begin, pch1End, pch2Begin, pch2End);
   }

   /*! Assignment operator.

   @param s
      Source string.
   @param ach
      Source NUL-terminated string literal.
   @return
      *this.
   */
   dmstr & operator=(dmstr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   dmstr & operator=(dmstr && s) {
      assign_move(std::move(s));
      return *this;
   }
   dmstr & operator=(istr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr & operator=(istr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   dmstr & operator=(mstr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr & operator=(mstr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   template <std::size_t t_cch>
   dmstr & operator=(char_t const (& ach)[t_cch]) {
      assign_copy(ach, ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'));
      return *this;
   }

#if ABC_HOST_CXX_MSC
   /*! MSC16 BUG: re-defined here because MSC16 seems unable to see the definition in
   detail::str_base. See detail::str_base::operator istr const &(). */
   operator istr const &() const {
      return detail::str_base::operator istr const &();
   }
#endif //if ABC_HOST_CXX_MSC
};


// Now these can be defined.

namespace detail {

inline dmstr str_base::substr(std::ptrdiff_t ichBegin) const {
   return substr(ichBegin, static_cast<std::ptrdiff_t>(size_in_chars()));
}
inline dmstr str_base::substr(std::ptrdiff_t ichBegin, std::ptrdiff_t ichEnd) const {
   auto range(translate_range(ichBegin, ichEnd));
   return dmstr(range.first.base(), range.second.base());
}
inline dmstr str_base::substr(const_iterator itBegin) const {
   validate_pointer(itBegin.base());
   return dmstr(itBegin.base(), chars_end());
}
inline dmstr str_base::substr(const_iterator itBegin, const_iterator itEnd) const {
   validate_pointer(itBegin.base());
   validate_pointer(itEnd.base());
   return dmstr(itBegin.base(), itEnd.base());
}

} //namespace detail

inline istr::istr(dmstr && s) :
   detail::str_base(0) {
   assign_move(std::move(s));
}

inline istr & istr::operator=(dmstr && s) {
   assign_move(std::move(s));
   return *this;
}

inline mstr & mstr::operator=(dmstr && s) {
   assign_move(std::move(s));
   return *this;
}


/*! Concatenation operator.

@param sL
   Left string operand.
@param sR
   Right string operand.
@param chL
   Left character operand.
@param chR
   Right character operand.
@return
   Resulting string.
*/
inline dmstr operator+(istr const & sL, istr const & sR) {
   return dmstr(sL.chars_begin(), sL.chars_end(), sR.chars_begin(), sR.chars_end());
}
// Overloads taking a character literal.
inline dmstr operator+(istr const & sL, char_t chR) {
   return dmstr(sL.chars_begin(), sL.chars_end(), &chR, &chR + 1);
}
#if ABC_HOST_UTF > 8
inline dmstr operator+(istr const & sL, char chR) {
   return operator+(sL, host_char(chR));
}
#endif
inline dmstr operator+(istr const & sL, char32_t chR) {
   char_t achR[host_char_traits::max_codepoint_length];
   return dmstr(
      sL.chars_begin(), sL.chars_end(), achR, host_char_traits::codepoint_to_chars(chR, achR)
   );
}
inline dmstr operator+(char_t chL, istr const & sR) {
   return dmstr(&chL, &chL + 1, sR.chars_begin(), sR.chars_end());
}
#if ABC_HOST_UTF > 8
inline dmstr operator+(char chL, istr const & sR) {
   return operator+(host_char(chL), sR);
}
#endif
inline dmstr operator+(char32_t chL, istr const & sR) {
   char_t achL[host_char_traits::max_codepoint_length];
   return dmstr(
      achL, host_char_traits::codepoint_to_chars(chL, achL),
      sR.chars_begin(), sR.chars_end()
   );
}
/* Overloads taking a temporary string as left or right operand; they can avoid creating an
intermediate string. */
inline dmstr operator+(istr && sL, char_t chR) {
   dmstr dmsL(std::move(sL));
   dmsL += chR;
   return std::move(dmsL);
}
/*inline dmstr operator+(char_t chL, istr && sR) {
   dmstr dmsR(std::move(sR));
   dmsR.insert(0, chL);
   return std::move(dmsR);
}*/
#if ABC_HOST_UTF > 8
inline dmstr operator+(istr && sL, char chR) {
   return operator+(std::move(sL), host_char(chR));
}
/*inline dmstr operator+(char chL, istr && sR) {
   return operator+(host_char(chL), std::move(sR));
}*/
#endif
inline dmstr operator+(istr && sL, char32_t chR) {
   dmstr dmsL(std::move(sL));
   dmsL += chR;
   return std::move(dmsL);
}
/*inline dmstr operator+(char32_t chL, istr && sR) {
   dmstr dmsR(std::move(sR));
   dmsR.insert(0, chL);
   return std::move(dmsR);
}*/
inline dmstr operator+(istr && sL, istr const & sR) {
   dmstr dmsL(std::move(sL));
   dmsL += sR;
   return std::move(dmsL);
}
/*inline dmstr operator+(istr const & sL, istr && sR) {
   dmstr dmsR(std::move(sR));
   dmsR.insert(0, sL);
   return std::move(dmsR);
}*/
inline dmstr operator+(mstr && sL, char_t chR) {
   dmstr dmsL(std::move(sL));
   dmsL += chR;
   return std::move(dmsL);
}
/*inline dmstr operator+(char_t chL, mstr && sR) {
   dmstr dmsR(std::move(sR));
   dmsR.insert(0, chL);
   return std::move(dmsR);
}*/
#if ABC_HOST_UTF > 8
inline dmstr operator+(mstr && sL, char chR) {
   return operator+(std::move(sL), host_char(chR));
}
/*inline dmstr operator+(char chL, mstr && sR) {
   return operator+(host_char(chL), std::move(sR));
}*/
#endif
inline dmstr operator+(mstr && sL, char32_t chR) {
   dmstr dmsL(std::move(sL));
   dmsL += chR;
   return std::move(dmsL);
}
/*inline dmstr operator+(char32_t chL, mstr && sR) {
   dmstr dmsR(std::move(sR));
   dmsR.insert(0, chL);
   return std::move(dmsR);
}*/
inline dmstr operator+(mstr && sL, istr const & sR) {
   dmstr dmsL(std::move(sL));
   dmsL += sR;
   return std::move(dmsL);
}
/*inline dmstr operator+(istr const & sL, mstr && sR) {
   dmstr dmsR(std::move(sR));
   dmsR.insert(0, sL);
   return std::move(dmsR);
}*/

} //namespace text
} //namespace abc

namespace std {

// Specialization of std::hash.
template <>
struct hash<abc::text::dmstr> : public hash<abc::text::detail::str_base> {};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::smstr

namespace abc {
namespace text {

/*! mstr-derived class, good for clients that need in-place manipulation of strings that are most
likely to be shorter than a known small size. */
template <std::size_t t_cchEmbeddedCapacity>
class smstr :
   public mstr,
   private collections::detail::raw_vextr_prefixed_item_array<char_t, t_cchEmbeddedCapacity> {
private:
   using collections::detail::raw_vextr_prefixed_item_array<
      char_t, t_cchEmbeddedCapacity
   >::smc_cbEmbeddedCapacity;

public:
   /*! Constructor.

   @param s
      Source string.
   @param ach
      Source NUL-terminated string literal.
   */
   smstr() :
      mstr(smc_cbEmbeddedCapacity) {
   }
   smstr(smstr const & s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_copy(s.chars_begin(), s.chars_end());
   }
   /* If the source is using its embedded character array, it will be copied without allocating a
   dynamic one; if the source is dynamic, it will be moved. Either way, this won’t throw. */
   smstr(smstr && s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   smstr(istr const & s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_copy(s.chars_begin(), s.chars_end());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   smstr(istr && s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   /* This can throw exceptions, but it’s allowed to since it’s not the smstr && overload. This also
   covers smstr of different template arguments. */
   smstr(mstr && s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   smstr(dmstr && s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_move(std::move(s));
   }
   template <std::size_t t_cch>
   smstr(char_t const (& ach)[t_cch]) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_copy(ach, ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'));
   }

   /*! Assignment operator.

   @param s
      Source string.
   @param ach
      Source NUL-terminated string literal.
   @return
      *this.
   */
   smstr & operator=(smstr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   /* If the source is using its embedded character array, it will be copied without allocating a
   dynamic one; if the source is dynamic, it will be moved. Either way, this won’t throw. */
   smstr & operator=(smstr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   smstr & operator=(istr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   smstr & operator=(istr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   /* This can throw exceptions, but it’s allowed to since it’s not the smstr && overload. This also
   covers smstr of different template arguments. */
   smstr & operator=(mstr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   smstr & operator=(dmstr && s) {
      assign_move(std::move(s));
      return *this;
   }
   template <std::size_t t_cch>
   smstr & operator=(char_t const (& ach)[t_cch]) {
      assign_copy(ach, ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'));
      return *this;
   }
};

} //namespace text
} //namespace abc

namespace std {

// Specialization of std::hash.
template <std::size_t t_cchEmbeddedCapacity>
struct hash<abc::text::smstr<t_cchEmbeddedCapacity>> : public hash<abc::text::detail::str_base> {};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////