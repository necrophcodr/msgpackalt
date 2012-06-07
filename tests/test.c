/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
test.c : a simple demonstration of message packing
----------------------------------------------------------------------
last modified by Martijn Jasperse, May 2012
*/
#include <stdio.h>

#define MSGPACK_INLINE
#include "msgpackalt.h"

#include <time.h>

int main( )
{
  int8_t i, j;
  int64_t i64;
  /* some stuff to pack */
  double pi = 3.14159265358979323846;
  uint64_t t = time( NULL );
  char str[32] = "ABC123xyz";
  
  const byte *buffer;
  uint32_t n,k;
  msgpack_u *u;
  
  /* pack message */
  msgpack_p *p = msgpack_pack_init();
  msgpack_pack_uint16( p, 12345u );
  msgpack_pack_str( p, "pi" );
  msgpack_pack_double( p, 3.14159 );
  msgpack_pack_int64( p, -9876543210 );
  msgpack_pack_array( p, 3 );
  for ( i = 0; i < 3; ++i )
    msgpack_pack_fix( p, -i );
  msgpack_get_buffer( p, &buffer, &n );
  
  for ( k = 0; k < n; ++k )
    printf( "%02X", buffer[k] );
  printf( "\n" );
  printf( "Packed into %u bytes\n", n );
  
  /* unpack message */
  u = msgpack_unpack_init( buffer, n );
  msgpack_unpack_uint32( u, &k );
  msgpack_unpack_str( u, str, 32 );
  msgpack_unpack_double( u, &pi );
  msgpack_unpack_int64( u, &i64 );
  msgpack_unpack_array( u, &n );
  for ( k = 0; k < n; ++k )
  {
    msgpack_unpack_int64( u, &i64 );
    printf( "%lli\n", i64 );
  }
  n = msgpack_unpack_len( u );
  printf( "Unpacked with %u bytes remaining\n", n );
  
  msgpack_unpack_free( u );
  msgpack_pack_free( p );
  return 0;
}