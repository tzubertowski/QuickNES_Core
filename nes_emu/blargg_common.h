
// Sets up common environment for Shay Green's libraries.
//
// To change configuration options, modify blargg_config.h, not this file.

#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

#include <stddef.h>
#include <stdlib.h>
#include <limits.h>

// User configuration (allow it to #include "blargg_common.h" normally)
#undef BLARGG_COMMON_H
#include "blargg_config.h"
#define BLARGG_COMMON_H

/* BLARGG_COMPILER_HAS_BOOL: If 0, provides bool support for old compiler. If 1,
   compiler is assumed to support bool. If undefined, availability is determined.
   If errors occur here, add the following line to your config.h file:
	#define BLARGG_COMPILER_HAS_BOOL 1
*/
#ifndef BLARGG_COMPILER_HAS_BOOL
	#if defined (__MWERKS__)
		#if !__option(bool)
			#define BLARGG_COMPILER_HAS_BOOL 0
		#endif
	#elif defined (_MSC_VER)
		#if _MSC_VER < 1100
			#define BLARGG_COMPILER_HAS_BOOL 0
		#endif
	#elif defined (__GNUC__)
		// supports bool
	#elif __cplusplus < 199711
		#define BLARGG_COMPILER_HAS_BOOL 0
	#endif
#endif
#if defined (BLARGG_COMPILER_HAS_BOOL) && !BLARGG_COMPILER_HAS_BOOL
	typedef int bool;
	const bool true  = 1;
	const bool false = 0;
#endif

#include <new>
// BLARGG_NEW is used in place of 'new' to create objects. By default, plain new is used.
// To prevent an exception if out of memory, #define BLARGG_NEW new (std::nothrow)
#ifndef BLARGG_NEW
#define BLARGG_NEW new (std::nothrow)
#endif

// BOOST_STATIC_ASSERT( expr ): Generates compile error if expr is 0.
#ifndef BOOST_STATIC_ASSERT
	#ifdef _MSC_VER
		// MSVC6 (_MSC_VER < 1300) fails for use of __LINE__ when /Zl is specified
		#define BOOST_STATIC_ASSERT( expr ) \
			void blargg_failed_( int (*arg) [2 / !!(expr) - 1] )
	#else
		// Some other compilers fail when declaring same function multiple times in class,
		// so differentiate them by line
		#define BOOST_STATIC_ASSERT( expr ) \
			void blargg_failed_( int (*arg) [2 / !!(expr) - 1] [__LINE__] )
	#endif
#endif

// In case compiler doesn't support these properly. Used rarely.
#define STATIC_CAST(T,expr) static_cast<T> (expr)
#define CONST_CAST( T,expr) const_cast<T> (expr)

// blargg_err_t (0 on success, otherwise error string)
#ifndef blargg_err_t
	typedef const char* blargg_err_t;
#endif

// Success; no error
blargg_err_t const blargg_ok = 0;

/* Pure virtual functions cause a vtable entry to a "called pure virtual"
error handler, requiring linkage to the C++ runtime library. This macro is
used in place of the "= 0", and simply expands to its argument. During
development, it expands to "= 0", allowing detection of missing overrides. */
#define BLARGG_PURE( def ) def

/* My code is not written with exceptions in mind, so either uses new (nothrow)
OR overrides operator new in my classes. The former is best since clients
creating objects will get standard exceptions on failure, but that causes it
to require the standard C++ library. So, when the client is using the C
interface, I override operator new to use malloc. */

#endif

