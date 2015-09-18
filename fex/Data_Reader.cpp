// File_Extractor 1.0.0. http://www.slack.net/~ant/

#include "Data_Reader.h"

#include "blargg_endian.h"
#include <stdio.h>
#include <errno.h>

/* Copyright (C) 2005-2009 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

#include "blargg_source.h"

// Data_Reader

blargg_err_t Data_Reader::read( void* p, int n )
{
	assert( n >= 0 );
	
	if ( n < 0 )
		return blargg_err_caller;
	
	if ( n <= 0 )
		return blargg_ok;
	
	if ( n > remain() )
		return blargg_err_file_eof;
	
	blargg_err_t err = read_v( p, n );
	if ( !err )
		remain_ -= n;
	
	return err;
}

blargg_err_t Data_Reader::read_avail( void* p, int* n_ )
{
	assert( *n_ >= 0 );
	
	int n = min( (BOOST::uint64_t)(*n_), remain() );
	*n_ = 0;
	
	if ( n < 0 )
		return blargg_err_caller;
	
	if ( n <= 0 )
		return blargg_ok;
	
	blargg_err_t err = read_v( p, n );
	if ( !err )
	{
		remain_ -= n;
		*n_ = n;
	}
	
	return err;
}

blargg_err_t Data_Reader::read_avail( void* p, long* n )
{
	int i = STATIC_CAST(int, *n);
	blargg_err_t err = read_avail( p, &i );
	*n = i;
	return err;
}

blargg_err_t Data_Reader::skip_v( int count )
{
	char buf [512];
	while ( count )
	{
		int n = min( count, (int) sizeof buf );
		count -= n;
		RETURN_ERR( read_v( buf, n ) );
	}
	return blargg_ok;
}

blargg_err_t Data_Reader::skip( int n )
{
	assert( n >= 0 );
	
	if ( n < 0 )
		return blargg_err_caller;
	
	if ( n <= 0 )
		return blargg_ok;
	
	if ( n > remain() )
		return blargg_err_file_eof;
	
	blargg_err_t err = skip_v( n );
	if ( !err )
		remain_ -= n;
	
	return err;
}


// File_Reader

blargg_err_t File_Reader::seek( BOOST::uint64_t n )
{
	assert( n >= 0 );
	
	if ( n < 0 )
		return blargg_err_caller;
	
	if ( n == tell() )
		return blargg_ok;
	
	if ( n > size() )
		return blargg_err_file_eof;
	
	blargg_err_t err = seek_v( n );
	if ( !err )
		set_tell( n );
	
	return err;
}

blargg_err_t File_Reader::skip_v( BOOST::uint64_t n )
{
	return seek_v( tell() + n );
}


// Subset_Reader

Subset_Reader::Subset_Reader( Data_Reader* dr, BOOST::uint64_t size ) :
	in( dr )
{
	set_remain( min( size, dr->remain() ) );
}

blargg_err_t Subset_Reader::read_v( void* p, int s )
{
	return in->read( p, s );
}


// Remaining_Reader

Remaining_Reader::Remaining_Reader( void const* h, int size, Data_Reader* r ) :
	in( r )
{
	header        = h;
	header_remain = size;
	
	set_remain( size + r->remain() );
}

blargg_err_t Remaining_Reader::read_v( void* out, int count )
{
	int first = min( count, header_remain );
	if ( first )
	{
		memcpy( out, header, first );
		header = STATIC_CAST(char const*, header) + first;
		header_remain -= first;
	}
	
	return in->read( STATIC_CAST(char*, out) + first, count - first );
}


// Mem_File_Reader

Mem_File_Reader::Mem_File_Reader( const void* p, long s ) :
	begin( STATIC_CAST(const char*, p) )
{
	set_size( s );
}

blargg_err_t Mem_File_Reader::read_v( void* p, int s )
{
	memcpy( p, begin + tell(), s );
	return blargg_ok;
}

blargg_err_t Mem_File_Reader::seek_v( int )
{
	return blargg_ok;
}


// Callback_Reader

Callback_Reader::Callback_Reader( callback_t c, BOOST::uint64_t s, void* d ) :
	callback( c ),
	user_data( d )
{
	set_remain( s );
}

blargg_err_t Callback_Reader::read_v( void* out, int count )
{
	return callback( user_data, out, count );
}


// Callback_File_Reader

Callback_File_Reader::Callback_File_Reader( callback_t c, BOOST::uint64_t s, void* d ) :
	callback( c ),
	user_data( d )
{
	set_size( s );
}

blargg_err_t Callback_File_Reader::read_v( void* out, int count )
{
	return callback( user_data, out, count, tell() );
}

blargg_err_t Callback_File_Reader::seek_v( int )
{
	return blargg_ok;
}
