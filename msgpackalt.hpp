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

INLINE void msgpack_assert( MSGPACK_ERR code )
{
	// interpret error codes and throw a relevant exception on error
	switch ( code ) {
		case MSGPACK_SUCCESS: 	return;
		case MSGPACK_TYPEERR: 	throw std::out_of_range("Unexpected type code");
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
		std::string string( ) const             { const byte* b = NULL; uint32_t n = 0; msgpack_get_buffer( this->m, &b, &n ); return std::string(( char* )b,n ); }
		// return pointer to underlying struct -- internal use only
		msgpack_p* ptr( )						{ return this->m; }
		// "clear" the contents of the buffer
		void clear( )							{ if ( this->m ) m->p = m->buffer; }
		
		void start_map( uint32_t n )			{ msgpack_assert( msgpack_pack_map( this->m, n )); }
		void start_array( uint32_t n )			{ msgpack_assert( msgpack_pack_array( this->m, n )); }
		
		// ***** PACKING FUNCTIONS *****
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
		packer& operator<<( const packer &p )		{ const byte* b = NULL; uint32_t n = 0; msgpack_get_buffer( p.m, &b, &n ); msgpack_assert( msgpack_pack_append( this->m, b, n )); return *this; }
		
		packer& pack_null( )									{ msgpack_assert( msgpack_pack_null( this->m )); return *this; }
		packer& pack_raw( const void* data, const uint32_t n )	{ msgpack_assert( msgpack_pack_raw( this->m, ( const byte* )data, n )); return *this; }
		
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
			{ this->u = msgpack_unpack_init( NULL, 0, 1 ); }
		// create unpacker from block of memory
		unpacker( const byte *buffer, uint32_t len, bool copy = true )
			{ this->u = msgpack_unpack_init( buffer, len, copy ); }
		// create unpacker as a copy of another unpacker
		//explicit unpacker( const unpacker &p );
		//	{ unpacker( p.u->p, msgpack_unpack_len( p.u ), true ); } // problem line
		// create unpacker from std string
		unpacker( const std::string &s, bool copy = true )
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
		void set( const byte *data, uint32_t len )		{ this->clear( ); this->append( data, len ); }
		// throw away the buffer and set to string contents
		unpacker& operator=( const std::string &s )		{ if ( this->u ) msgpack_unpack_free( this->u ); this->u = msgpack_unpack_init( s.c_str(),s.size(),true ); return *this; }
		
		// ***** UNPACKING FUNCTIONS *****
		//unpacker& operator>>( const void* x )   { msgpack_assert( msgpack_unpack_null( this->u )); return *this; }
		unpacker& operator>>( bool &b )         { int x = msgpack_unpack_bool( this->u ); b = x > 0; msgpack_assert(( MSGPACK_ERR )x ); return *this; }
		
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
		
		unpacker& operator>>( std::string &s )  { const byte* b; uint32_t n; msgpack_assert( msgpack_unpack_raw( this->u, &b, &n )); s = std::string(( const char* )b, n ); return *this; }
		
		// templated unpack into any type of STL vector
		template<class T> unpacker& operator>>( std::vector<T> &v )
			{ uint32_t n = start_array( ); T x; v.clear( ); for ( uint32_t i = 0; i < n; ++i ) { *this >> x; v.push_back( x ); } return *this; }
		// templated unpack into any type of STL map
		template<class T, class U> unpacker& operator>>( std::map<T,U> &v )
			{ uint32_t n = start_map( ); T x; U y; v.clear( ); for ( uint32_t i = 0; i < n; ++i ) { *this >> x >> y; v[x]=y; } return *this; }
		
		uint32_t start_map( )		{ uint32_t n; msgpack_assert( msgpack_unpack_map( this->u, &n )); return n; }
		uint32_t start_array( )		{ uint32_t n; msgpack_assert( msgpack_unpack_array( this->u, &n )); return n; }
		
	protected:
		packer& operator=( const packer& P );	// because reference-counting is not implement, DO NOT USE
		msgpack_u *u;
};


#define OPERATOR_INST(x)	operator x ()	{ return this->as< x >(); }
class packed_object {
	public:
		packed_object( std::string k, std::string v ):data( v ),key( k ) { }
		packed_object( )	{ }
		~packed_object( )	{ }
		
		template<class T> T as() {
			unpacker u( this->data, false );	// we own this data so it's safe
			T x; u >> x;
			return x;
		}
		
		OPERATOR_INST(bool)
		OPERATOR_INST(int8_t)
		OPERATOR_INST(int16_t)
		OPERATOR_INST(int32_t)
		OPERATOR_INST(int64_t)
		OPERATOR_INST(uint8_t)
		OPERATOR_INST(uint16_t)
		OPERATOR_INST(uint32_t)
		OPERATOR_INST(uint64_t)
		OPERATOR_INST(float)
		OPERATOR_INST(double)
		OPERATOR_INST(std::string)
		
		std::string key, data;
};
#undef OPERATOR_INST

class pack_dict {
	/* a helper class to simplify the common use case of a string-indexed dictionary with heterogenous values
	   i.e. something which does not work in a std::map */
	public:
		pack_dict( )	{ n = 0; }
		~pack_dict( )	{ }
		
		// traditional interface
		template<class T> pack_dict& pack( const char* name, const T& val )
			{ p << name << val; ++n; return *this; }
		// non-tradition interface: make sure to use the insertion operator!
		packer &operator[]( const std::string &s )	{ ++n; return p << s; }
		// return packed string
		std::string string()	{ packer p; p << *this; return p.string(); }
		
		// add to a packer object
		friend packer& operator<<( packer &m, const pack_dict &d )
			{ m.start_map( d.n ); m << d.p; return m; }
		
	protected:
		packer p;
		uint32_t n;
};

class unpack_dict {
	/* a helper class to simplify unpacking a string-indexed dictionary with heterogenous values */
	public:
		unpack_dict( )							{ }
		unpack_dict( const std::string &s )		{ unpacker u(s,true); u >> *this; }
		unpack_dict( const packed_object &o )	{ unpacker u(o.data,true); u >> *this; }
		unpack_dict( unpacker &u )				{ u >> *this; }
		~unpack_dict( )		{ }// default destructor: clean up any unpack objects
		//	{ for ( entry_map::iterator i = data.begin(); i != data.end(); ++i ) delete i->second; }
		typedef std::map<std::string,packed_object> entry_map;
		typedef entry_map::const_iterator const_iterator;
		
		friend unpacker& operator>>( unpacker &u, unpack_dict &d )
		{
			size_t k; std::string s;
			// get the number of entries
			uint32_t n = u.start_map( );
			for ( uint32_t i = 0; i < n; ++i )
			{
				u >> s;			// read the name
				k = u.skip( );	// find out how many bytes the entry is
				if ( k < 0 ) throw std::out_of_range("failed to process map");
				// store a new unpacker object for this entry
				d.data[s] = packed_object( s, std::string(( const char* )( u.ptr()->p - k ), k ));
				//d.data[s] = new unpacker( u.ptr()->p - k, k, true ); // copy in case u is temporary
			}
			return u;
		}
		// traditional interface: returns true is exists otherwise leaves x alone
		template <class T> bool get( const std::string &s, T &x )
		{
			if ( !this->has_key( s )) return false;
			x = this->data[s].as<T>();
			return true;
		}
		// non-traditional interface: make sure to use the extraction operator!
		packed_object& operator[]( const std::string &s )
		{
			if ( !this->has_key( s )) throw std::out_of_range("key does not exist in map");
			return this->data[s];
		}
		// check for a given key
		bool has_key( const std::string &s )
		{
			return data.find(s) != data.end( );
		}
		
		entry_map::const_iterator begin()	{ return this->data.begin(); }
		entry_map::const_iterator end()		{ return this->data.end(); }
		
		entry_map data;
};

} // namespace

#endif
