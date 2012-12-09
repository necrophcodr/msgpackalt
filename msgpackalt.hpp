/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
*/
/** \file msgpackalt.hpp
 *  \brief A simple C++ interface, using overloaded operators (<< and >>)
 *  to simplify code syntax, with support for STL strings, maps, vectors
 *  and dictionaries.
*/
#ifndef MSGPACK_HPP
#define MSGPACK_HPP

#include <string>
#include <map>
#include <vector>
#include <stdexcept>

namespace msgpackalt {
#include "msgpackalt.h"

/// Takes the msgpackalt return code and throws a relevant exception on error 
/** The underlying msgpack C library returns negative values denoting failure.
 *	A different exception is thrown for each error type so errors can be trapped.
 *	
 *	Note that all can be caught as std::exception and the what() function can
 *	be used to interpret the cause.
 *	
 *	MSGPACK_TYPEERR: function call does not match the expected type code
 *	                    throws std::out_of_range
 *	MSGPACK_MEMERR:  failed to allocate sufficient memory for the operation
 *	                    throws std::bad_alloc
 *	MSGPACK_ARGERR:  function called with invalid argument
 *						throws std::invalid_argument
 *	other error:     received negative return code, but unknown cause
 *						throws std::bad_exception
 */	
INLINE void msgpack_assert( MSGPACK_ERR code )
{
	if ( code < MSGPACK_SUCCESS ) 	// error codes are negative
		if ( code == MSGPACK_TYPEERR )
			throw std::out_of_range("Unexpected type code");
		else if ( code == MSGPACK_MEMERR )
			throw std::bad_alloc();
		else if ( code == MSGPACK_ARGERR )
			throw std::invalid_argument("Invalid argument");
		else
			throw std::bad_exception();
}

/// The serialisation class which packs data in the MessagePack format
class packer {
	public:
		/// default copy constructor; allocate a msgpack packer object
		packer( )       	{ this->m = msgpack_pack_init( ); }
		/// default destructor; cleans up any allocated memory
		~packer( )      	{ msgpack_pack_free( this->m ); this->m = NULL; }
		
		/// return pointer to underlying C structs -- internal use only
		msgpack_p* ptr( )	{ return this->m; }
		
		/// return the number of bytes packed so far
		uint32_t len( ) const                   { return msgpack_get_len( this->m ); }
		/// return an STL string with the contents of the buffer
		std::string string( ) const             { const byte* b = NULL; uint32_t n = 0; msgpack_get_buffer( this->m, &b, &n ); return std::string(( char* )b,n ); }
		/// clears the contents of the internal buffer
		void clear( )							{ if ( this->m ) m->p = m->buffer; }
		
		/// LOW-LEVEL: specifies an array is to follow, with elements consisting of the next "n" packing calls.
		void start_array( uint32_t n )			{ msgpack_assert( msgpack_pack_array( this->m, n )); }
		/// LOW-LEVEL: specifies a map is to follow, with "n" sets of keys and values consisting of the next 2*n calls.
		void start_map( uint32_t n )			{ msgpack_assert( msgpack_pack_map( this->m, n )); }
		
		// *********************************** PACKING FUNCTIONS ***********************************
		/// Pack a boolean value
		packer& operator<<( bool b )            { msgpack_assert( msgpack_pack_bool( this->m, b )); return *this; }
		/// Pack an 8-bit unsigned int
		packer& operator<<( const uint8_t &x )  { msgpack_assert( msgpack_pack_uint8( this->m, x )); return *this; }
		/// Pack a 16-bit unsigned int
		packer& operator<<( const uint16_t &x ) { msgpack_assert( msgpack_pack_uint16( this->m, x )); return *this; }
		/// Pack a 32-bit unsigned int
		packer& operator<<( const uint32_t &x ) { msgpack_assert( msgpack_pack_uint32( this->m, x )); return *this; }
		/// Pack a 64-bit unsigned int
		packer& operator<<( const uint64_t &x ) { msgpack_assert( msgpack_pack_uint64( this->m, x )); return *this; }
		
