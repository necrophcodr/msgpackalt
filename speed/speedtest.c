#include "speedtest.h"
#include <stdio.h>

#ifdef _MSC_VER
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    typedef __int64 TIMER_T;
    TIMER_T start_timer( )
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter( &li );
        return li.QuadPart;
    }
    double stop_timer( TIMER_T t0 )
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter( &li );
        t0 = li.QuadPart - t0;
        QueryPerformanceFrequency( &li );   
        return t0 / ( double )li.QuadPart;
    }
#else
    #include <sys/time.h>
    typedef struct timeval TIMER_T;
    inline TIMER_T start_timer( )
    {
        TIMER_T t;
        gettimeofday( &t, NULL );
        return t;
    }
    inline double stop_timer( TIMER_T t0 )
    {
        TIMER_T t;
        gettimeofday( &t, NULL );
        return ( t.tv_sec - t0.tv_sec ) + ( t.tv_usec - t0.tv_usec ) * 1e-6;
    }
#endif


int main( int nargs, char** args )
{
    TIMER_T T;
    int i, j, n=0, m=0, nobj;
    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    
    test_t t = { 1, 2, 3, "Blah" };
    
    if ( nargs < 2 )
    {
        printf( "Usage: %s n m [nobj]\n\tn = number of sequential packs to measure\n\tm = number of repeats to average\n\tnobj = number of objects to pack\n", args[0] );
        return 1;
    }
    n = atoi( args[1] );
    m = atoi( args[2] );
    nobj = ( nargs > 3 ) ? atoi( args[3] ) : 100;
    
    printf( "\nBeginning pack speed-run:\n\t %i packs, %i repeats\n\n", n, m );
    fflush( stdout );
    
    for ( j = 0; j < m; ++j )
    {
        T = start_timer( );
        for ( i = 0; i < n; ++i )
        {
            test_msgpackalt( &t, nobj );
        }
        t1 += stop_timer( T );
        
        T = start_timer( );
        for ( i = 0; i < n; ++i )
        {
            test_msgpack( &t, nobj );
        }
        t2 += stop_timer( T );
        
        T = start_timer( );
        for ( i = 0; i < n; ++i )
        {
            test_protobuf( &t, nobj );
        }
        t3 += stop_timer( T );
        
        T = start_timer( );
        for ( i = 0; i < n; ++i )
        {
            test_yajl( &t, nobj );
        }
        t4 += stop_timer( T );
    }
    printf( "Results:\n" );
    printf( ">> msgpackalt  : %f\n", t1/m );
    printf( ">> MessagePack : %f\n", t2/m );
    printf( ">> Protobuf    : %f\n", t3/m );
    printf( ">> YAJL JSON   : %f\n", t4/m );
    
    return 0;
}


