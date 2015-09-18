
// Nes_Emu 0.5.6. http://www.slack.net/~ant/

#include "Nes_Rom.h"

#include <stdlib.h>
#include <string.h>

/* Copyright (C) 2004-2005 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details. You should have received a copy of the GNU Lesser General
Public License along with this module; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */

#include BLARGG_SOURCE_BEGIN

Nes_Rom::Nes_Rom()
{
	prg_ = NULL;
	chr_ = NULL;
	reset();
}

Nes_Rom::~Nes_Rom()
{
	reset();
}

void Nes_Rom::reset()
{
	free( prg_ );
	prg_ = NULL;
	
	free( chr_ );
	chr_ = NULL;
	
	prg_size_ = 0;
	chr_size_ = 0;
	mapper = 0;
}

long Nes_Rom::round_to_bank_size( long n )
{
	n += bank_size - 1;
	return n - n % bank_size;
}

blargg_err_t Nes_Rom::resize_prg( long size )
{
	if ( size != prg_size_ )
	{
		// extra byte allows CPU to always read operand of instruction, which
		// might go past end of ROM
		void* p = realloc( prg_, round_to_bank_size( size ) + 1 );
		BLARGG_CHECK_ALLOC( p || !size );
		prg_ = (uint8_t*) p;
		prg_size_ = size;
	}
   return 0;
}

blargg_err_t Nes_Rom::resize_chr( long size )
{
	if ( size != chr_size_ )
	{
		void* p = realloc( chr_, round_to_bank_size( size ) );
		BLARGG_CHECK_ALLOC( p || !size );
		chr_ = (uint8_t*) p;
		chr_size_ = size;
	}
   return 0;
}

// iNES reading

struct ines_header_t {
	uint8_t signature [4];
	uint8_t prg_count; // number of 16K PRG banks
	uint8_t chr_count; // number of 8K CHR banks
	uint8_t flags;     // MMMM FTBV Mapper low, Four-screen, Trainer, Battery, V mirror
	uint8_t flags2;    // MMMM --XX Mapper high 4 bits
	uint8_t zero [8];  // if zero [7] is non-zero, treat flags2 as zero
};
BOOST_STATIC_ASSERT( sizeof (ines_header_t) == 16 );

blargg_err_t Nes_Rom::load_ines_rom( Data_Reader& in )
{
	ines_header_t h;
	BLARGG_RETURN_ERR( in.read( &h, sizeof h ) );
	
	if ( 0 != memcmp( h.signature, "NES\x1A", 4 ) )
		return "Not a iNES ROM file";
	
	if ( h.zero [7] ) // handle header defaced by a fucking idiot's handle
		h.flags2 = 0;
	
	set_mapper( h.flags, h.flags2 );
	
	if ( h.flags & 0x04 ) // skip trainer
		BLARGG_RETURN_ERR( in.skip( 512 ) );
	
	BLARGG_RETURN_ERR( resize_prg( h.prg_count * 16 * 1024L ) );
	BLARGG_RETURN_ERR( resize_chr( h.chr_count * 8 * 1024L ) );
	
	BLARGG_RETURN_ERR( in.read( prg(), prg_size() ) );
	BLARGG_RETURN_ERR( in.read( chr(), chr_size() ) );
	
   return 0;
}