		/// Pack an 8-bit signed int
		packer& operator<<( const int8_t &x )   { msgpack_assert( msgpack_pack_int8( this->m, x )); return *this; }
		/// Pack a 16-bit signed int
		packer& operator<<( const int16_t &x )  { msgpack_assert( msgpack_pack_int16( this->m, x )); return *this; }
		/// Pack a 32-bit signed int
		packer& operator<<( const int32_t &x )  { msgpack_assert( msgpack_pack_int32( this->m, x )); return *this; }
		/// Pack a 64-bit signed int
		packer& operator<<( const int64_t &x )  { msgpack_assert( msgpack_pack_int64( this->m, x )); return *this; }
		
		/// Pack a 32-bit float
		packer& operator<<( const float &x )    { msgpack_assert( msgpack_pack_float( this->m, x )); return *this; }
		/// Pack a double (64-bit float)
		packer& operator<<( const double &x )   { msgpack_assert( msgpack_pack_double( this->m, x )); return *this; }
		
		/// Pack a std::string as raw data
		packer& operator<<( const std::string &s ) 
			{ msgpack_assert( msgpack_pack_str( this->m, s.c_str( ))); return *this; }
		/// Pack a C-style string as raw data
		packer& operator<<( const char *s )
			{ msgpack_assert( msgpack_pack_str( this->m, s )); return *this; }
		/// Pack "n" bytes of raw data specified by the given pointer
		packer& pack_raw( const void* data, const uint32_t n )
			{ msgpack_assert( msgpack_pack_raw( this->m, ( const byte* )data, n )); return *this; }
		/// Pack the "null" object
		packer& pack_null( )
			{ msgpack_assert( msgpack_pack_null( this->m )); return *this; }
		
		/// Append the contents of another packer object
		packer& operator<<( const packer &p )
			{ const byte* b = NULL; uint32_t n = 0; msgpack_get_buffer( p.m, &b, &n ); msgpack_assert( msgpack_pack_append( this->m, b, n )); return *this; }
		
		/// Pack a C-style array of "n" points starting at the pointer "v"
		template<class T> packer& pack_array( const T* v, const uint32_t n )
			{ msgpack_pack_array( this->m, n ); for ( uint32_t i = 0; i < n; ++i ) *this << v[i]; return *this; }
		/// Pack an STL vector of any valid type
		template<class T> packer& operator<<( const std::vector<T> &v )
			{ return this->pack_array( &v[0], v.size( )); }
		/// Pack an STL map between any valid types
		template<class T, class U> packer& operator<<( const std::map<T,U> &v )
			{ msgpack_pack_map( this->m, v.size()); for ( typename std::map<T,U>::const_iterator i = v.begin(); i != v.end(); ++i ) *this << i.key << i.val; return *this; }
		
	protected:
		/// Underlying C packer object
		msgpack_p *m;
		friend class unpacker;
		
	private:
		/// Pointers are not reference counted, so prevent automatic copies. Use the << operator to append instead.
		packer& operator=( const packer& P );
};


/// The deserialisation class which retrieves data packed in the MessagePack format from a binary string
class unpacker {
	public:
		/// Default constructor: start with the empty string (explicit to prevent unwanted copies)
		explicit unpacker( )
			{ this->u = msgpack_unpack_init( NULL, 0, 1 ); }
		/// Create unpacker from a raw block of memory
		unpacker( const byte *buffer, uint32_t len, bool copy = true )
			{ this->u = msgpack_unpack_init( buffer, len, copy ); }
		/// Create unpacker from std::string
		unpacker( const std::string &s, bool copy = true )
			{ this->u = msgpack_unpack_init( s.c_str(),s.size(),copy ); }
		/// Default destructor
		~unpacker( )
			{ msgpack_unpack_free( this->u ); this->u = NULL; }
		
		/// Return pointer to underlying struct -- internal use only
		msgpack_u* ptr( )						{ return this->u; }
		
		/// Return number of bytes remaining in the buffer to unpack
		uint32_t len( ) const                   { return msgpack_unpack_len( this->u ); }
		/// Return the code of the next item to unpack
		int peek( ) const        				{ return msgpack_unpack_peek( this->u ); }
		/// Skip the next item in the buffer and return the number of bytes skipped
		int skip( )								{ return msgpack_unpack_skip( this->u ); }
		
