/* Copyright notice for this file:
 * Copyright (C) 2018
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * This mapper was added by retrowertz for Libretro port of QuickNES.
 * 3/24/18
 *
 * VRC-3 Konami, Salamander
 */

#include "Nes_Mapper.h"

struct vrc3_state_t
{
	uint8_t  prg_bank;
	uint8_t  irq_control;
	uint8_t  irq_pending;
	uint8_t  unused;
	uint16_t irq_latch;
	uint16_t next_time;
};

BOOST_STATIC_ASSERT( sizeof ( vrc3_state_t ) == 8 );

class Mapper_VRC3 : public Nes_Mapper, vrc3_state_t {
public:
	Mapper_VRC3()
	{
		vrc3_state_t * state = this;
		register_state( state, sizeof * state );
	}

	void reset_state()
	{
	}

	void apply_mapping()
	{
		enable_sram();
		set_prg_bank( 0x8000, bank_16k, prg_bank );
	}

	virtual void run_until( nes_time_t end_time )
	{
		if ( irq_control & 2)
		{
			uint32_t tmp = irq_latch + ( end_time - next_time );

			if ( tmp > 0xffff )
				irq_pending = true;

			irq_latch = tmp;
		}

		next_time = end_time;
	}

	virtual void end_frame( nes_time_t end_time )
	{
		if ( end_time > next_time )
			run_until( end_time );
		next_time -= end_time;
	}

	virtual nes_time_t next_irq( nes_time_t present )
	{
		if ( irq_pending )
			return present;

		if ( !(irq_control & 2 ) )
			return no_irq;

		return 0x10000 - irq_latch + next_time;
	}

	void write( nes_time_t time, nes_addr_t addr, int data )
	{
		if ( ( addr & 0xF000 ) == 0xF000 )
		{
			prg_bank = data;
			set_prg_bank( 0x8000, bank_16k, data );
		}

		else
		{
			run_until( time );

			switch ( addr & 0xF000 )
			{
				case 0x8000:
					irq_latch &= 0xFFF0;
					irq_latch |= ( data & 0xF );
					break;
				case 0x9000:
					irq_latch &= 0xFF0F;
					irq_latch |= ( data & 0xF ) << 4;
					break;
				case 0xa000:
					irq_latch &= 0xF0FF;
					irq_latch |= ( data & 0xF ) << 8;
					break;
				case 0xb000:
					irq_latch &= 0x0FFF;
					irq_latch |= ( data & 0xF ) << 12;
					break;
				case 0xc000:
					irq_pending = false;
					irq_control = data;
					irq_changed();
					break;
				case 0xd000:
					irq_pending = false;
					irq_control &= ~2;
					irq_control |= ( irq_control << 1 ) & 2;
					irq_changed();
					break;
			}
		}
	}
};

void register_vrc3_mapper();
void register_vrc3_mapper()
{
	register_mapper< Mapper_VRC3> ( 73 );
}
