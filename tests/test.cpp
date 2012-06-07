/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
test.cpp : a demonstration of msgpackalt C++ classes
----------------------------------------------------------------------
last modified by Martijn Jasperse, May 2012
*/
#include <algorithm>
#include <cstdio>

#define MSGPACK_INLINE
#include "msgpackalt.hpp"
using namespace msgpackalt;

void outhex( char c )   { std::printf( "%02X", ( byte )c ); }

int main( )
{
  uint32_t n;
  const int8_t v0[] = {-1,-2,-3};
  std::vector<int8_t> v(v0,v0+3); // static vector allocation
  
  packer p;
  p << 12345u << "pi" << 3.14159 << -9876543210 << v;
  
  std::string s = p.string();
  std::for_each( s.begin(), s.end(), outhex );
  v.clear( ); std::string str; int64_t i64; double f;
  
  unpacker u(p);
  u >> n >> str >> f >> i64 >> v;
  std::printf( "\nUnpacked with %u bytes remaining\n", u.len( ));
  return 0;
}