		/// Clear the buffer
		void clear( )
			{ this->u->end = this->u->p = this->u->end - this->u->max; }
		/// Append data to the end of the buffer, e.g. streaming data, and return the total size of the buffer (not necessarily bytes remaining to be unpacked)
		uint32_t append( const byte *data, uint32_t len )
			{ msgpack_assert( msgpack_unpack_append( this->u, data, len )); return this->u->max; }
		/// Throw away the current buffer and copy the given data
		void set( const byte *data, uint32_t len )
			{ this->clear( ); this->append( data, len ); }
		/// Throw away the current buffer and copy the given string
		unpacker& operator=( const std::string &s )
			{ this->clear( ); this->append(( byte* )s.c_str( ), s.size( )); return *this; }
		
		// *********************************** UNPACKING FUNCTIONS ***********************************
		/// Unpack a boolean value
		unpacker& operator>>( bool &b )         { int x = msgpack_unpack_bool( this->u ); b = x > 0; msgpack_assert(( MSGPACK_ERR )x ); return *this; }

		/// Unpack an 8-bit unsigned int
		unpacker& operator>>( uint8_t &x )      { msgpack_assert( msgpack_unpack_uint8( this->u, &x )); return *this; }
		/// Unpack a 16-bit unsigned int
		unpacker& operator>>( uint16_t &x )     { msgpack_assert( msgpack_unpack_uint16( this->u, &x )); return *this; }
		/// Unpack a 32-bit unsigned int
		unpacker& operator>>( uint32_t &x )     { msgpack_assert( msgpack_unpack_uint32( this->u, &x )); return *this; }
		/// Unpack a 64-bit unsigned int
		unpacker& operator>>( uint64_t &x )     { msgpack_assert( msgpack_unpack_uint64( this->u, &x )); return *this; }
		
		/// Unpack an 8-bit signed int
		unpacker& operator>>( int8_t &x )       { msgpack_assert( msgpack_unpack_int8( this->u, &x )); return *this; }
		/// Unpack a 16-bit signed int
		unpacker& operator>>( int16_t &x )      { msgpack_assert( msgpack_unpack_int16( this->u, &x )); return *this; }
		/// Unpack a 32-bit signed int
		unpacker& operator>>( int32_t &x )      { msgpack_assert( msgpack_unpack_int32( this->u, &x )); return *this; }
		/// Unpack a 64-bit signed int
		unpacker& operator>>( int64_t &x )      { msgpack_assert( msgpack_unpack_int64( this->u, &x )); return *this; }
		
		/// Unpack a U8 value
		unpacker& operator>>( float &x )        { msgpack_assert( msgpack_unpack_float( this->u, &x )); return *this; }
		/// Unpack a U8 value
		unpacker& operator>>( double &x )       { msgpack_assert( msgpack_unpack_double( this->u, &x )); return *this; }
		
		/// Unpack raw data into a std::string
		unpacker& operator>>( std::string &s )  { const byte* b; uint32_t n; msgpack_assert( msgpack_unpack_raw( this->u, &b, &n )); s = std::string(( const char* )b, n ); return *this; }
		
		/// Unpack a vector of homogeneous (single typed) data into the given STL vector
		template<class T> unpacker& operator>>( std::vector<T> &v )
			{ uint32_t n = start_array( ); T x; v.clear( ); for ( uint32_t i = 0; i < n; ++i ) { *this >> x; v.push_back( x ); } return *this; }
		/// Unpack a map object with key and value types given by the STL map
		template<class T, class U> unpacker& operator>>( std::map<T,U> &v )
			{ uint32_t n = start_map( ); T x; U y; v.clear( ); for ( uint32_t i = 0; i < n; ++i ) { *this >> x >> y; v[x]=y; } return *this; }
		
		/// LOW-LEVEL: Expect the next object to be the start of an array. Return the number of entries N, comprising the next N unpack calls.
		uint32_t start_array( )		{ uint32_t n; msgpack_assert( msgpack_unpack_array( this->u, &n )); return n; }
		/// LOW-LEVEL: Expect the next object to be the start of a map. Return the number of (key,value) pairs N, comprising the next 2*N unpack calls.
		uint32_t start_map( )		{ uint32_t n; msgpack_assert( msgpack_unpack_map( this->u, &n )); return n; }
		
