/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
definition of C++ interface, using overloaded operators (<< and >>)
to simplify code syntax, with support for STL string, map and vector
----------------------------------------------------------------------
*/
#ifndef MSGPACK_HPP
#define MSGPACK_HPP

#include <string>
#include <map>
#include <vector>
#include <stdexcept>

namespace msgpackalt {
#include "msgpackalt.h"

void msgpack_assert( MSGPACK_ERR code )
{
	// interpret error codes and throw a relevant exception
	switch ( code ) {
		case MSGPACK_SUCCESS: 	return;
		case MSGPACK_TYPEERR: 	throw std::out_of_range("Unknown type code");
		case MSGPACK_MEMERR:	throw std::bad_alloc();
		case MSGPACK_ARGERR:	throw std::invalid_argument("Invalid argument");
		default:				throw std::bad_exception();
	}
}


class packer {
	public:
		// default constructor -- allocate an object
		packer( )       { this->m = msgpack_pack_init( ); }
		// default destructor -- clean up
		~packer( )      { msgpack_pack_free( this->m ); this->m = NULL; }
		
		// return the number of bytes packed so far
		uint32_t len( ) const                   { return msgpack_get_len( this->m ); }
		// create a STL string with the contents of the buffer
		std::string string( ) const             { const byte* b; uint32_t n; msgpack_get_buffer( this->m, &b, &n ); return std::string(( char* )b,n ); }
		// return pointer to underlying struct -- internal use only
		msgpack_p* ptr( )						{ return this->m; }
		// "clear" the contents of the buffer
		void clear( )							{ if ( this->m ) m->p = m->buffer; }
		
		// ***** PACKING FUNCTIONS *****
		//packer& operator<<( void* x )           { msgpack_assert( msgpack_pack_null( this->m )); return *this; }
		packer& operator<<( bool b )            { msgpack_assert( msgpack_pack_bool( this->m, b )); return *this; }
		
		packer& operator<<( const uint8_t &x )  { msgpack_assert( msgpack_pack_uint8( this->m, x )); return *this; }
		packer& operator<<( const uint16_t &x ) { msgpack_assert( msgpack_pack_uint16( this->m, x )); return *this; }
		packer& operator<<( const uint32_t &x ) { msgpack_assert( msgpack_pack_uint32( this->m, x )); return *this; }
		packer& operator<<( const uint64_t &x ) { msgpack_assert( msgpack_pack_uint64( this->m, x )); return *this; }
		
		packer& operator<<( const int8_t &x )   { msgpack_assert( msgpack_pack_int8( this->m, x )); return *this; }
		packer& operator<<( const int16_t &x )  { msgpack_assert( msgpack_pack_int16( this->m, x )); return *this; }
		packer& operator<<( const int32_t &x )  { msgpack_assert( msgpack_pack_int32( this->m, x )); return *this; }
		packer& operator<<( const int64_t &x )  { msgpack_assert( msgpack_pack_int64( this->m, x )); return *this; }
		
		packer& operator<<( const float &x )    { msgpack_assert( msgpack_pack_float( this->m, x )); return *this; }
		packer& operator<<( const double &x )   { msgpack_assert( msgpack_pack_double( this->m, x )); return *this; }
		
		packer& operator<<( const std::string &s )  { msgpack_assert( msgpack_pack_str( this->m, s.c_str( ))); return *this; }
		packer& operator<<( const char *s )         { msgpack_assert( msgpack_pack_str( this->m, s )); return *this; }
		
		// append the contents of another packer object to the stream
		packer& operator<<( const packer &p )		{ const byte* b; uint32_t n; msgpack_get_buffer( p.m, &b, &n ); msgpack_assert( msgpack_pack_append( this->m, b, n )); return *this; }
		
		// pack a C-style array of n points starting at the pointer address v
		template<class T> packer& pack_array( const T* v, const uint32_t n )
			{ msgpack_pack_array( this->m, n ); for ( uint32_t i = 0; i < n; ++i ) *this << v[i]; return *this; }
		// pack an STL vector of any valid type
		template<class T> packer& operator<<( const std::vector<T> &v )
			{ return this->pack_array( &v[0], v.size( )); }
		// pack an STL map between any valid types
		template<class T, class U> packer& operator<<( const std::map<T,U> &v )
			{ msgpack_pack_map( this->m, v.size()); for ( typename std::map<T,U>::const_iterator i = v.begin(); i != v.end(); ++i ) *this << i.key << i.val; return *this; }
		
	protected:
		packer& operator=( const packer& P );	// because reference-counting is not implement, DO NOT USE
		msgpack_p *m;
		friend class unpacker;
};


class unpacker {
	public:
		// default constructor: empty string (explicit to prevent unwanted copies)
		explicit unpacker( )
			{ this->u = msgpack_unpack_init( NULL, 0, true ); }
		// create unpacker from block of memory
		unpacker( const byte *buffer, uint32_t len, bool copy = false )
			{ this->u = msgpack_unpack_init( buffer, len, copy ); }
		// create unpacker as a copy of another unpacker
		explicit unpacker( const unpacker &p )
			{ unpacker( p.u->p, msgpack_unpack_len( this->u ), true ); }
		// create unpacker from std string
		unpacker( const std::string &s, bool copy = false )
			{ this->u = msgpack_unpack_init( s.c_str(),s.size(),copy ); }
		// default destructor
		~unpacker( )
			{ msgpack_unpack_free( this->u ); this->u = NULL; }
		
