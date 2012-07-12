/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
speed_test.c : a simple timed test of packing and unpacking
----------------------------------------------------------------------
last modified by Martijn Jasperse, May 2012
*/

#define MSGPACK_INLINE 1
#include "msgpackalt.h"

#include <stdio.h>
#include <assert.h>

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
    uint32_t n, m, l, x=0;
    float f;
    const byte *buffer = NULL;
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
        msgpack_u *u;
        msgpack_p *p = msgpack_pack_init();
        msgpack_pack_map( p, 1000 );
        for ( n = 0; n < 1000; ++n )
        {
            msgpack_pack_uint16( p, n );
            msgpack_pack_float( p, 1.0 );
        }
        msgpack_get_buffer( p, &buffer, &n );
    
        x = 0;
        u = msgpack_unpack_init( buffer, n, 0 );
        msgpack_unpack_map( u, &m );
        for ( n = 0; n < m; ++n )
        {
            msgpack_unpack_uint32( u, &l );
            x += l;
            msgpack_unpack_float( u, &f );
        }
        free( u );
        msgpack_pack_free( p );
    }
    
    #ifdef _MSC_VER
        QueryPerformanceCounter( &li );
        elapsed = ( li.QuadPart - start )/( double )freq;
    #else
        gettimeofday( &tv2, NULL );
        elapsed = ( tv2.tv_sec - tv1.tv_sec ) + ( tv2.tv_usec - tv1.tv_usec ) * 1e-6;
    #endif
    printf( "Elapsed = %f, Sum = %u\n", elapsed, x );
    return 0;
}
