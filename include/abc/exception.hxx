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

#ifndef ABC_EXCEPTION_HXX
#define ABC_EXCEPTION_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/core.hxx>
#include <abc/char.hxx>
#include <exception>



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

// Forward declaration from iostream.hxx.
class ostream;
// Methods here need to use ostream * instead of ostream &, because at this point ostream has only
// been forward-declared above, but not defined yet (a pointer to a forward-declared type is legal,
// but a reference to it is not).


/// DESIGN_8191 Exceptions
//
// Combined with [DESIGN_8503 Stack tracing], the use of abc_throw() augments the stack trace with
// the exact line where the throw statement occurred.
// Only instances of abc::exception (or a derived class) can be thrown using abc_throw(), because of
// the additional members that the latter expects to be able to set in the former.
//
// The class abc::exception also implements the actual stack trace printing for abc::_stack_trace,
// since this file is included in virtually every file whereas trace.hxx is not.

/// Throws the specified object, after providing it with debugging information.
//
#define abc_throw(x) \
	throw(abc::exception::_before_throw((x), __FILE__, __LINE__, _ABC_ASSERT_FN))


/// Verifies an expression at compile time. Failure is reported as a compiler error.
//
#if !defined(_GCC_VER) && !defined(_MSC_VER)
	#define static_assert(expr, msg) \
		extern char _static_assert_failed[(expr) ? 1 : -1]
#endif


