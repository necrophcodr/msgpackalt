/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
speed_test0.c : comparison against the original msgpack library
----------------------------------------------------------------------
last modified by Martijn Jasperse, May 2012
*/

#include "msgpack.h"
#include <stdio.h>

#ifdef _MSC_VER
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <sys/time.h>
#endif
#include <time.h>

int main( )
{
  uint16_t i;
  uint32_t n, m, l, s;
  float f;
  double elapsed;
  
  #ifdef _MSC_VER
    LARGE_INTEGER li;
    __int64 start, freq;
    QueryPerformanceFrequency( &li );   
    freq = li.QuadPart;
    QueryPerformanceCounter( &li );
    start = li.QuadPart;
  #else
    struct timeval tv1, tv2;
    gettimeofday( &tv1, NULL );
  #endif
  
  for ( i = 0; i < 10000; ++i )
  {
    msgpack_sbuffer* buffer = msgpack_sbuffer_new();
    msgpack_packer* p = msgpack_packer_new(buffer, msgpack_sbuffer_write);
    msgpack_unpacked u;
    msgpack_object o;
    
    msgpack_pack_map( p, 1000 );
    for ( n = 0; n < 1000; ++n )
    {
        msgpack_pack_uint16( p, ( uint16_t )n );
        msgpack_pack_float( p, 1.0 );
    }
    
    msgpack_unpacked_init(&u);
    msgpack_unpack_next( &u, buffer->data, buffer->size, NULL );
    o = u.data;
    m = o.via.map.size;
    s = 0;
    for ( n = 0; n < m; ++n )
    {
        l = o.via.map.ptr[n].key.via.u64;
        f = o.via.map.ptr[n].val.via.dec;
        s += l;
    }
    
    msgpack_unpacked_destroy(&u);
    msgpack_sbuffer_free(buffer);
    msgpack_packer_free(p);
  }
  
  #ifdef _MSC_VER
    QueryPerformanceCounter( &li );
    elapsed = ( li.QuadPart - start )/( double )freq;
  #else
    gettimeofday( &tv2, NULL );
    elapsed = ( tv2.tv_sec - tv1.tv_sec ) + ( tv2.tv_usec - tv1.tv_usec ) * 1e-6;
  #endif
  printf( "Elapsed = %f, Sum = %u\n", elapsed, s );
  return 0;
}
