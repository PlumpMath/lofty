﻿/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2010, 2011, 2012, 2013
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef ABC_STRING_HXX
#define ABC_STRING_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/_vextr.hxx>
#include <abc/utf_traits.hxx>
#include <functional>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_str_to_str_backend_base


namespace abc {

/** Base class for the specializations of to_str_backend for string types. Not using templates, so
the implementation can be in a cxx file.
*/
class _str_to_str_backend_base {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	_str_to_str_backend_base(char_range const & crFormat);


protected:

	/** Writes the contents of the str, applying the specified format.

	TODO: comment signature.
	*/
	void write(void const * p, size_t cb, text::encoding enc, ostream * posOut);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_str_to_str_backend


namespace abc {

/** Mid-class for the specializations of to_str_backend for string types. This is used by string
literal types as well (see to_str_backend.hxx).
*/
template <typename T, typename C>
class _str_to_str_backend :
	public _str_to_str_backend_base {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	_str_to_str_backend(char_range const & crFormat) :
		_str_to_str_backend_base(crFormat) {
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::char_range_


namespace abc {

// Specialization of to_str_backend.
template <typename C>
class to_str_backend<char_range_<C>> :
	public _str_to_str_backend<char_range_<C>, C> {

	typedef _str_to_str_backend<char_range_<C>, C> str_to_str_backend;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_str_backend(char_range const & crFormat = char_range()) :
		str_to_str_backend(crFormat) {
	}


	/** See to_str_backend::write().
	*/
	void write(char_range_<C> const & cr, ostream * posOut) {
		str_to_str_backend::write(
			cr.cbegin().base(), sizeof(C) * cr.size(), text::utf_traits<C>::host_encoding, posOut
		);
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_str


namespace abc {

/** Template-independent methods of str_.
*/
class _raw_str :
	public _raw_trivial_vextr_impl {

	ABC_CLASS_PREVENT_COPYING(_raw_str)

public:

	/** Returns the current size of the str buffer, in characters, minus room for the trailing NUL
	terminator.

	TODO: comment signature.
	*/
	size_t capacity() const {
		return _raw_trivial_vextr_impl::capacity(true);
	}


	/** Returns the current length of the string, in characters, excluding the trailing NUL
	terminator.

	TODO: comment signature.
	*/
	size_t size() const {
		return _raw_trivial_vextr_impl::size(true);
	}


	/** Computes the hash value of the string.

	TODO: comment signature.
	*/
	size_t hash(size_t cbItem) const;


	/** See _raw_trivial_vextr_impl::set_capacity().

	TODO: comment signature.
	*/
	void set_capacity(size_t cbItem, size_t cchMin, bool bPreserve) {
		_raw_trivial_vextr_impl::set_capacity(cbItem, cchMin, bPreserve, true);
	}


	/** Changes the length of the string, without changing its capacity.

	TODO: comment signature.
	*/
	void set_size(size_t cbItem, size_t cch);


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	_raw_str(size_t cchStaticMax) :
		_raw_trivial_vextr_impl(cchStaticMax, true) {
	}
	_raw_str(void const * pConstSrc, size_t cchSrc) :
		_raw_trivial_vextr_impl(pConstSrc, cchSrc + 1 /*NUL*/) {
	}


	/** See _raw_trivial_vextr_impl::adjust_index().

	TODO: comment signature.
	*/
	size_t adjust_index(ptrdiff_t i) const {
		return _raw_trivial_vextr_impl::adjust_index(i, true);
	}


	/** See _raw_trivial_vextr_impl::adjust_range().

	TODO: comment signature.
	*/
	void adjust_range(ptrdiff_t * piFirst, ptrdiff_t * pci) const {
		_raw_trivial_vextr_impl::adjust_range(piFirst, pci, true);
	}


	/** See _raw_trivial_vextr_impl::assign_copy().

	TODO: comment signature.
	*/
	void assign_copy(size_t cbItem, void const * p, size_t ci) {
		_raw_trivial_vextr_impl::assign_copy(cbItem, p, ci, true);
	}
	void assign_copy(size_t cbItem, void const * p1, size_t ci1, void const * p2, size_t ci2) {
		_raw_trivial_vextr_impl::assign_copy(cbItem, p1, ci1, p2, ci2, true);
	}


	/** See _raw_trivial_vextr_impl::assign_move().

	TODO: comment signature.
	*/
	void assign_move(_raw_str && rs) {
		_raw_trivial_vextr_impl::assign_move(static_cast<_raw_trivial_vextr_impl &&>(rs), true);
	}


	/** See _raw_trivial_vextr_impl::assign_move_dynamic_or_copy().

	TODO: comment signature.
	*/
	void assign_move_dynamic_or_copy(size_t cbItem, _raw_str && rs) {
		_raw_trivial_vextr_impl::assign_move_dynamic_or_copy(
			cbItem, static_cast<_raw_trivial_vextr_impl &&>(rs), true
		);
	}


	/** See _raw_trivial_vextr_impl::assign_share_ro_or_copy().

	TODO: comment signature.
	*/
	void assign_share_ro_or_copy(size_t cbItem, _raw_str const & rs) {
		_raw_trivial_vextr_impl::assign_share_ro_or_copy(cbItem, rs, true);
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::str_base_


namespace abc {

// Forward declarations.
template <typename C, class TTraits = text::utf_traits<C>>
class istr_;
template <typename C, class TTraits = text::utf_traits<C>>
class dmstr_;

/** Base class for strings. It behaves like a vector with a last NUL element hidden from clients; an
empty string always has an accessible trailing NUL character.

Methods that take an array of characters whose length is obtained by its size instead of e.g.
strlen(), will discard the last element, asserting that it’s the NUL terminator. See [DOC:4019
abc::*str_ and abc::*vector design] for implementation details for this and all the *str_ classes.
*/
template <typename C, class TTraits>
class str_base_ :
	protected _raw_str,
	public _iterable_vector<str_base_<C, TTraits>, C>,
	public support_explicit_operator_bool<str_base_<C, TTraits>> {

	ABC_CLASS_PREVENT_COPYING(str_base_)

	typedef istr_<C, TTraits> istr;
	typedef dmstr_<C, TTraits> dmstr;

protected:

	/** Shortcut for the base class providing iterator-based types and methods. */
	typedef _iterable_vector<str_base_<C, TTraits>, C> itvec;


public:

	/** Character type. */
	typedef C char_t;
	/** See _iterable_vector::const_iterator. */
	typedef typename itvec::const_iterator const_iterator;


public:

	/** Character access operator.

	TODO: comment signature.
	*/
	C operator[](size_t i) const {
		if (i > size()) {
			abc_throw(index_error(intptr_t(i)));
		}
		return data()[i];
	}


	/** Returns true if the length is greater than 0.

	TODO: comment signature.
	*/
	explicit_operator_bool() const {
		return size() > 0;
	}


	/** Support for relational operators.

	TODO: comment signature.
	*/
	int compare_to(istr const & s) const {
		return TTraits::str_cmp(data(), size(), s.data(), s.size());
	}
	template <size_t t_cch>
	int compare_to(C const (& ach)[t_cch]) const {
		assert(ach[t_cch - 1 /*NUL*/] == '\0');
		return TTraits::str_cmp(data(), size(), ach, t_cch - 1 /*NUL*/);
	}
	// This overload needs to be template, or it will take precedence over the one above.
	template <typename = void>
	int compare_to(C const * psz) const {
		return TTraits::str_cmp(data(), size(), psz, TTraits::str_len(psz));
	}


	/** Searches for the specified value; returns an iterator to the first matching item, or
	const_iterator() (evaluates to false) for no matches.

	TODO: comment signature.
	*/
	const_iterator find(char32_t chNeedle, const_iterator itFirst = const_iterator()) const {
		return const_iterator(TTraits::str_chr(
			(itFirst ? itFirst : itvec::cbegin()).base(), itvec::cend().base(),
			chNeedle
		));
	}
	const_iterator find(istr const & sNeedle, const_iterator itFirst = const_iterator()) const {
		return const_iterator(TTraits::str_str(
			(itFirst ? itFirst : itvec::cbegin()).base(), itvec::cend().base(),
			sNeedle.cbegin().base(), sNeedle.cend().base()
		));
	}


	/** Searches for the specified value, starting from the end or the provided iterator; returns an
	iterator to the first matching item, or const_iterator() (evaluates to false) for no matches.

	TODO: comment signature.
	*/
	const_iterator find_last(char32_t chNeedle, const_iterator itEnd = const_iterator()) const {
		return const_iterator(TTraits::str_chr_r(
			itvec::cbegin().base(), (itEnd ? itEnd : itvec::cend()).base(),
			chNeedle
		));
	}
	const_iterator find_last(istr const & sNeedle, const_iterator itEnd = const_iterator()) const {
		return const_iterator(TTraits::str_str_r(
			itvec::cbegin().base(), (itEnd ? itEnd : itvec::cend()).base(),
			sNeedle.cbegin().base(), sNeedle.cend().base()
		));
	}


	/** Uses the current contents of this string to generate a new one using str_ostream::print().

	TODO: comment signature.
	*/
	template <typename ... Ts>
	dmstr format(Ts const & ... ts) const;


	/** Returns the current size of the string buffer, in characters, minus room for the trailing NUL
	terminator.

	TODO: comment signature.
	*/
	size_t capacity() const {
		return _raw_str::capacity();
	}


	/** Returns a read-only pointer to the character array.

	TODO: comment signature.
	*/
	C const * data() const {
		return _raw_str::data<C>();
	}


	/** Work around the protected inheritance, forcing the raw access to be explicit.

	TODO: comment signature.
	*/
	_raw_str & get_raw() {
		return *this;
	}
	_raw_str const & get_raw() const {
		return *this;
	}


	/** Returns the count of characters in the string.

	TODO: comment signature.
	*/
	size_t size() const {
		return _raw_str::size();
	}


	/** Returns the count of code points in the string.

	TODO: comment signature.
	*/
	size_t size_cp() const {
		C const * pchBegin(data());
		return TTraits::str_cp_len(pchBegin, pchBegin + size());
	}


	/** Returns a portion of the string.

	ichFirst
		0-based index of the first character. If negative, it’s 1-based index from the end of the
		string.
	[cch]
		Count of characters to return. If negative, it’s the count of characters to skip, from the end
		of the string.
	*/
	dmstr substr(ptrdiff_t ichFirst) const {
		return substr(ichFirst, size());
	}
	dmstr substr(ptrdiff_t ichFirst, ptrdiff_t cch) const {
		adjust_range(&ichFirst, &cch);
		return dmstr(data() + ichFirst, size_t(cch));
	}
	dmstr substr(const_iterator itFirst) const {
		return substr(itFirst, itvec::cend());
	}
	dmstr substr(const_iterator itBegin, const_iterator itEnd) const {
		return dmstr(itBegin.base(), size_t(itEnd - itBegin));
	}


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	str_base_(size_t cchStatic) :
		_raw_str(cchStatic) {
	}
	str_base_(C const * pch, size_t cch) :
		_raw_str(pch, cch) {
	}


	/** See _raw_str::assign_copy().

	TODO: comment signature.
	*/
	void assign_copy(C const * pch, size_t cch) {
		_raw_str::assign_copy(sizeof(C), pch, cch);
	}
	void assign_copy(C const * pch1, size_t cch1, C const * pch2, size_t cch2) {
		_raw_str::assign_copy(sizeof(C), pch1, cch1, pch2, cch2);
	}


	/** See _raw_str::assign_move().

	TODO: comment signature.
	*/
	void assign_move(str_base_ && s) {
		_raw_str::assign_move(static_cast<_raw_str &&>(s));
	}


	/** See _raw_str::assign_move_dynamic_or_copy().

	TODO: comment signature.
	*/
	void assign_move_dynamic_or_copy(str_base_ && s) {
		_raw_str::assign_move_dynamic_or_copy(sizeof(C), static_cast<_raw_str &&>(s));
	}


	/** See _raw_str::assign_share_ro_or_copy().

	TODO: comment signature.
	*/
	void assign_share_ro_or_copy(str_base_ const & s) {
		_raw_str::assign_share_ro_or_copy(sizeof(C), s);
	}
};

} //namespace abc


// Relational operators.
#define ABC_RELOP_IMPL(op) \
	template <typename C, class TTraits> \
	inline bool operator op( \
		abc::str_base_<C, TTraits> const & s1, abc::str_base_<C, TTraits> const & s2 \
	) { \
		return s1.compare_to(static_cast<abc::istr_<C, TTraits>>(s2)) op 0; \
	} \
	template <typename C, class TTraits, size_t t_cch> \
	inline bool operator op(abc::str_base_<C, TTraits> const & s, C const (& ach)[t_cch]) { \
		return s.compare_to(ach) op 0; \
	} \
	template <typename C, class TTraits, size_t t_cch> \
	inline bool operator op(C const (& ach)[t_cch], abc::str_base_<C, TTraits> const & s) { \
		return -s.compare_to(ach) op 0; \
	} \
	template <typename C, class TTraits> \
	inline bool operator op(abc::str_base_<C, TTraits> const & s, C const * psz) { \
		return s.compare_to(psz) op 0; \
	} \
	template <typename C, class TTraits, size_t t_cch> \
	inline bool operator op(C const * psz, abc::str_base_<C, TTraits> const & s) { \
		return -s.compare_to(psz) op 0; \
	}
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL


namespace abc {

// Specialization of to_str_backend.
template <typename C, class TTraits>
class to_str_backend<str_base_<C, TTraits>> :
	public _str_to_str_backend<str_base_<C, TTraits>, C> {

	typedef _str_to_str_backend<str_base_<C, TTraits>, C> str_to_str_backend;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_str_backend(char_range const & crFormat = char_range()) :
		str_to_str_backend(crFormat) {
	}


	/** See to_str_backend::write().
	*/
	void write(str_base_<C, TTraits> const & s, ostream * posOut) {
		str_to_str_backend::write(s.data(), sizeof(C) * s.size(), TTraits::host_encoding, posOut);
	}
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <typename C, class TTraits>
struct hash<abc::str_base_<C, TTraits>> {

	size_t operator()(abc::str_base_<C, TTraits> const & s) const {
		return s.get_raw().hash(sizeof(C));
	}
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::istr_


namespace abc {

// Forward declaration.
template <typename C, class TTraits = text::utf_traits<C>>
class mstr_;

/** str_base_-derived class, to be used as “the” string class in most cases. It cannot be modified
in-place, which means that it shouldn’t be used in code performing intensive string manipulations.
*/
template <typename C, class TTraits /*= text::utf_traits<C>*/>
class istr_ :
	public str_base_<C, TTraits> {

	typedef str_base_<C, TTraits> str_base;
	typedef mstr_<C, TTraits> mstr;
	typedef dmstr_<C, TTraits> dmstr;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	istr_() :
		str_base(0) {
	}
	istr_(istr_ const & s) :
		str_base(0) {
		operator=(s);
	}
	istr_(istr_ && s) :
		str_base(0) {
		operator=(std::move(s));
	}
	istr_(str_base && s) :
		str_base(0) {
		operator=(std::move(s));
	}
	istr_(dmstr && s) :
		str_base(0) {
		operator=(std::move(s));
	}
	template <size_t t_cch>
	istr_(C const (& ach)[t_cch]) :
		str_base(ach, t_cch - 1 /*NUL*/) {
		assert(ach[t_cch - 1 /*NUL*/] == '\0');
	}
	istr_(C const * psz, size_t cch) :
		str_base(0) {
		assign_copy(psz, cch);
	}
	istr_(unsafe_t, C const * psz) :
		str_base(psz, TTraits::str_len(psz)) {
	}
	istr_(unsafe_t, C const * psz, size_t cch) :
		str_base(psz, cch) {
	}


	/** Assignment operator.

	TODO: comment signature.
	*/
	istr_ & operator=(istr_ const & s) {
		assign_share_ro_or_copy(s);
		return *this;
	}
	istr_ & operator=(istr_ && s) {
		// Non-const, so it can’t be anything but a real istr_, so it owns its item array.
		assign_move(std::move(s));
		return *this;
	}
	istr_ & operator=(str_base && s) {
		assign_move_dynamic_or_copy(std::move(s));
		return *this;
	}
	istr_ & operator=(dmstr && s) {
		assign_move(std::move(s));
		return *this;
	}
	template <size_t t_cch>
	istr_ & operator=(C const (& ach)[t_cch]) {
		// This order is safe, because the constructor invoked on the next line won’t throw.
		this->~istr_();
		::new(this) istr_(ach);
		return *this;
	}


	/** Automatic conversion to char_range.

	TODO: comment signature.
	*/
	operator char_range_<C>() const {
		return char_range_<C>(str_base::cbegin().base(), str_base::cend().base());
	}
};

typedef istr_<char_t> istr;
typedef istr_<char8_t> istr8;
typedef istr_<char16_t> istr16;
typedef istr_<char32_t> istr32;


// Specialization of to_str_backend.
template <typename C, class TTraits>
class to_str_backend<istr_<C, TTraits>> :
	public to_str_backend<str_base_<C, TTraits>> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_str_backend(char_range const & crFormat = char_range()) :
		to_str_backend<str_base_<C, TTraits>>(crFormat) {
	}
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <typename C, class TTraits>
struct hash<abc::istr_<C, TTraits>> :
	public hash<abc::str_base_<C, TTraits>> {
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::mstr_


namespace abc {

/** str_base_-derived class, to be used as argument type for functions that want to modify a string
argument, since unlike istr_, it allows in-place alterations to the string. Both smstr and dmstr_
are automatically converted to this.
*/
template <typename C, class TTraits /*= text::utf_traits<C>*/>
class mstr_ :
	public str_base_<C, TTraits> {

	typedef str_base_<C, TTraits> str_base;
	typedef istr_<C, TTraits> istr;
	typedef dmstr_<C, TTraits> dmstr;

public:

	/** Assignment operator.

	TODO: comment signature.
	*/
	mstr_ & operator=(mstr_ const & s) {
		return operator=(static_cast<str_base const &>(s));
	}
	// WARNING - this move-assignment operator CAN throw!
	mstr_ & operator=(mstr_ && s) {
		return operator=(static_cast<str_base &&>(s));
	}
	mstr_ & operator=(str_base const & s) {
		assign_copy(s.data(), s.size());
		return *this;
	}
	mstr_ & operator=(str_base && s) {
		assign_move_dynamic_or_copy(std::move(s));
		return *this;
	}
	mstr_ & operator=(dmstr && s) {
		assign_move(std::move(s));
		return *this;
	}
	template <size_t t_cch>
	mstr_ & operator=(C const (& ach)[t_cch]) {
		assert(ach[t_cch - 1 /*NUL*/] == '\0');
		assign_copy(ach, t_cch - 1 /*NUL*/);
		return *this;
	}


	/** Concatenation-assignment operator.

	TODO: comment signature.
	*/
	mstr_ & operator+=(C ch) {
		append(&ch, 1);
		return *this;
	}
	template <size_t t_cch>
	mstr_ & operator+=(C const (& ach)[t_cch]) {
		assert(ach[t_cch - 1 /*NUL*/] == '\0');
		append(ach, t_cch - 1 /*NUL*/);
		return *this;
	}
	mstr_ & operator+=(istr const & s) {
		append(s.data(), s.size());
		return *this;
	}


	/** Allows automatic cross-class-hierarchy casts.

	TODO: comment signature.
	*/
	operator istr const &() const {
		return *static_cast<istr const *>(static_cast<str_base const *>(this));
	}


	/** Item access operator.

	TODO: comment signature.
	*/
	C & operator[](size_t i) {
		if (i > str_base::size()) {
			abc_throw(index_error(intptr_t(i)));
		}
		return data()[i];
	}
	C operator[](size_t i) const {
		return str_base::operator[](intptr_t(i));
	}


	/** Same as operator+=(), but for multi-argument overloads.

	TODO: comment signature.
	*/
	void append(C const * pchAdd, size_t cchAdd) {
		_raw_str::append(sizeof(C), pchAdd, cchAdd, true);
	}


	/** Returns a pointer to the character array.

	TODO: comment signature.
	*/
	C * data() {
		return _raw_str::data<C>();
	}
	C const * data() const {
		return _raw_str::data<C>();
	}


	/** Grows the item array until the specified callback succeeds in filling it and returns a number
	of needed characters that’s less than the size of the buffer. For example, for cchMax == 3 (NUL
	terminator included), it must return <= 2 (NUL excluded).

	This method is not transaction-safe; if an exception is thrown in the callback or elsewhere,
	*this will not be restored to its previous state.

	TODO: maybe improve exception resilience? Check typical usage to see if it’s an issue.

	Note: this method probably benefits from being inlined in spite of its size, because in most
	cases the callback will be just a lambda wrapper around some API/OS function, so when the
	compiler inlines this method, it will most likely blend its code with the (also inlined) lambda,
	probably resulting in some optimizations that would be otherwise missed.

	TODO: comment signature.
	*/
	void grow_for(std::function<size_t (C * pch, size_t cchMax)> fnRead) {
		typedef _raw_vextr_impl_base rvib;
		// The initial size avoids a couple of reallocations.
		// Also, these numbers should guarantee that set_capacity() will allocate exactly the
		// requested number of characters, eliminating the need to query back with capacity().
		size_t cchRet(rvib::smc_cMinSlots * rvib::smc_iGrowthRate * rvib::smc_iGrowthRate);
		for (size_t cchMax(cchRet); cchRet >= cchMax; cchMax *= rvib::smc_iGrowthRate) {
			set_capacity(cchMax - 1 /*NUL*/, false);
			cchRet = fnRead(data(), cchMax);
		}
		// Finalize the length.
		set_size(cchRet);
	}


	/** See _raw_str::set_capacity().

	TODO: comment signature.
	*/
	void set_capacity(size_t cchMin, bool bPreserve) {
		_raw_str::set_capacity(sizeof(C), cchMin, bPreserve);
	}


	/** See _raw_str::set_size().

	TODO: comment signature.
	*/
	void set_size(size_t cch) {
		_raw_str::set_size(sizeof(C), cch);
	}


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	mstr_(size_t cchStatic) :
		str_base(cchStatic) {
	}
	mstr_(C const * pch, size_t cch) :
		str_base(pch, cch) {
	}
};

typedef mstr_<char_t> mstr;
typedef mstr_<char8_t> mstr8;
typedef mstr_<char16_t> mstr16;
typedef mstr_<char32_t> mstr32;


// Specialization of to_str_backend.
template <typename C, class TTraits>
class to_str_backend<mstr_<C, TTraits>> :
	public to_str_backend<str_base_<C, TTraits>> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_str_backend(char_range const & crFormat = char_range()) :
		to_str_backend<str_base_<C, TTraits>>(crFormat) {
	}
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <typename C, class TTraits>
struct hash<abc::mstr_<C, TTraits>> :
	public hash<abc::str_base_<C, TTraits>> {
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::dmstr_


namespace abc {

/** mstr_-derived class, good for clients that need in-place manipulation of strings whose length
is unknown at design time.
*/
template <typename C, class TTraits /*= text::utf_traits<C>*/>
class dmstr_ :
	public mstr_<C, TTraits> {

	typedef str_base_<C, TTraits> str_base;
	typedef mstr_<C, TTraits> mstr;
	typedef istr_<C, TTraits> istr;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	dmstr_() :
		mstr(0) {
	}
	dmstr_(dmstr_ const & s) :
		mstr(0) {
		operator=(s);
	}
	dmstr_(dmstr_ && s) :
		mstr(0) {
		operator=(std::move(s));
	}
	dmstr_(str_base const & s) :
		mstr(0) {
		operator=(s);
	}
	dmstr_(str_base && s) :
		mstr(0) {
		operator=(std::move(s));
	}
	template <size_t t_cch>
	dmstr_(C const (& ach)[t_cch]) :
		mstr(0) {
		operator=(ach);
	}
	dmstr_(C const * pch, size_t cch) :
		mstr(0) {
		assign_copy(pch, cch);
	}
	dmstr_(C const * pch1, size_t cch1, C const * pch2, size_t cch2) :
		mstr(0) {
		assign_copy(pch1, cch1, pch2, cch2);
	}


	/** Assignment operator.

	TODO: comment signature.
	*/
	dmstr_ & operator=(dmstr_ const & s) {
		mstr::operator=(s);
		return *this;
	}
	dmstr_ & operator=(dmstr_ && s) {
		mstr::operator=(std::move(s));
		return *this;
	}
	dmstr_ & operator=(str_base const & s) {
		mstr::operator=(s);
		return *this;
	}
	dmstr_ & operator=(str_base && s) {
		mstr::operator=(std::move(s));
		return *this;
	}
	template <size_t t_cch>
	dmstr_ & operator=(C const (& ach)[t_cch]) {
		mstr::operator=(ach);
		return *this;
	}
};

} //namespace abc


/** Concatenation operator.

TODO: comment signature.
*/
template <typename C, class TTraits>
inline abc::dmstr_<C, TTraits> operator+(
	abc::str_base_<C, TTraits> const & s1, abc::str_base_<C, TTraits> const & s2
) {
	return abc::dmstr_<C, TTraits>(s1.data(), s1.size(), s2.data(), s2.size());
}
// Overloads taking a character literal.
template <typename C, class TTraits>
inline abc::dmstr_<C, TTraits> operator+(abc::str_base_<C, TTraits> const & s, C ch) {
	return abc::dmstr_<C, TTraits>(s.data(), s.size(), &ch, 1);
}
template <typename C, class TTraits>
inline abc::dmstr_<C, TTraits> operator+(C ch, abc::str_base_<C, TTraits> const & s) {
	return abc::dmstr_<C, TTraits>(&ch, 1, s.data(), s.size());
}
// Overloads taking a string literal.
template <typename C, class TTraits, size_t t_cch>
inline abc::dmstr_<C, TTraits> operator+(
	abc::str_base_<C, TTraits> const & s, C const (& ach)[t_cch]
) {
	assert(ach[t_cch - 1 /*NUL*/] == '\0');
	return abc::dmstr_<C, TTraits>(s.data(), s.size(), ach, t_cch - 1 /*NUL*/);
}
template <typename C, class TTraits, size_t t_cch>
inline abc::dmstr_<C, TTraits> operator+(
	C const (& ach)[t_cch], abc::str_base_<C, TTraits> const & s
) {
	assert(ach[t_cch - 1 /*NUL*/] == '\0');
	return abc::dmstr_<C, TTraits>(ach, t_cch - 1 /*NUL*/, s.data(), s.size());
}
// Overloads taking a temporary dmstr as left operand; they can avoid creating an intermediate
// string.
template <typename C, class TTraits>
inline abc::dmstr_<C, TTraits> operator+(abc::dmstr_<C, TTraits> && s, C ch) {
	s += ch;
	return std::move(s);
}
template <typename C, class TTraits, size_t t_cch>
inline abc::dmstr_<C, TTraits> operator+(abc::dmstr_<C, TTraits> && s, C const (& ach)[t_cch]) {
	s += ach;
	return std::move(s);
}


namespace abc {

// Specialization of to_str_backend.
template <typename C, class TTraits>
class to_str_backend<dmstr_<C, TTraits>> :
	public to_str_backend<str_base_<C, TTraits>> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_str_backend(char_range const & crFormat = char_range()) :
		to_str_backend<str_base_<C, TTraits>>(crFormat) {
	}
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <typename C, class TTraits>
struct hash<abc::dmstr_<C, TTraits>> :
	public hash<abc::str_base_<C, TTraits>> {
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::smstr


namespace abc {

/** mstr_-derived class, good for clients that need in-place manipulation of strings that are most
likely to be shorter than a known small size.
*/
template <size_t t_cchStatic, typename C = char_t, class TTraits = text::utf_traits<C>>
class smstr :
	public mstr_<C, TTraits> {

	typedef str_base_<C, TTraits> str_base;
	typedef mstr_<C, TTraits> mstr;
	typedef istr_<C, TTraits> istr;
	typedef dmstr_<C, TTraits> dmstr;

private:

	/** Actual static item array size. */
	static size_t const smc_cchFixed = _ABC__RAW_VEXTR_IMPL_BASE__ADJUST_ITEM_COUNT(
		t_cchStatic + 1 /*NUL*/
	);


public:

	/** Constructor.

	TODO: comment signature.
	*/
	smstr() :
		mstr(smc_cchFixed) {
	}
	smstr(smstr const & s) :
		mstr(smc_cchFixed) {
		operator=(s);
	}
	// This won’t throw exceptions - see operator=(smstr &&).
	smstr(smstr && s) :
		mstr(smc_cchFixed) {
		operator=(std::move(s));
	}
	smstr(str_base const & s) :
		mstr(smc_cchFixed) {
		operator=(s);
	}
	smstr(str_base && s) :
		mstr(smc_cchFixed) {
		operator=(std::move(s));
	}
	smstr(dmstr && s) :
		mstr(smc_cchFixed) {
		operator=(std::move(s));
	}
	template <size_t t_cch>
	smstr(C const (& ach)[t_cch]) :
		mstr(smc_cchFixed) {
		operator=(ach);
	}


	/** Assignment operator.

	TODO: comment signature.
	*/
	smstr & operator=(smstr const & s) {
		mstr::operator=(s);
		return *this;
	}
	// If the source is using its static item array, it will be copied without allocating a dynamic
	// one; if the source is dynamic, it will be moved. Either way, this won’t throw.
	smstr & operator=(smstr && s) {
		mstr::operator=(std::move(s));
		return *this;
	}
	// This also covers smstr of different template arguments.
	smstr & operator=(str_base const & s) {
		mstr::operator=(s);
		return *this;
	}
	// This also covers smstr of different template arguments.
	smstr & operator=(str_base && s) {
		mstr::operator=(std::move(s));
		return *this;
	}
	smstr & operator=(dmstr && s) {
		mstr::operator=(std::move(s));
		return *this;
	}
	template <size_t t_cch>
	smstr & operator=(C const (& ach)[t_cch]) {
		mstr::operator=(ach);
		return *this;
	}


private:

	// This section must match exactly _raw_vextr_impl_base_with_static_item_array.

	/** See _raw_vextr_impl_base_with_static_item_array::m_ciStaticMax. */
	size_t m_ciStaticMax;
	/** See _raw_vextr_impl_base_with_static_item_array::m_at. */
	std::max_align_t m_at[ABC_ALIGNED_SIZE(sizeof(C) * smc_cchFixed)];
};

typedef dmstr_<char_t> dmstr;
typedef dmstr_<char8_t> dmstr8;
typedef dmstr_<char16_t> dmstr16;
typedef dmstr_<char32_t> dmstr32;


// Specialization of to_str_backend.
template <size_t t_cchStatic, typename C, class TTraits>
class to_str_backend<smstr<t_cchStatic, C, TTraits>> :
	public to_str_backend<str_base_<C, TTraits>> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_str_backend(char_range const & crFormat = char_range()) :
		to_str_backend<str_base_<C, TTraits>>(crFormat) {
	}
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <size_t t_cchStatic, typename C, class TTraits>
struct hash<abc::smstr<t_cchStatic, C, TTraits>> :
	public hash<abc::str_base_<C, TTraits>> {
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_STRING_HXX

