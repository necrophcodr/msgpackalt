/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
Unit testing code -- C library

This program performs all packing and unpacking operations checking
boundary-case data to ensure correct operation

Objects are packed into a buffer and the buffer is compared against the
correct answer (checked independently with python MessagePack) then
unpacked checking that unpacking functions and type-casting works
correctly.

Generates python script "testing.py" for debugging errors
*/
#define MSGPACK_INLINE
#include "msgpackalt.h"
#include <stdio.h>
#include <float.h>
#include <limits.h>

const char test1[] = { 0xe1, 0xd0, 0xe0, 0x7f, 0xd0, 0x81, 0xd1, 0xff, 0x80, 0xd1, 0x80, 0x01, 0xd2, 0xff, 0xff, 0x80, 0x00, 0xd2, 0x80, 0x00, 0x00, 0x01, 0xd3, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x7f,
						0xcc, 0x80, 0xcc, 0xff, 0xcd, 0x01, 0x00, 0xcd, 0xff, 0xff, 0xce, 0x00, 0x01, 0x00, 0x00, 0xce, 0xff, 0xff, 0xff, 0xff, 0xcf, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };
const char test2[] = { 0xca, 0x00, 0x80, 0x00, 0x00, 0xca, 0x7f, 0x7f, 0xff, 0xff, 0xca, 0x00, 0x80, 0x00, 0x00, 0xca, 0x7f, 0x7f, 0xff, 0xff, 0xcb, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb, 0x7f, 0xef,
						0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
const char test3[] = { 0xa3, 0x61, 0x20, 0x62, 0xa1, 0x61, 0xa3, 0x61, 0x00, 0x62 };
const char test4[] = { 0x83, 0xa3, 0x61, 0x62, 0x63, 0x92, 0xc2, 0xa1, 0x64, 0xa3, 0x78, 0x79, 0x7a, 0xc0, 0xa3, 0x6d, 0x61, 0x70, 0x81, 0xa1, 0x3f, 0xc3 };


#define PRINT_C_ARR 0	/* set this flag to print a c-array to screen to generate above constants */
void printhex( byte* buffer, size_t l ) {
	size_t i;
#if PRINT_C_ARR
	printf( "{ " );
	for ( i = 0; i < l; ++i ) printf( "0x%02x, ", buffer[i] );
	puts( "}" );
#else
	for ( i = 0; i < l; ++i ) printf( "%02x", buffer[i] );
#endif
	puts( "" );
}

void pyprint( FILE *fp, const char* name, byte* buffer, size_t l ) {
	size_t i;
	fprintf( fp, "%s = msgpack.Unpacker(StringIO('", name );
	for ( i = 0; i < l; ++i ) fprintf( fp, "\\x%02x", buffer[i] );
	fprintf( fp, "'))\n" );
	fprintf( fp, "print '%s contains ', tuple(%s)\n", name, name );
}

#define CHK_PACK(p,l,n) 			printf( ">> %s\n", ( l != sizeof( test##n ) || memcmp( p->buffer, test##n, l ))&&++nfailp ? "FAILED PACK TEST" : "Passed pack test" );
#define UNPK_CHK(u,T,f,chk)			!(( msgpack_unpack_peek( u ) == MSGPACK_##T ) && ( msgpack_unpack_##f == MSGPACK_SUCCESS ) && ( chk )) && ( puts( "Failed check " #chk " unpacking " #T ) || 1 );
#define UNPK_CHK_NUM(u,T,t,var,x)	UNPK_CHK(u, T, t( u,&var ), var==x)
#define UNPK_CHK_RAW(u,var,x)		UNPK_CHK(u,RAW,raw( u,&var,&u32 ),memcmp( var,x,strlen( x ))==0 )
#define UNPK_CHK_STR(u,var,x)		UNPK_CHK(u,RAW,str( u,var,sizeof(var) ),memcmp( var,x,sizeof(x))==0 )

int main( )
{
	int8_t i8; int16_t i16; int32_t i32; int64_t i64;
	uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
	float f32; double f64;
	char s16[16], s16rubbish[] = "123456789abcdef\0";
	const byte *pd;
	
	msgpack_p *p1, *p2, *p3, *p4;
	msgpack_u *u1, *u2, *u3, *u4;
	size_t l, nfailp = 0, nfailu = 0, n = 0;
	
	FILE *fpy = fopen( "testing.c.py","w" );
	fprintf( fpy, "import msgpack\nfrom StringIO import StringIO\n" );
	puts( "*************** MSGPACKALT TESTING ***************\n" );
	
	// *************** INTEGERS ***************
	puts( "1. Integer data" );
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
	l = msgpack_get_len( p1 );
	CHK_PACK( p1,l,1 );
	pyprint( fpy, "test1", p1->buffer, l );
	
	u1 = msgpack_unpack_init( p1->buffer, l, 0 );
	n = UNPK_CHK_NUM( u1,FIX,fix,i8,-31 );
	n += UNPK_CHK_NUM( u1,INT8,int8,i8,-32 );
	n += UNPK_CHK_NUM( u1,FIX,int16,i16,127 );		// test upcasting
	n += UNPK_CHK_NUM( u1,INT8,int16,i16,-127 );
	n += UNPK_CHK_NUM( u1,INT16,int16,i16,-128 );
	n += UNPK_CHK_NUM( u1,INT16,int16,i16,-32767 );
	n += UNPK_CHK_NUM( u1,INT32,int32,i32,-32768 );
	n += UNPK_CHK_NUM( u1,INT32,int64,i64,-2147483647l );
	n += UNPK_CHK_NUM( u1,INT64,int64,i64,-2147483648ll );
	n += UNPK_CHK_NUM( u1,FIX,uint16,u16,127u );
	n += UNPK_CHK_NUM( u1,UINT8,uint8,u8,128u );
	n += UNPK_CHK_NUM( u1,UINT8,uint16,u16,255u );
	n += UNPK_CHK_NUM( u1,UINT16,uint16,u16,256u );
	n += UNPK_CHK_NUM( u1,UINT16,uint16,u16,65535ul );
	n += UNPK_CHK_NUM( u1,UINT32,uint32,u32,65536ul );
	n += UNPK_CHK_NUM( u1,UINT32,uint32,u32,4294967295ul );
	n += UNPK_CHK_NUM( u1,UINT64,uint64,u64,4294967296ull );
	printf( ">> %s\n", n ? "FAILED UNPACK TESTS" : "Passed unpack tests" );
	nfailu += n;
	
	puts( "" );
	printhex( p1->buffer, l );
	puts( "" );
	msgpack_pack_free( p1 );
	msgpack_unpack_free( u1 );
	
	
	// *************** FLOATING POINT ***************
	puts( "2. Float data" );
	p2 = msgpack_pack_init( );
	msgpack_pack_float( p2, FLT_MIN );
	msgpack_pack_float( p2, FLT_MAX );
	msgpack_pack_float( p2, FLT_MIN );
	msgpack_pack_float( p2, FLT_MAX );
	msgpack_pack_double( p2, DBL_MIN );
	msgpack_pack_double( p2, DBL_MAX );
	l = msgpack_get_len( p2 );
	CHK_PACK( p2,l,2 );
	pyprint( fpy, "test2", p1->buffer, l );
	
	u2 = msgpack_unpack_init( p2->buffer, l, 0 );			// consider epsilon checks here
	n = UNPK_CHK_NUM( u2,FLOAT,float,f32,FLT_MIN );
	n += UNPK_CHK_NUM( u2,FLOAT,float,f32,FLT_MAX );
	n += UNPK_CHK_NUM( u2,FLOAT,double,f64,FLT_MIN );	// upcasting
	n += UNPK_CHK_NUM( u2,FLOAT,double,f64,FLT_MAX );	// upcasting
	n += UNPK_CHK_NUM( u2,DOUBLE,double,f64,DBL_MIN );
	n += UNPK_CHK_NUM( u2,DOUBLE,double,f64,DBL_MAX );
	printf( ">> %s\n", n ? "FAILED UNPACK TESTS" : "Passed unpack tests" );
	nfailu += n;
	
	puts( "" );
	printhex( p2->buffer, l );
	puts( "" );
	msgpack_pack_free( p2 );
	msgpack_unpack_free( u2 );
	
	
	// *************** STRINGS ***************
	puts( "3. Raw data" );
	p3 = msgpack_pack_init( );
	msgpack_pack_str( p3, "a b" );
	msgpack_pack_str( p3, "a\0b" );		// truncates
	msgpack_pack_raw( p3, "a\0b", 3 );	// does not truncate
	l = msgpack_get_len( p3 );
	CHK_PACK( p3,l,3 );
	pyprint( fpy, "test3", p1->buffer, l );
	
	u3 = msgpack_unpack_init( p3->buffer, l, 0 );
	// straight string
	strncpy( s16, s16rubbish, 16 );
	n += UNPK_CHK_STR( u3,s16,"a b" );
	// truncated string
	strncpy( s16, s16rubbish, 16 );
	n += UNPK_CHK_STR( u3,s16,"a" );
	// raw memory
	strncpy( s16, s16rubbish, 16 );
	n += UNPK_CHK_RAW( u3,pd,"a\0b" );
	printf( ">> %s\n", n ? "FAILED UNPACK TESTS" : "Passed unpack tests" );
	nfailu += n;
	
	puts( "" );
	printhex( p3->buffer, l );
	puts( "" );
	msgpack_pack_free( p3 );
	msgpack_unpack_free( u3 );
	
	
	// *************** MAPS ***************
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
	l = msgpack_get_len( p4 );
	CHK_PACK( p4,l,4 );
	pyprint( fpy, "test4", p1->buffer, l );
	
	u4 = msgpack_unpack_init( p4->buffer, l, 0 );
	n = UNPK_CHK( u4, MAP, map( u4,&u32 ), u32==3 );
	n += UNPK_CHK_STR( u4,s16,"abc" );
	n += UNPK_CHK( u4,ARRAY,array( u4,&u32 ), u32==2 );
	n += UNPK_CHK( u4,BOOL,bool(u4)==0 && MSGPACK_SUCCESS,1 );
	n += UNPK_CHK_STR( u4,s16,"d" );
	n += UNPK_CHK_STR( u4,s16,"xyz" );
	n += UNPK_CHK( u4,NULL,null(u4),1 );
	n += UNPK_CHK_STR( u4,s16,"map" );
	n += UNPK_CHK( u4, MAP, map( u4,&u32 ), u32==1 );
	n += UNPK_CHK_STR( u4,s16,"?" );
	n += UNPK_CHK( u4,BOOL,bool(u4)==1 && MSGPACK_SUCCESS,1 );
	printf( ">> %s\n", n ? "FAILED UNPACK TESTS" : "Passed unpack tests" );
	nfailu += n;
	
	puts( "" );
	printhex( p4->buffer, l );
	puts( "" );
	msgpack_pack_free( p4 );
	msgpack_unpack_free( u4 );
	
	
	printf( "Failed %d packing tests, %d unpacking tests\n", nfailp, nfailu );
	fclose( fpy );
	return 0;
}