/// Verifies a condition at runtime, throwing a assertion_error exception if the assertion turns out
// to be incorrect.
//
#undef assert
#ifdef DEBUG
	#define assert(expr) \
		do { \
			if (!(expr)) { \
				abc::assertion_error::_assertion_failed(__FILE__, __LINE__, _ABC_ASSERT_FN, #expr); \
			} \
		} while (0)
#else
	#define assert(expr) \
		static_cast<void>(0)
#endif


/// Integer type used by the OS to represent error numbers.
#if ABC_HOST_API_POSIX
	typedef int errint_t;
#elif ABC_HOST_API_WIN32
	typedef DWORD errint_t;
#else
	#error TODO-PORT: HOST_API
#endif


#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32

/// Throws an exception matching a specified OS-defined error, or the last reported by the OS.
ABC_FUNC_NORETURN void throw_os_error();
ABC_FUNC_NORETURN void throw_os_error(errint_t err);

#endif


/// Pretty-printed name of the current function.
#if defined(_GCC_VER)
	#define _ABC_ASSERT_FN \
		__PRETTY_FUNCTION__
#elif defined(_MSC_VER)
	#define _ABC_ASSERT_FN \
		__FUNCTION__
#else
	#define _ABC_ASSERT_FN \
		NULL
#endif


/// Base for all exceptions classes.
class exception;

/// An assertion failed.
class assertion_error;

/// Base for all error-related exceptions classes.
class generic_error;

/// Defines a member mapped_error to the default OS-specific error code associated to an exception
// class.
template <class TError>
struct os_error_mapping;

/// The user hit an interrupt key (usually Ctrl-C or Del).
class user_interrupt;


// Other exception classes

/// A function/method received an argument that had an inappropriate value.
class argument_error;

/// Base for arithmetic errors.
class arithmetic_error;

/// An attribute reference or assignment failed.
class attribute_error;

/// A buffer-related I/O operation could not be performed.
class buffer_error;

/// The divisor of a division or modulo operation was zero.
class division_by_zero_error;

/// Base for errors that occur in the outer system.
class environment_error;

/// A file could not be found.
class file_not_found_error;

/// A floating point operation failed.
class floating_point_error;

/// Sequence subscript out of range.
class index_error;

/// The specified file path is not a valid path.
class invalid_path_error;

/// An I/O operation failed for an I/O-related reason.
class io_error;

/// Mapping (dictionary) key not found in the set of existing keys.
class key_error;

/// Base for errors due to an invalid key or index being used on a mapping or sequence.
class lookup_error;

/// An invalid memory access (e.g. misaligned pointer) was detected.
class memory_access_error;

/// An attempt was made to access an invalid memory location.
class memory_address_error;

/// A memory allocation request could not be satisfied.
class memory_allocation_error;

/// An attempt was made to access the memory location 0 (NULL).
class null_pointer_error;

/// A network-related error occurred.
class network_error;

/// An I/O operation failed for a network-related reason.
class network_io_error;

/// Method not implemented for this class. Usually thrown when a class is not able to provide a full
// implementation of an interface; in practice, this should be avoided.
class not_implemented_error;

/// Result of an arithmetic operation too large to be represented. Because of the lack of
// standardization of floating point exception handling in C, most floating point operations also
// aren’t checked.
class overflow_error;

/// An operation failed to prevent a security hazard.
class security_error;

/// The syntax for the specified expression is invalid.
class syntax_error;

/// A text encoding or decoding error occurred.
class text_error;

/// A text decoding error occurred.
class text_decode_error;

/// A text encoding error occurred.
class text_encode_error;

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception


namespace abc {

class exception :
	public std::exception {
public:

	/// Constructor.
	exception();
	exception(exception const & x);

	/// Destructor.
	virtual ~exception() decl_throw(());

	/// Assignment operator.
	exception & operator=(exception const & x);


	/// Stores context information to be displayed if the exception is not caught.
	//
	void _before_throw(char const * pszFileName, uint16_t iLine, char const * pszFunction);
	// Overload that operates on a temporary object that it also returns.
	template <class T>
	static T && _before_throw(
		T && t, char const * pszFileName, uint16_t iLine, char const * pszFunction
	) {
		t._before_throw(pszFunction, iLine, pszFileName);
		return static_cast<T &&>(t);
	}


	/// Shows a stack trace after an exception has unwound the stack up to the main() level.
	static void _uncaught_exception_end(std::exception const * pstdx = NULL);

	/// See std::exception::what().
	virtual char const * what() const decl_throw(());


protected:

	/// Prints extended information for the exception.
	virtual void _print_extended_info(ostream * pos) const;


public:

	/// Establishes, and restores upon destruction, special-case handlers to convert non-C++
	// asynchronous error events (POSIX signals, Win32 Structured Exceptions) into C++ exceptions.
	//
	// Note: this class uses global or thread-local variables (OS-dependent) for all its member
	// variables, since their types cannot be specified without #including a lot of files into this
	// one.
	//
	class async_handler_manager {
	public:

#if ABC_HOST_API_LINUX || ABC_HOST_API_WIN32
		// Constructor.
		async_handler_manager();

		// Destructor.
		~async_handler_manager();
#else
		// For unsupported OSes, these are just no-ops. A little silly, but avoids conditional code in
		// other files that shouldn’t care whether the target OS is supported in this respect or not.

		/// Constructor.
		async_handler_manager() {
		}


		/// Destructor.
		~async_handler_manager() {
		}
#endif
	};


protected:

	/// String to be returned by what(). Derived classes can overwrite this instead of overriding the
	// entire std::exception::what() method.
	char const * m_pszWhat;


private:

	/// Source function name.
	char const * m_pszSourceFunction;
	/// Name of the source file.
	char const * m_pszSourceFileName;
	/// Number of the source line.
	uint16_t m_iSourceLine;
	/// true if *this is an in-flight exception (it has been thrown) or is a copy of one.
	bool m_bInFlight;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::assertion_error


namespace abc {

class assertion_error :
	public exception {
public:

	static ABC_FUNC_NORETURN void _assertion_failed(
		char const * pszFileName, unsigned iLine, char const * pszFunction, char const * pszExpr
	);


protected:

	/// Set to true for the duration of the execution of _assertion_failed(). If another assertion
	// fails due to code executed during the call to _assertion_failed(), the latter will just throw,
	// without printing anything; otherwise we’ll most likely get stuck in an infinite recursion.
	static /*tls*/ bool sm_bReentering;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::user_interrupt


namespace abc {

class user_interrupt :
	public exception {
public:
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::generic_error


namespace abc {

class generic_error :
	public exception {
public:

	/// Constructor.
	generic_error(errint_t err = 0);
	generic_error(generic_error const & x);


	/// Assignment operator.
	generic_error & operator=(generic_error const & x);


	/// Returns the OS-specific error number, if any.
	//
	errint_t get_os_error() const {
		return m_err;
	}


protected:

	/// OS-specific error wrapped by this exception.
	errint_t m_err;
};

} //namespace abc


// Relational operators.
#define ABC_RELOP_IMPL(op) \
	inline bool operator op(abc::generic_error const & ge1, abc::generic_error const & ge2) { \
		return ge1.get_os_error() op ge2.get_os_error(); \
	} \
	inline bool operator op(abc::generic_error const & ge, abc::errint_t err) { \
		return ge.get_os_error() op err; \
	} \
	inline bool operator op(abc::errint_t err, abc::generic_error const & ge) { \
		return err op ge.get_os_error(); \
	}
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
#undef ABC_RELOP_IMPL


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::os_error_mapping


namespace abc {

template <class TError>
struct os_error_mapping {

	static errint_t const mapped_error = 0;
};


/// Defines an OS-specific error code to be the default for an exception class.
#define ABC_MAP_ERROR_CLASS_TO_ERRINT(errclass, err) \
	template <> \
	class os_error_mapping<errclass> { \
	public: \
	\
		/** Default error code the class errclass maps from. */ \
		static errint_t const mapped_error = err; \
	}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::argument_error


namespace abc {

class argument_error :
	public virtual generic_error {
public:

	/// Constructor.
	// TODO: add arguments name/value, to be passed by macro abc_throw_argument_error(argname).
	argument_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::arithmetic_error


namespace abc {

class arithmetic_error :
	public virtual generic_error {
public:

	/// Constructor.
	arithmetic_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::division_by_zero_error


namespace abc {

class division_by_zero_error :
	public virtual arithmetic_error {
public:

	/// Constructor.
	division_by_zero_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::domain_error


namespace abc {

class domain_error :
	public virtual generic_error {
public:

	/// Constructor.
	domain_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::floating_point_error


namespace abc {

class floating_point_error :
	public virtual arithmetic_error {
public:

	/// Constructor.
	floating_point_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::lookup_error


namespace abc {

class lookup_error :
	public virtual generic_error {
public:

	/// Constructor.
	lookup_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::index_error


namespace abc {

class index_error :
	public virtual lookup_error {
public:

	/// Constructor.
	index_error(intptr_t iInvalid, errint_t err = 0);
	index_error(index_error const & x);

	/// Assignment operator.
	index_error & operator=(index_error const & x);


	/// Returns the invalid index.
	//
	intptr_t get_index() const {
		return m_iInvalid;
	}


protected:

	/// See exception::_print_extended_info().
	virtual void _print_extended_info(ostream * pos) const;


private:

	/// Index that caused the error.
	intptr_t m_iInvalid;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_address_error


namespace abc {

class memory_address_error :
	public virtual generic_error {
public:

	/// Constructor.
	memory_address_error(errint_t err = 0);
	memory_address_error(void const * pInvalid, errint_t err = 0);
	memory_address_error(memory_address_error const & x);

	/// Assignment operator.
	memory_address_error & operator=(memory_address_error const & x);


	/// Returns the faulty address.
	//
	void const * get_address() const {
		return m_pInvalid;
	}


protected:

	/// See exception::_print_extended_info().
	virtual void _print_extended_info(ostream * pos) const;


private:

	/// Address that could not be accessed.
	void const * m_pInvalid;
	/// String used as special value for when the address is not available.
	static char_t const smc_achUnknownAddress[];
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_access_error


namespace abc {

class memory_access_error :
	public virtual memory_address_error {
public:

	/// Constructor.
	memory_access_error(void const * pInvalid, errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_allocation_error


namespace abc {

class memory_allocation_error :
	public virtual generic_error,
	public virtual std::bad_alloc {
public:

	/// Constructor.
	memory_allocation_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::null_pointer_error


namespace abc {

class null_pointer_error :
	public virtual memory_address_error {
public:

	/// Constructor.
	null_pointer_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::overflow_error


namespace abc {

class overflow_error :
	public virtual arithmetic_error {
public:

	/// Constructor.
	overflow_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::syntax_error


namespace abc {

class syntax_error :
	public virtual generic_error {
public:

	/// Constructor.
	//
	// All arguments are optional, and can be specified leaving defaulted gaps in between; the
	// resulting exception message will not contain omitted arguments.
	//
	// The order of line and character is inverted, so that this single overload can be used to
	// differenciate between cases in which pszSource is the single line containing the failing
	// expression (the thrower would not pass iLine) and cases where pszSource is the source file
	// containing the error (the thrower would pass the non-zero line number).
	//
	// Examples:
	//
	//    syntax_error(SL("expression cannot be empty"))
	//    syntax_error(SL("unmatched '{'"), sExpr, iChar)
	//    syntax_error(SL("expected expression"), char_range(), iChar, iLine)
	//    syntax_error(SL("unexpected end of file"), fpSource.get_path(), iChar, iLine)
	//
	syntax_error(
		char_range const & crDescription = char_range(), 
		char_range const & crSource = char_range(), unsigned iChar = 0, unsigned iLine = 0,
		errint_t err = 0
	);
	syntax_error(syntax_error const & x);

	/// Assignment operator.
	syntax_error & operator=(syntax_error const & x);


protected:

	/// See exception::_print_extended_info().
	virtual void _print_extended_info(ostream * pos) const;


private:

	/// Description of the syntax error.
	char_range m_crDescription;
	/// Source of the syntax error (whole or individual line).
	char_range m_crSource;
	/// Character at which the error is located.
	unsigned m_iChar;
	/// Line where the error is located.
	unsigned m_iLine;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc other exception classes


namespace abc {

#define ABC_DERIVE_ERROR_CLASS(derived, base) \
	class derived : \
		public virtual base { \
	public: \
	\
		/** Constructor. */ \
		derived(errint_t err = 0) : \
			generic_error(err) { \
		} \
	}


#define ABC_DERIVE_ERROR_CLASS2(derived, base1, base2) \
	class derived : \
		public virtual base1, \
		public virtual base2 { \
	public: \
	\
		/** Constructor. */ \
		derived(errint_t err = 0) : \
			generic_error(err) { \
		} \
	}

ABC_DERIVE_ERROR_CLASS(attribute_error, generic_error);
ABC_DERIVE_ERROR_CLASS(environment_error, generic_error);
ABC_DERIVE_ERROR_CLASS(file_not_found_error, environment_error);
ABC_DERIVE_ERROR_CLASS(io_error, environment_error);
ABC_DERIVE_ERROR_CLASS(buffer_error, io_error);
ABC_DERIVE_ERROR_CLASS(invalid_path_error, generic_error);
ABC_DERIVE_ERROR_CLASS(key_error, lookup_error);
ABC_DERIVE_ERROR_CLASS(network_error, environment_error);
ABC_DERIVE_ERROR_CLASS2(network_io_error, io_error, network_error);
ABC_DERIVE_ERROR_CLASS(not_implemented_error, generic_error);
ABC_DERIVE_ERROR_CLASS(security_error, environment_error);
ABC_DERIVE_ERROR_CLASS(text_error, generic_error);
ABC_DERIVE_ERROR_CLASS(text_decode_error, text_error);
ABC_DERIVE_ERROR_CLASS(text_encode_error, text_error);

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_EXCEPTION_HXX

