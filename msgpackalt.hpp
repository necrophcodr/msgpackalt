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

namespace msgpackalt {
#include "msgpackalt.h"

class packer {
	public:
		packer( )       { m = msgpack_pack_init( ); }
		~packer( )      { msgpack_pack_free( this->m ); m = NULL; }
		
		uint32_t len( ) const                   { return msgpack_get_len( this->m ); }
		
		std::string string( ) const             { const byte* b; uint32_t n; msgpack_get_buffer( this->m, &b, &n ); return std::string(( char* )b,n ); }
		
		packer& operator<<( void* x )           { return this->check( msgpack_pack_null( this->m )); }
		packer& operator<<( bool b )            { return this->check( msgpack_pack_bool( this->m, b )); }
		
		packer& operator<<( const uint8_t &x )  { return this->check( msgpack_pack_uint8( this->m, x )); }
		packer& operator<<( const uint16_t &x ) { return this->check( msgpack_pack_uint16( this->m, x )); }
		packer& operator<<( const uint32_t &x ) { return this->check( msgpack_pack_uint32( this->m, x )); }
		packer& operator<<( const uint64_t &x ) { return this->check( msgpack_pack_uint64( this->m, x )); }
		
		packer& operator<<( const int8_t &x )   { return this->check( msgpack_pack_int8( this->m, x )); }
		packer& operator<<( const int16_t &x )  { return this->check( msgpack_pack_int16( this->m, x )); }
		packer& operator<<( const int32_t &x )  { return this->check( msgpack_pack_int32( this->m, x )); }
		packer& operator<<( const int64_t &x )  { return this->check( msgpack_pack_int64( this->m, x )); }
		
		packer& operator<<( const float &x )    { return this->check( msgpack_pack_float( this->m, x )); }
		packer& operator<<( const double &x )   { return this->check( msgpack_pack_double( this->m, x )); }
		
		packer& operator<<( const std::string &s )  { return this->check( msgpack_pack_str( this->m, s.c_str( ))); }
		packer& operator<<( const char *s )         { return this->check( msgpack_pack_str( this->m, s )); }
		
		template<class T> packer& operator<<( const std::vector<T> &v )
			{ msgpack_pack_array( this->m, v.size()); for ( typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); ++i ) *this << *i; return *this; }
		template<class T, class U> packer& operator<<( const std::map<T,U> &v )
			{ msgpack_pack_map( this->m, v.size()); for ( typename std::map<T,U>::const_iterator i = v.begin(); i != v.end(); ++i ) *this << i.key << i.val; return *this; }
		
	protected:
		inline packer& check( MSGPACK_ERR x )   { if ( x ) throw x; return *this; }
		msgpack_p *m;
		friend class unpacker;
};

class unpacker {
	public:
		unpacker( const byte *buffer, uint32_t len, bool copy = false )
			{ this->u = msgpack_unpack_init( buffer, len, copy ); }
		unpacker( const packer &p )
			{ const byte *b; uint32_t n; msgpack_get_buffer( p.m, &b, &n ); this->u = msgpack_unpack_init( b,n,0 ); }
		~unpacker( )
			{ free( this->u ); }
		
		uint32_t len( ) const                   { return msgpack_unpack_len( this->u ); }
		int peek( ) const        				{ return msgpack_unpack_peek( this->u ); }
		
		uint32_t append( const byte *data, uint32_t len )	{ this->check( msgpack_unpack_append( this->u, data, len )); return this->u->max; }
		
		unpacker& operator>>( const void* x )   { return this->check( msgpack_unpack_null( this->u )); }
		unpacker& operator>>( bool &b )         { int x = msgpack_unpack_bool( this->u ); b = x > 0; return check(( MSGPACK_ERR )x ); }
		
		unpacker& operator>>( uint8_t &x )      { return this->check( msgpack_unpack_uint8( this->u, &x )); }
		unpacker& operator>>( uint16_t &x )     { return this->check( msgpack_unpack_uint16( this->u, &x )); }
		unpacker& operator>>( uint32_t &x )     { return this->check( msgpack_unpack_uint32( this->u, &x )); }
		unpacker& operator>>( uint64_t &x )     { return this->check( msgpack_unpack_uint64( this->u, &x )); }
		
		unpacker& operator>>( int8_t &x )       { return this->check( msgpack_unpack_int8( this->u, &x )); }
		unpacker& operator>>( int16_t &x )      { return this->check( msgpack_unpack_int16( this->u, &x )); }
		unpacker& operator>>( int32_t &x )      { return this->check( msgpack_unpack_int32( this->u, &x )); }
		unpacker& operator>>( int64_t &x )      { return this->check( msgpack_unpack_int64( this->u, &x )); }
		
		unpacker& operator>>( float &x )        { return this->check( msgpack_unpack_float( this->u, &x )); }
		unpacker& operator>>( double &x )       { return this->check( msgpack_unpack_double( this->u, &x )); }
		
		unpacker& operator>>( std::string &s )  { const byte* b; uint32_t n; msgpack_unpack_raw( this->u, &b, &n ); s = std::string(( const char* )b, n ); return *this; }
		
		template<class T> unpacker& operator>>( std::vector<T> &v )
			{ uint32_t n; T x; msgpack_unpack_array( this->u, &n ); v.clear( ); for ( uint32_t i = 0; i < n; ++i ) { *this >> x; v.push_back( x ); } return *this; }
		template<class T, class U> unpacker& operator>>( std::map<T,U> &v )
			{ uint32_t n; T x; U y; msgpack_unpack_map( this->u, &n ); v.clear( ); for ( uint32_t i = 0; i < n; ++i ) { *this >> x >> y; v[x]=y; } return *this; }

	protected:
		inline unpacker& check( MSGPACK_ERR x ) { if ( x ) throw x; return *this; }
		msgpack_u *u;
};

} // namespace

#endif
