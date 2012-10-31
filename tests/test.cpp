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

//////#define MSGPACK_INLINE
#include "msgpackalt.hpp"
using namespace msgpackalt;

void outhex( char c )   { std::printf( "%02X", ( byte )c ); }

int main( )
{
  uint32_t n;
  const int32_t v0[3] = {0,-1,-2};
  std::vector<int64_t> v(v0,v0+3); // static vector allocation
  
  printf( "***** Comparison C-test *****\n" );
  
  packer p;
  p << 12345u << "pi" << 3.14159 << -9876543210 << v;
  
  std::string s = p.string();
  std::for_each( s.begin(), s.end(), outhex );
  std::printf( "\nPacked into %u bytes\n", s.size() );
  v.clear( ); std::string str; int64_t i64; double f;
  
  unpacker u( s,true );
  u >> n >> str >> f >> i64 >> v;
  for ( int i = 0; i < v.size(); ++i ) printf( "%lli\n", v[i] );
  std::printf( "Unpacked with %u bytes remaining\n", u.len( ));
  
  std::time_t t = time(NULL);
  printf( "\n***** Map test *****\n" );
  
  p.clear( );
  pack_dict d1, d2;
  d1.pack( "method", "x" );
  d1.pack( "host", "me" );
  d1.pack( "time", std::ctime(&t));
  d2.pack( "level", 5 );
  d2.pack( "pri", 3u );
  d2.pack( "pi", 3.14159f );
  d1.pack( "params", d2 );
  p << d1;
  s = p.string();
  std::for_each( s.begin(), s.end(), outhex );
  
  u = s;
  unpack_dict ud;
  u >> ud;
  /*
  for ( std::map<std::string,unpacker>::const_iterator i = ud.data.begin(); i != ud.data.end(); ++i )
	puts( i->first.c_str() );
  */
  printf("\n\nDone\n");
  return 0;
}
