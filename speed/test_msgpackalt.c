#include "speedtest.h"
#define MSGPACK_INLINE
#include <msgpackalt.h>

void test_msgpackalt( test_t* t, int nobj )
{
    int i, j;
	msgpack_p *p;
    msgpack_u *u;
    const byte *buffer;
    uint32_t len;
    char dummy[256];
    
    p = msgpack_pack_init( );
    for ( i = 0; i < nobj; ++i )
    {
        msgpack_pack_int32( p, t->id );
        msgpack_pack_int32( p, t->width );
        msgpack_pack_int32( p, t->height );
        msgpack_pack_str( p, t->str );
    }
    
    msgpack_get_buffer( p, &buffer, &len );
    u = msgpack_unpack_init( buffer, len );
    
    for ( i = 0; i < nobj; ++i )
    {
        msgpack_unpack_int32( u, &j );
        msgpack_unpack_int32( u, &j );
        msgpack_unpack_int32( u, &j );
        msgpack_unpack_str( u, dummy, 256 );
    }
    
    msgpack_pack_free( p );
    msgpack_unpack_free( u );
}

