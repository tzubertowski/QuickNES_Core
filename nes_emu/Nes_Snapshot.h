
// NES snapshot for saving and restoring emulator state

// Nes_Emu 0.5.6. Copyright (C) 2004-2005 Shay Green. GNU LGPL license.

#ifndef NES_SNAPSHOT_H
#define NES_SNAPSHOT_H

#include "Nes_File.h"
#include "Nes_Cpu.h"
class Nes_Emu;

typedef long frame_count_t;
frame_count_t const invalid_frame_count = LONG_MAX / 2; // a large positive value

class Nes_Snapshot;

class Nes_Snapshot_Array {
public:
	Nes_Snapshot_Array();
	~Nes_Snapshot_Array();
	
	// Change size of array
	const char * resize( int new_size );
	
	// Current size of array
	int size() const;
	
	Nes_Snapshot& operator [] ( int );
	Nes_Snapshot const& operator [] ( int ) const;
	
private:
	Nes_Snapshot* data;
	int size_;
};

class Nes_Snapshot_Writer : public Nes_File_Writer {
public:
	// See Nes_File.h
	const char * begin( Data_Writer* );
	
	// Write snapshot of current emulator state and finish writing file
	const char * end( Nes_Emu const& );
	
	// Write snapshot and finish writing file
	const char * end( Nes_Snapshot const& );
};

class Nes_Snapshot_Reader : public Nes_File_Reader {
public:
	Nes_Snapshot_Reader();
	~Nes_Snapshot_Reader();
	
	// Optionally read snapshot into designated location instead of
	// internal snapshot.
	const char * begin( Data_Reader*, Nes_Snapshot* out = NULL );
	
	// See Nes_File.h
	const char * next_block();
	
	// Snapshot valid after all blocks have been read
	Nes_Snapshot const& snapshot() const;
private:
	Nes_Snapshot_Array snapshots;
	Nes_Snapshot* snapshot_;
};

class Nes_Snapshot {
	Nes_Snapshot(); // use Nes_Snapshot_Array
	Nes_Snapshot( Nes_Snapshot const& );
public:
	
	// Invalidate all state
	void clear();
	
	// Change timestamp
	void set_timestamp( frame_count_t );
	
	// Timestamp snapshot was taken at
	frame_count_t timestamp() const;
	
	// Write snapshot to file
	const char * write( Data_Writer& ) const;
	
	// Read snapshot from file
	const char * read( Data_Reader& );
	
	// End of general interface
public:
	const char * write_blocks( Nes_File_Writer& ) const;
	void set_nes_state( nes_state_t const& );
	const char * read_blocks( Nes_File_Reader& );
private:
	
	nes_state_t nes;
	bool nes_valid;
	
	Nes_Cpu::registers_t cpu;
	bool cpu_valid;
	
	joypad_state_t joypad;
	bool joypad_valid;
	
	apu_snapshot_t apu;
	bool apu_valid;
	
	ppu_state_t ppu;
	bool ppu_valid;
	
	mapper_state_t mapper;
	bool mapper_valid;
	
	uint8_t ram [0x800];
	bool ram_valid;
	
	uint8_t sram [0x2000];
	int sram_size;
	
	uint8_t spr_ram [0x100];
	bool spr_ram_valid;
	
	uint8_t nametable [0x1000];
	int nametable_size;
	
	uint8_t chr [0x2000];
	int chr_size;
	
	friend class Nes_Emu;
	friend class Nes_Ppu_Impl;
};

inline Nes_Snapshot const& Nes_Snapshot_Reader::snapshot() const
{
	return *snapshot_;
}

inline const char * Nes_Snapshot_Writer::begin( Data_Writer* dw )
{
	return Nes_File_Writer::begin( dw, snapshot_file_tag );
}

inline void Nes_Snapshot::set_timestamp( frame_count_t t ) { nes.frame_count = t; }

inline frame_count_t Nes_Snapshot::timestamp() const { return nes.frame_count; }

inline int Nes_Snapshot_Array::size() const { return size_; }

inline Nes_Snapshot& Nes_Snapshot_Array::operator [] ( int i )
{
	return data [i];
}

inline Nes_Snapshot const& Nes_Snapshot_Array::operator [] ( int i ) const
{
	return data [i];
}

#endif