		// return number of bytes left to unpack
		uint32_t len( ) const                   { return msgpack_unpack_len( this->u ); }
		// return the code of the next item to unpack
		int peek( ) const        				{ return msgpack_unpack_peek( this->u ); }
		// skip the next item, return number of bytes passed
		int skip( )								{ return msgpack_unpack_skip( this->u ); }
		// reset the buffer
		void clear( )							{ this->u->end = this->u->p = this->u->end - this->u->max; }
		// return pointer to underlying struct -- internal use only
		msgpack_u* ptr( )						{ return this->u; }
		// append (streaming) data to be decoded
		uint32_t append( const byte *data, uint32_t len )	{ msgpack_assert( msgpack_unpack_append( this->u, data, len )); return this->u->max; }
		// throw away the buffer and set to given buffer
		uint32_t set( const byte *data, uint32_t len )		{ this->clear( ); this->append( data, len ); }
		// throw away the buffer and set to string contents
		unpacker& operator=( const std::string &s )		{ if ( this->u ) msgpack_unpack_free( this->u ); this->u = msgpack_unpack_init( s.c_str(),s.size(),true ); return *this; }
		
		// ***** UNPACKING FUNCTIONS *****
		//unpacker& operator>>( const void* x )   { msgpack_assert( msgpack_unpack_null( this->u )); return *this; }
		unpacker& operator>>( bool &b )         { int x = msgpack_unpack_bool( this->u ); b = x > 0; msgpack_assert(( MSGPACK_ERR )x ); }
		
		unpacker& operator>>( uint8_t &x )      { msgpack_assert( msgpack_unpack_uint8( this->u, &x )); return *this; }
		unpacker& operator>>( uint16_t &x )     { msgpack_assert( msgpack_unpack_uint16( this->u, &x )); return *this; }
		unpacker& operator>>( uint32_t &x )     { msgpack_assert( msgpack_unpack_uint32( this->u, &x )); return *this; }
		unpacker& operator>>( uint64_t &x )     { msgpack_assert( msgpack_unpack_uint64( this->u, &x )); return *this; }
		
		unpacker& operator>>( int8_t &x )       { msgpack_assert( msgpack_unpack_int8( this->u, &x )); return *this; }
		unpacker& operator>>( int16_t &x )      { msgpack_assert( msgpack_unpack_int16( this->u, &x )); return *this; }
		unpacker& operator>>( int32_t &x )      { msgpack_assert( msgpack_unpack_int32( this->u, &x )); return *this; }
		unpacker& operator>>( int64_t &x )      { msgpack_assert( msgpack_unpack_int64( this->u, &x )); return *this; }
		
		unpacker& operator>>( float &x )        { msgpack_assert( msgpack_unpack_float( this->u, &x )); return *this; }
		unpacker& operator>>( double &x )       { msgpack_assert( msgpack_unpack_double( this->u, &x )); return *this; }
		
		unpacker& operator>>( std::string &s )  { const byte* b; uint32_t n; msgpack_unpack_raw( this->u, &b, &n ); s = std::string(( const char* )b, n ); return *this; }
		
		// templated unpack into any type of STL vector
		template<class T> unpacker& operator>>( std::vector<T> &v )
			{ uint32_t n; T x; msgpack_unpack_array( this->u, &n ); v.clear( ); for ( uint32_t i = 0; i < n; ++i ) { *this >> x; v.push_back( x ); } return *this; }
		// templated unpack into any type of STL map
		template<class T, class U> unpacker& operator>>( std::map<T,U> &v )
			{ uint32_t n; T x; U y; msgpack_unpack_map( this->u, &n ); v.clear( ); for ( uint32_t i = 0; i < n; ++i ) { *this >> x >> y; v[x]=y; } return *this; }

	protected:
		packer& operator=( const packer& P );	// because reference-counting is not implement, DO NOT USE
		msgpack_u *u;
};

/*
// CURRENTLY TESTING

class pack_dict {
	public:
		pack_dict( )	{ n = 0; }
		~pack_dict( )	{ }
		
		template<class T> void pack( const char* name, const T& val )
			{ p << name << val; ++n; }
		friend packer& operator<<( packer &m, const pack_dict &d )
			{ msgpack_pack_map( m.ptr(), d.n ); m << d.p; return m; }
	
	protected:
		packer p;
		uint32_t n;
};

class unpack_dict {
	public:
		unpack_dict( )		{ }
		~unpack_dict( )		{ }
		
		friend unpacker& operator>>( unpacker &u, unpack_dict &d )
		{
			std::string s;
			uint32_t n;
			int k = 0;
			msgpack_unpack_map( u.ptr(), &n );
			for ( uint32_t i = 0; i < n; ++i )
			{
				u >> s;
				k = u.skip( );
				if ( k < 0 ) throw std::out_of_range("failed to process map");
				d.data[s].set( u.ptr()->p - k, k );
			}
		}
		
		template<class T> T operator[]( std::string s )	{ return data[s]; }
		
		std::map<std::string,unpacker> data;
};
*/
} // namespace

#endif
