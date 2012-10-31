/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
test0.c : equivalent message packing operations with original library
----------------------------------------------------------------------
*/
#include "msgpack.h"

#include <stdio.h>

int main( )
{
    int8_t i;
    uint32_t n=0,k;
    msgpack_sbuffer* buffer = msgpack_sbuffer_new();
    msgpack_packer* p = msgpack_packer_new(buffer, msgpack_sbuffer_write);
    msgpack_unpacked u;
    msgpack_object o;
    
    msgpack_pack_uint32( p, 12345u );
    msgpack_pack_raw( p, 2 );
    msgpack_pack_raw_body( p, "pi", 2 );
    msgpack_pack_double( p, 3.14159 );
    msgpack_pack_int64( p, -9876543210 );
    msgpack_pack_array( p, 3 );
    for ( i = 0; i < 3; ++i )
        msgpack_pack_int8( p, -i );
    
    n = buffer->size;
    printf( "%u\n", n );
    for ( k = 0; k < n; ++k )
        printf( "%02X", ((char*)buffer->data)[k] );
    printf( "\n" );
    
    msgpack_unpacked_init( &u );
    msgpack_unpack_next( &u, buffer->data, buffer->size, NULL );
    u.data.via.u64;
    msgpack_unpack_next( &u, buffer->data, buffer->size, NULL );
    u.data.via.raw;
    msgpack_unpack_next( &u, buffer->data, buffer->size, NULL );
    u.data.via.dec;
    msgpack_unpack_next( &u, buffer->data, buffer->size, NULL );
    u.data.via.array;
    for ( k = 0; k < n; ++k )
        u.data.via.array.ptr[k].via.i64;
    
    msgpack_unpacked_destroy(&u);
    msgpack_sbuffer_free(buffer);
    msgpack_packer_free(p);
    return 0;
}
