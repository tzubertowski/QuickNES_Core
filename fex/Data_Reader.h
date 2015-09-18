// Lightweight interface for reading data from byte stream

// File_Extractor 1.0.0
#ifndef DATA_READER_H
#define DATA_READER_H

#include "blargg_common.h"

/* Some functions accept a long instead of int for convenience where caller has
a long due to some other interface, and would otherwise have to get a warning,
or cast it (and verify that it wasn't outside the range of an int).

To really support huge (>2GB) files, long isn't a solution, since there's no
guarantee it's more than 32 bits. We'd need to use long long (if available), or
something compiler-specific, and change all places file sizes or offsets are
used. */

// Supports reading and finding out how many bytes are remaining
class Data_Reader {
public:

	// Reads min(*n,remain()) bytes and sets *n to this number, thus trying to read more
	// tham remain() bytes doesn't result in error, just *n being set to remain().
	blargg_err_t read_avail( void* p, int* n );
	blargg_err_t read_avail( void* p, long* n );

	// Reads exactly n bytes, or returns error if they couldn't ALL be read.
	// Reading past end of file results in blargg_err_file_eof.
	blargg_err_t read( void* p, int n );

	// Number of bytes remaining until end of file
	BOOST::uint64_t remain() const                              { return remain_; }

	// Reads and discards n bytes. Skipping past end of file results in blargg_err_file_eof.
	blargg_err_t skip( int n );
	
	virtual ~Data_Reader() { }

private:
	// noncopyable
	Data_Reader( const Data_Reader& );
	Data_Reader& operator = ( const Data_Reader& );

// Derived interface
protected:
	Data_Reader()                                   : remain_( 0 ) { }
	
	// Sets remain
	void set_remain( BOOST::uint64_t n )                        { remain_ = n; }
	
	// Do same as read(). Guaranteed that 0 < n <= remain(). Value of remain() is updated
	// AFTER this call succeeds, not before. set_remain() should NOT be called from this.
	virtual blargg_err_t read_v( void*, int n )     BLARGG_PURE( { (void)n; return 0; } )
	
	// Do same as skip(). Guaranteed that 0 < n <= remain(). Default just reads data
	// and discards it. Value of remain() is updated AFTER this call succeeds, not
	// before. set_remain() should NOT be called from this.
	virtual blargg_err_t skip_v( int n );

// Implementation
public:
	BLARGG_DISABLE_NOTHROW
	
private:
	BOOST::uint64_t remain_;
};


// Supports seeking in addition to Data_Reader operations
class File_Reader : public Data_Reader {
public:

	// Size of file
	BOOST::uint64_t size() const                    { return size_; }

	// Current position in file
	BOOST::uint64_t tell() const                    { return size_ - remain(); }

	// Goes to new position
	blargg_err_t seek( BOOST::uint64_t );

// Derived interface
protected:
	// Sets size and resets position
	void set_size( BOOST::uint64_t n )              { size_ = n; Data_Reader::set_remain( n ); }
	void set_size( int n )             { set_size( STATIC_CAST(BOOST::uint64_t, n) ); }
	void set_size( long n )             { set_size( STATIC_CAST(BOOST::uint64_t, n) ); }
	
	// Sets reported position
	void set_tell( BOOST::uint64_t i )              { Data_Reader::set_remain( size_ - i ); }
	
	// Do same as seek(). Guaranteed that 0 <= n <= size().  Value of tell() is updated
	// AFTER this call succeeds, not before. set_* functions should NOT be called from this.
	virtual blargg_err_t seek_v( BOOST::uint64_t n ) BLARGG_PURE( { (void)n; return 0; } )
	
// Implementation
protected:
	File_Reader()                       : size_( 0 ) { }
	
	virtual blargg_err_t skip_v( BOOST::uint64_t );

private:
	BOOST::uint64_t size_;
	
	void set_remain(); // avoid accidental use of set_remain
};


// Treats range of memory as a file
class Mem_File_Reader : public File_Reader {
public:

	Mem_File_Reader( const void* begin, long size );

// Implementation
protected:
	virtual blargg_err_t read_v( void*, int );
	virtual blargg_err_t seek_v( int );

private:
	const char* const begin;
};

#endif
