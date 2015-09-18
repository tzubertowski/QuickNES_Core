// Sets up common environment for Shay Green's libraries.
// To change configuration options, modify blargg_config.h, not this file.

// File_Extractor 1.0.0
#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

/* BLARGG_STATIC_ASSERT( expr ): Generates compile error if expr is 0.
Can be used at file, function, or class scope. */
#ifdef _MSC_VER
	// MSVC6 (_MSC_VER < 1300) __LINE__ fails when /Zl is specified
	#define BLARGG_STATIC_ASSERT( expr )    \
		void blargg_failed_( int (*arg) [2 / (int) !!(expr) - 1] )
#else
	// Others fail when declaring same function multiple times in class,
	// so differentiate them by line
	#define BLARGG_STATIC_ASSERT( expr )    \
		void blargg_failed_( int (*arg) [2 / !!(expr) - 1] [__LINE__] )
#endif

/* Pure virtual functions cause a vtable entry to a "called pure virtual"
error handler, requiring linkage to the C++ runtime library. This macro is
used in place of the "= 0", and simply expands to its argument. During
development, it expands to "= 0", allowing detection of missing overrides. */
#define BLARGG_PURE( def ) def

/* My code depends on ASCII anywhere a character or string constant is
compared with data read from a file, and anywhere file data is read and
treated as a string. */
#if '\n'!=0x0A || ' '!=0x20 || '0'!=0x30 || 'A'!=0x41 || 'a'!=0x61
	#error "ASCII character set required"
#endif

/* My code depends on int being at least 32 bits. Almost everything these days
uses at least 32-bit ints, so it's hard to even find a system with 16-bit ints
to test with. The issue can't be gotten around by using a suitable blargg_int
everywhere either, because int is often converted to implicitly when doing
arithmetic on smaller types. */
#if UINT_MAX < 0xFFFFFFFF
	#error "int must be at least 32 bits"
#endif

// In case compiler doesn't support these properly. Used rarely.
#define STATIC_CAST(T,expr) static_cast<T> (expr)
#define CONST_CAST( T,expr) const_cast<T> (expr)

// User configuration can override the above macros if necessary
#include "blargg_config.h"

/* BLARGG_DEPRECATED [_TEXT] for any declarations/text to be removed in a
future version. In GCC, we can let the compiler warn. In other compilers,
we strip it out unless BLARGG_LEGACY is true. */
#if BLARGG_LEGACY
	// Allow old client code to work without warnings
	#define BLARGG_DEPRECATED_TEXT( text ) text
	#define BLARGG_DEPRECATED(      text ) text
#elif __GNUC__ >= 4
	// In GCC, we can mark declarations and let the compiler warn
	#define BLARGG_DEPRECATED_TEXT( text ) text
	#define BLARGG_DEPRECATED(      text ) __attribute__ ((deprecated)) text
#else
	// By default, deprecated items are removed, to avoid use in new code
	#define BLARGG_DEPRECATED_TEXT( text )
	#define BLARGG_DEPRECATED(      text )
#endif

// BLARGG_NEW is used in place of new in library code
#include <new>
#ifndef BLARGG_NEW
#define BLARGG_NEW new (std::nothrow)
#endif

// Callback function with user data.
// blargg_callback<T> set_callback; // for user, this acts like...
// void set_callback( T func, void* user_data = NULL ); // ...this
// To call function, do set_callback.f( .. set_callback.data ... );
template<class T>
struct blargg_callback
{
	T f;
	void* data;
	blargg_callback() { f = NULL; }
	void operator () ( T callback, void* user_data = NULL ) { f = callback; data = user_data; }
};

BLARGG_DEPRECATED( typedef signed   int blargg_long; )
BLARGG_DEPRECATED( typedef unsigned int blargg_ulong; )
#if BLARGG_LEGACY
	#define BOOST_STATIC_ASSERT BLARGG_STATIC_ASSERT
#endif

#endif
