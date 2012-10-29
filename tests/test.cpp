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
#include <iostream>

#define MSGPACK_INLINE
#include "msgpackalt.hpp"
using namespace msgpackalt;

void outhex( char c )   { std::printf( "%02X", ( byte )c ); }

int main( )
{
  uint32_t n;
  const int32_t v0[3] = {0,-1,-2};
  std::vector<int64_t> v(v0,v0+3); // static vector allocation
  
  packer p;
  p << 12345u << "pi" << 3.14159 << -9876543210 << v;
  
  std::string s = p.string();
  std::for_each( s.begin(), s.end(), outhex );
  printf( "\nPacked into %u bytes\n", s.size() );
  v.clear( ); std::string str; int64_t i64; double f;
  
  unpacker u(p);
  u >> n >> str >> f >> i64 >> v;
  for ( int i = 0; i < v.size(); ++i ) printf( "%lli\n", v[i] );
  std::printf( "Unpacked with %u bytes remaining\n", u.len( ));
  return 0;
}
