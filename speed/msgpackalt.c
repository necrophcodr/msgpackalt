#define MSGPACK_INLINE
#include <msgpackalt.h>

#include "speedtest.h"

void test_msgpackalt( int nobj )
{
    msgpack_p *p;
    msgpack_u *u;
    
    p = msgpack_pack_init( );
    
    
    u = msgpack_unpack_init( );
    
    msgpack_pack_free( p );
    free( u );
}

