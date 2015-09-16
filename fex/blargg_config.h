// Library configuration. Modify this file as necessary.

// File_Extractor 1.0.0
#ifndef BLARGG_CONFIG_H
#define BLARGG_CONFIG_H

// Accept file paths encoded as UTF-8. Currently only affects Windows,
// as Unix/Linux/Mac OS X already use UTF-8 paths.
#define BLARGG_UTF8_PATHS 1

// Enable support for as building DLL on Windows.
//#define BLARGG_BUILD_DLL 1

// Support only the listed archive types. Remove any you don't need.
/*
#define FEX_TYPE_LIST \
	fex_7z_type,\
	fex_gz_type,\
	fex_rar_type,\
	fex_zip_type,
*/

#define HAVE_STDINT_H

// Use standard config.h if present
#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#endif