	protected:
		/// Underlying C unpacker object
		msgpack_u *u;
		
	private:
		/// Pointers are not reference counted, so prevent automatic copies. Use the << operator to append instead.
		unpacker& operator=( const unpacker& P );
};


/// Helper class to pack "dictionaries" (string-indexed maps with varying value types)
class pack_dict {
	public:
		/// Default constructor
		pack_dict( )	{ n = 0; }
		/// Default destructor
		~pack_dict( )	{ }
		
		/// Pack a (key,value) pair into the map
		template<class T> pack_dict& pack( const char* key, const T& value )
			{ p << key << value; ++n; return *this; }
		/// Advanced interface: pack a (key,value) pair using the insertion operator
		packer &operator[]( const std::string &s )	{ ++n; return p << s; }
		
		/// Return std::string containing the packed dictionary data
		std::string string()	{ packer p; p << *this; return p.string(); }
		
		/// Write the dictionary data to a packer buffer
		friend packer& operator<<( packer &m, const pack_dict &d )
			{ m.start_map( d.n ); m << d.p; return m; }
		
	protected:
		/// Packer object containing data written so far
		packer p;
		/// Number of objects currently in map
		uint32_t n;
};


/// A simple class used with unpack_dict to contain a single object. Enables type-casting instead of unpacking for syntax simplification.
class unpack_object {
	public:
		/// Construct an object from the given std::string data
		unpack_object( std::string v ):data( v ) { }
		/// Default constructor - required for use with std::map
		unpack_object( )	{ }
		/// Default destructor
		~unpack_object( )	{ }
		/// Attempt to unpack the object as the specified datatype 
		template<class T> T as() {
			unpacker u( this->data, false );	// we own this data so it's safe not to copy
			T x; u >> x;
			return x;
		}
		/// Cast the object to the specified type - syntactic simplification of as<> call
		template<class T> operator T()	{ return this->as<T>(); }
		/// MessagePack'd object data
		std::string data;
};

/// Helper class to unpack "dictionaries" (string-indexed maps with varying value types)
class unpack_dict {
	public:
		/// Default constructor
		unpack_dict( )						{ }
		/// Construct dictionary from the given string buffer
		unpack_dict( const std::string &s )	{ unpacker u(s,true); u >> *this; }
		/// Construct dictionary by unpacking from the given unpacker object
		unpack_dict( unpacker &u )			{ u >> *this; }
		/// Construct dictionary from unpacked object (nested maps)
		unpack_dict( const unpack_object &obj )	{ unpacker u(obj.data,false); u >> *this; }
		/// Default deconstructor
		~unpack_dict( )						{ }
		
		/// Convenience type of internal map object
		typedef std::map<std::string,unpack_object> entry_map;
		/// Convenience type of const map iterator ("first" contains the key name, "second" the value)
		typedef entry_map::const_iterator const_iterator;
		
		/// Friend function which extracts entire map from unpacker buffer
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
				d.data[s] = std::string(( const char* )( u.ptr()->p - k ), k );
			}
			return u;
		}
		/// Unpack the value corresponding to key as type T. Returns true on success or false on failure.
		template <class T> bool get( const std::string &key, T &x )
		{
			if ( !this->has_key( key )) return false;
			x = this->data[key].as<T>();
			return true;
		}
		/// Return an unpack object corresponding to the given key, or throw std::out_of_range if it doesn't exist
		unpack_object& operator[]( const std::string &key )
		{
			if ( !this->has_key( key )) throw std::out_of_range("key does not exist in map");
			return this->data[key];
		}
		/// Check whether a given key exists in the map
		bool has_key( const std::string &key )
		{
			return data.find(key) != data.end( );
		}
		
		/// Returns a const iterator to the start of the data
		entry_map::const_iterator begin()	{ return this->data.begin(); }
		/// Returns a const iterator to the end of the data
		entry_map::const_iterator end()		{ return this->data.end(); }
		/// Data array
		entry_map data;
};

} // namespace

#endif
