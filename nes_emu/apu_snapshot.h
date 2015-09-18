
// NES APU snapshot support

// Nes_Snd_Emu 0.1.7. Copyright (C) 2003-2005 Shay Green. GNU LGPL license.

#ifndef APU_SNAPSHOT_H
#define APU_SNAPSHOT_H

#include "blargg_common.h"

struct apu_snapshot_t
{
	typedef uint8_t env_t [3];
	
	uint8_t w40xx [0x14]; // $4000-$4013
	uint8_t w4015; // enables
	uint8_t w4017; // mode
	uint16_t delay;
	uint8_t step;
	uint8_t irq_flag;
	
	struct square_t {
		uint16_t delay;
		env_t env;
		uint8_t length;
		uint8_t phase;
		uint8_t swp_delay;
		uint8_t swp_reset;
		uint8_t unused [1];
	};
	
	square_t square1;
	square_t square2;
	
	struct triangle_t {
		uint16_t delay;
		uint8_t length;
		uint8_t phase;
		uint8_t linear_counter;
		uint8_t linear_mode;
	} triangle;
	
	struct noise_t {
		uint16_t delay;
		env_t env;
		uint8_t length;
		uint16_t shift_reg;
	} noise;
	
	struct dmc_t {
		uint16_t delay;
		uint16_t remain;
		uint16_t addr;
		uint8_t buf;
		uint8_t bits_remain;
		uint8_t bits;
		uint8_t buf_empty;
		uint8_t silence;
		uint8_t irq_flag;
	} dmc;
	
	enum { tag = 'APUR' };
	void swap();
};
BOOST_STATIC_ASSERT( sizeof (apu_snapshot_t) == 72 );

#endif

