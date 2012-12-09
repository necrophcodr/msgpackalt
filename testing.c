#define MSGPACK_INLINE
#include "msgpackalt.h"
#include <stdio.h>
#include <float.h>

const char test1[] = { 0xe1, 0xd0, 0xe0, 0x7f, 0xd0, 0x81, 0xd1, 0xff, 0x80, 0xd1, 0x80, 0x01, 0xd2, 0xff, 0xff, 0x80, 0x00, 0xd2, 0x80, 0x00, 0x00, 0x01, 0xd3, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x7f, 0xcc, 0x80, 0xcc, 0xff, 0xcd, 0x01, 0x00, 0xcd, 0xff, 0xff, 0xce, 0x00, 0x01, 0x00, 0x00, 0xce, 0xff, 0xff, 0xff, 0xff, 0xcf, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };
const char test2[] = { 0xca, 0x00, 0x80, 0x00, 0x00, 0xca, 0x7f, 0x7f, 0xff, 0xff, 0xca, 0x00, 0x80, 0x00, 0x00, 0xca, 0x7f, 0x7f, 0xff, 0xff, 0xcb, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb, 0x7f, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
const char test3[] = { 0xa3, 0x61, 0x20, 0x62, 0xa1, 0x61, 0xa3, 0x61, 0x00, 0x62 };
const char test4[] = { 0x83, 0xa3, 0x61, 0x62, 0x63, 0x92, 0xc2, 0xa1, 0x64, 0xa3, 0x78, 0x79, 0x7a, 0xc0, 0xa3, 0x6d, 0x61, 0x70, 0x81, 0xa1, 0x3f, 0xc3 };

void printhex( byte* buffer, size_t l ) {
	size_t i;
	for ( i = 0; i < l; ++i ) printf( "%02x", buffer[i] );
	puts( "" );
}
void printhex4c( byte* buffer, size_t l ) {
	size_t i;
	printf( "{ " );
	for ( i = 0; i < l; ++i ) printf( "0x%02x, ", buffer[i] );
	puts( "}" );
}

#define CHK_TEST(p,n) { \
	size_t l = msgpack_get_len(p); \
	printhex( p->buffer, l ); \
	if ( l != sizeof( test##n ) || memcmp( p->buffer, test##n, l )) { printf( ">>> FAILED TEST %d\n\n",n ); failed = 1; }\
	else printf( ">> Passed Test %d\n\n",n ); }

int main( )
{
	msgpack_p *p1, *p2, *p3, *p4; int failed = 0;
	
	puts( "*************** MSGPACKALT TESTING ***************\n" );
	puts( "1. Integer packing test" );
	p1 = msgpack_pack_init( );
	msgpack_pack_int8( p1, -31 );			// fix
	msgpack_pack_int8( p1, -32 );			// i8
	msgpack_pack_int8( p1, 127 );			// fix
	msgpack_pack_int16( p1, -127 );			// i8
	msgpack_pack_int16( p1, -128 );			// i16
	msgpack_pack_int32( p1, -32767 ); 		// i16
	msgpack_pack_int32( p1, -32768 );		// i32
	msgpack_pack_int64( p1, -2147483647l );	// i32
	msgpack_pack_int64( p1, -2147483648ll );// i64
	msgpack_pack_uint8( p1, 127u );			// fix
	msgpack_pack_uint8( p1, 128u );			// u8
	msgpack_pack_uint16( p1, 255u );		// u8
	msgpack_pack_uint16( p1, 256u );		// u16
	msgpack_pack_uint32( p1, 65535ul ); 	// u16
	msgpack_pack_uint32( p1, 65536ul );		// u32
	msgpack_pack_uint64( p1, 4294967295ul ); // u32
	msgpack_pack_uint64( p1, 4294967296ull );// u64
	CHK_TEST( p1,1 );
	
	puts( "2. Float test" );
	p2 = msgpack_pack_init( );
	msgpack_pack_float( p2, FLT_MIN );
	msgpack_pack_float( p2, FLT_MAX );
	msgpack_pack_float( p2, FLT_MIN );
	msgpack_pack_float( p2, FLT_MAX );
	msgpack_pack_double( p2, DBL_MIN );
	msgpack_pack_double( p2, DBL_MAX );
	CHK_TEST( p2,2 );
	
	puts( "3. Raw data" );
	p3 = msgpack_pack_init( );
	msgpack_pack_str( p3, "a b" );
	msgpack_pack_str( p3, "a\0b" );
	msgpack_pack_raw( p3, "a\0b", 3 );
	CHK_TEST( p3,3 );
	
	puts( "4. Structured data" );
	p4 = msgpack_pack_init( );
	msgpack_pack_map( p4, 3 );
		msgpack_pack_str( p4, "abc" );
			msgpack_pack_array( p4, 2 );
				msgpack_pack_bool( p4, 0 );
				msgpack_pack_str( p4, "d" );
		msgpack_pack_str( p4, "xyz" );
			msgpack_pack_null( p4 );
		msgpack_pack_str( p4, "map" ); // nested
			msgpack_pack_map( p4, 1 );
				msgpack_pack_str( p4, "?" );
					msgpack_pack_bool( p4, 1 );
	CHK_TEST( p4,4 );
	
	if ( failed ) puts( "Some tests failed\n" );
	else puts( "All tests passed!!\n" );
	return 0;
}

