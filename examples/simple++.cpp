/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
test.cpp : a demonstration of msgpackalt C++ classes
----------------------------------------------------------------------
*/
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <inttypes.h>

#define MSGPACK_INLINE
#include "msgpackalt.hpp"
using namespace msgpackalt;

void outhex( char c )   { printf( "%02X", ( byte )c ); }

int main( )
{
	printf( "***** Simple example, C++ version *****\n" );
	
	uint32_t n;
	const int32_t v0[3] = {0,-1,-2};
	std::vector<int32_t> v(v0,v0+3); // static vector allocation
	
	// very simple to pack a sequence of values
	try {
		packer p;
		p << 12345u << "pi" << 3.14159 << -9876543210 << v;
		
		// much more code to print it!
		std::string s = p.string();
		std::for_each( s.begin(), s.end(), outhex );
		std::printf( "\nPacked into %u bytes\n", s.size() );
		
		v.clear( ); std::string str; int64_t i64; double f;
		
		// unpack the string
		unpacker u = s;
		
		u >> n >> str >> f >> i64 >> v;
		// print the values to screen
		for ( size_t i = 0; i < v.size(); ++i )
			printf( "%d\n", v[i] );
		//std::printf( "Unpacked with %u bytes remaining\n", u.len( ));
	} catch ( std::exception &e ) {
		fprintf( stderr, "EXCEPTION: %s", e.what());
	}
	return 0;
}
