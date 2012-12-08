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

#define MSGPACK_INLINE
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
  printf( "\n***** Dict test *****\n" );
  p.clear( );
  
  // let's pack a dictionary of values -----
  // the pack_dict object supports two syntaxes: this first example shows the object style
  pack_dict d1;
  d1["method"] << "simple";
  d1["time"] << std::ctime(&t);
  d1["level"] << 5;
  // dictionaries can be nested too -- this second example uses the function syntax
  pack_dict d2;
  d2.pack( "host", "me" );
  d2.pack( "max", 3 );
  d2.pack( "pi", 3.14159f );
  d1.pack( "params", d2 );
  // pack that into the buffer
  p << d1;
  
  // print the result to screen
  s = p.string();
  std::for_each( s.begin(), s.end(), outhex );
  puts( "" );
  
  // it's easy to unpack the dictionaries too, again using either function or object notation
  unpack_dict ud(s);
  ud.get( "method", s );
  n = ud["level"];
  // here we unpack the nested dictionary
  unpack_dict params( ud["params"] );
  
  // the entries in the dictionary can be directly interrogated
  printf( "\nNames in message:\n" );
  for ( unpack_dict::const_iterator i = ud.begin(); i != ud.end(); ++i )
	printf( " > %s\n", i->first.c_str( ));
  printf( "\nNames in 'params':\n" );
  for ( unpack_dict::const_iterator i = params.begin(); i != params.end(); ++i )
	printf( " > %s\n", i->first.c_str( ));
  return 0;
}
