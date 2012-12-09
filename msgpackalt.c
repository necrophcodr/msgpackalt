/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
msgpackalt.c : function implementations
----------------------------------------------------------------------
*/
#include "msgpackalt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __LITTLE_ENDIAN__           /* have to swap for network-endian */
	#ifdef _MSC_VER
		/* shuttup msvc! */
		#pragma warning( disable: 4127 )	/* "conditional expression is constant" */
		/* MSVC builtins, http://msdn.microsoft.com/en-us/library/a3140177.aspx */
		#define BYTESWAP16   _byteswap_ushort
		#define BYTESWAP32   _byteswap_ulong
		#define BYTESWAP64   _byteswap_uint64
	#elif ( __GNUC__*100+__GNUC_MINOR__ >= 403 )
		/* GCC v4.3+ builtins, http://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
				http://old.nabble.com/-Bug-target-52624--New%3A-Missing-__builtin_bswap16-td33533370.html
			*/
		static inline unsigned short __builtin_bswap16( unsigned short a ) { return (a<<8)|(a>>8); } 
		#define BYTESWAP16   __builtin_bswap16
		#define BYTESWAP32   __builtin_bswap32
		#define BYTESWAP64   __builtin_bswap64
	#else
		/* attempt generic functions */
		#include <byteswap.h>
		#define BYTESWAP16   bswap_16
		#define BYTESWAP32   bswap_32
		#define BYTESWAP64   bswap_64
	#endif
	#define		BYTESWAP8
#elif __BIG_ENDIAN__ /* already network-endian */
	#define		BYTESWAP8
	#define		BYTESWAP16
	#define		BYTESWAP32
	#define		BYTESWAP64
#else
	#error Unsupported endian-ness
#endif


#define PTR_CHK(m)	if ( !m || !m->p ) return MSGPACK_ARGERR;

/* **************************************** MEMORY FUNCTIONS **************************************** */

MSGPACKF msgpack_p* msgpack_pack_init( )
{
	msgpack_p *m = ( msgpack_p* )malloc( sizeof( msgpack_p ));
	m->max = 256;
	m->p = m->buffer = ( byte* )malloc( m->max );
	return m->p ? m : NULL;
}

INLINE MSGPACK_ERR msgpack_expand( msgpack_p *m, uint32_t num )
{
	PTR_CHK( m );
	if ( m->p + num > m->buffer + m->max )	/* too much for allocated buffer? */
	{
		byte *p;							/* pointer for new buffer */
		uint32_t l = m->p - m->buffer;		/* current buffer length */
		uint32_t m2 = 2*m->max;				/* guess at next length */
		if ( l + num > m2 ) m2 = l + num;	/* is it enough? otherwise expand to fit */
		p = ( byte* )malloc( m2 );			/* attempt to allocate new space */
		if ( !p ) return MSGPACK_MEMERR;	/* failed, but buffer still intact */
		memcpy( p, m->buffer, l );			/* copy the previous buffer across */
		free( m->buffer );					/* free the old buffer */
		m->buffer = p;						/* updated stored values */
		m->p = p + l;
		m->max = m2;
	}
	return MSGPACK_SUCCESS;
}

MSGPACKF MSGPACK_ERR msgpack_pack_append( msgpack_p *m, const void* data, uint32_t n )
{
	MSGPACK_ERR ret = msgpack_expand( m, n );
	if ( ret ) return ret;
	if ( data )
		memcpy( m->p, data, n );
	else
		memset( m->p, 0, n );
	m->p += n;
	return MSGPACK_SUCCESS;
}

MSGPACKF MSGPACK_ERR msgpack_pack_free( msgpack_p *m )
{
	PTR_CHK( m );
	if ( m->buffer ) free( m->buffer ); //m->p = NULL; m->buffer = NULL; }
	memset( m, 0, sizeof( msgpack_p ));	// for sanity
	free( m );
	return MSGPACK_SUCCESS;
}

MSGPACKF uint32_t msgpack_get_len( const msgpack_p *m )
{
	if ( !m || !m->p ) return 0;
	return m->p - m->buffer;
}

MSGPACKF MSGPACK_ERR msgpack_get_buffer( msgpack_p *m, const byte ** data, uint32_t *n )
{
	PTR_CHK( m );
	*data = m->buffer;
	*n = m->p - m->buffer;
	return MSGPACK_SUCCESS;
}

MSGPACKF uint32_t msgpack_copy_to( const msgpack_p *m, void *data, uint32_t max )
{
	uint32_t l;
	if ( !m || !m->p || !data || !max ) return 0;
	l = m->p - m->buffer;
	if ( l > max ) return 0;
	memcpy( data, m->buffer, l );
	return l;
}

INLINE MSGPACK_ERR msgpack_copy_bits( const void *src, void* dest, byte n )
{
	if ( src && dest )
		switch ( n ) {
			case 0:     break;
			case 1:     *( uint8_t* )dest = BYTESWAP8( *( uint8_t* )src ); break;
			case 2:     *( uint16_t* )dest = BYTESWAP16( *( uint16_t* )src ); break;
			case 4:     *( uint32_t* )dest = BYTESWAP32( *( uint32_t* )src ); break;
			case 8:     *( uint64_t* )dest = BYTESWAP64( *( uint64_t* )src ); break;
			default:    return MSGPACK_ARGERR;
		}
	return MSGPACK_SUCCESS;
}


/* **************************************** PACKING FUNCTIONS **************************************** */
INLINE MSGPACK_ERR msgpack_pack_internal( msgpack_p *m, byte code, const void* p, byte n )
{
	if ( !m || !m->p ) return MSGPACK_ARGERR;
	if ( msgpack_expand( m, n + 1 )) return MSGPACK_MEMERR;
	*m->p = code; ++m->p;
	if ( msgpack_copy_bits( p, m->p, n )) return MSGPACK_ARGERR; else m->p += n;
	return MSGPACK_SUCCESS;
}


#define fix_t int8_t
#define DEFINE_INT_PACK( T, MT, chk, P ) \
	MSGPACKF MSGPACK_ERR msgpack_pack_##T( msgpack_p *m, T##_t x ) \
		{ if ( chk ) return msgpack_pack_##P( m, ( P##_t )x ); return msgpack_pack_internal( m, MSGPACK_##MT, &x, sizeof( x )); }
DEFINE_INT_PACK( uint8,  UINT8,  x<128, fix )
DEFINE_INT_PACK( uint16, UINT16, x<(1u<<8), uint8 )
DEFINE_INT_PACK( uint32, UINT32, x<(1ul<<16), uint16 )
DEFINE_INT_PACK( uint64, UINT64, x<(1ull<<32), uint32 )

DEFINE_INT_PACK( int8,  INT8,  x>-32,  fix )
DEFINE_INT_PACK( int16, INT16, (x>-1<<7)&&(x<1<<7), int8 )
DEFINE_INT_PACK( int32, INT32, (x>-1l<<15)&&(x<1l<<15), int16 )
DEFINE_INT_PACK( int64, INT64, (x>-1ll<<31)&&(x<1ll<<31), int32 )
#undef INT_PACK
#undef fix_t

MSGPACKF MSGPACK_ERR msgpack_pack_float( msgpack_p *m, float x )      { return msgpack_pack_internal( m, MSGPACK_FLOAT, &x, 4 ); }
MSGPACKF MSGPACK_ERR msgpack_pack_double( msgpack_p *m, double x )    { return msgpack_pack_internal( m, MSGPACK_DOUBLE, &x, 8 ); }

MSGPACKF MSGPACK_ERR msgpack_pack_null( msgpack_p* m )                { return msgpack_pack_internal( m, MSGPACK_NULL, NULL, 0 ); }
MSGPACKF MSGPACK_ERR msgpack_pack_bool( msgpack_p* m, bool x )        { return msgpack_pack_internal( m, x?MSGPACK_TRUE:MSGPACK_FALSE, NULL, 0 ); }
MSGPACKF MSGPACK_ERR msgpack_pack_fix( msgpack_p* m, int8_t x )       { return ( x>-32 )?msgpack_pack_internal( m,( x<0 )?( x|0xe0 ):x, NULL, 0 ):MSGPACK_TYPEERR; }

INLINE MSGPACK_ERR msgpack_pack_arr_head( msgpack_p *m, byte c1, byte c2, uint32_t n )
{
	if ( n < ( 1u<<( c1 >= 0xa0 ? 5 : 4 )))
		return msgpack_pack_internal( m, c1|( byte )n, NULL, 0 );
	else if ( n < ( 1u<<16 ))
		return msgpack_pack_internal( m, c2, &n, 2 );
	else
		return msgpack_pack_internal( m, c2+1, &n, 4 );
}
MSGPACKF MSGPACK_ERR msgpack_pack_raw( msgpack_p* m, const byte *data, uint32_t n )
{
	if ( msgpack_pack_arr_head( m, 0xa0, MSGPACK_RAW, n )) return MSGPACK_TYPEERR;
	if ( msgpack_expand( m, n )) return MSGPACK_MEMERR;
	memcpy( m->p, data, n );
	m->p += n;
	return MSGPACK_SUCCESS;
}
MSGPACKF MSGPACK_ERR msgpack_pack_str( msgpack_p* m, const char *str )
{
	if ( !str ) return MSGPACK_ARGERR;
	return msgpack_pack_raw( m, ( byte* )str, strlen( str ));
}
MSGPACKF MSGPACK_ERR msgpack_pack_array( msgpack_p* m, uint32_t n )
{
	return msgpack_pack_arr_head( m, 0x90, MSGPACK_ARRAY, n );
}
MSGPACKF MSGPACK_ERR msgpack_pack_map( msgpack_p* m, uint32_t n )
{
	return msgpack_pack_arr_head( m, 0x80, MSGPACK_MAP, n );
}

MSGPACKF MSGPACK_ERR msgpack_prepend_header( msgpack_p *m )
{
	const uint32_t l = msgpack_get_len( m );
	/* smallest pack size */
	byte n = 5;
	if ( l + 1 < 128 )          n = 1;
	else if ( l + 3 < 65536 )   n = 3;
	if ( l == 0 ) return MSGPACK_MEMERR;
	/* expand buffer */
	if ( msgpack_expand( m,n )) return MSGPACK_MEMERR;
	/* shift buffer for prepend */
	memmove( m->buffer + n, m->buffer, l ); /* overlap is ok */
	/* pack length at front */
	m->p = m->buffer;
	if ( n == 1 )       msgpack_pack_fix( m, ( int8_t )( l+n ));
	else if ( n == 3 )  msgpack_pack_uint16( m, ( uint16_t )( l+n ));
	else                msgpack_pack_uint32( m, ( uint32_t )( l+n ));
	/* reset pointer */
	m->p += l;
	return MSGPACK_SUCCESS;
}

MSGPACKF int msgpack_check_header( msgpack_u *m )
{
	int t = msgpack_unpack_peek( m );
	if ( t == MSGPACK_FIX ) {
		int8_t x = 0;
		msgpack_unpack_fix( m, &x );
		return ( uint32_t )x == m->max;
	} else if ( t == MSGPACK_UINT16 ) {
		uint16_t x = 0;
		msgpack_unpack_uint16( m, &x );
		return ( uint32_t )x == m->max;
	} else if ( t == MSGPACK_UINT32 ) {
		uint32_t x = 0;
		msgpack_unpack_uint32( m, &x );
		return x == m->max;
	} else if ( t < 0 )
		return t;
	return MSGPACK_TYPEERR;
}

/* **************************************** UNPACKING FUNCTIONS **************************************** */
MSGPACKF msgpack_u* msgpack_unpack_init( const void* data, uint32_t n, const int flags )
{
	msgpack_u *m = ( msgpack_u* )malloc( sizeof( msgpack_u ));
	if ( flags || !data ) {
		if ( n < 16 ) n = 16;
		m->p = ( byte* )malloc( n );			/* allocate a block of memory */
		if ( data ) memcpy(( byte* )m->p, data, n );	/* a non-const operation, but that's fine since it's our memory */
		m->flags = 1;							/* indicate the memory should be free'd */
	} else {
		m->p = ( byte* )data;	/* use the pointer directly */
		m->flags = 0;			/* DON'T free it */
	}
	m->end = m->p + n;
	m->max = n;
	return m;
}

MSGPACKF MSGPACK_ERR msgpack_unpack_free( msgpack_u *m )
{
	if ( m )
	{
		/* is there an associated buffer, and do we need to free it? */
		if ( m->p && ( m->flags & 1 )) free(( void* )( m->end - m->max ));
		memset( m, 0, sizeof( msgpack_u ));	// for sanity
		/* free the struct itself */
		free( m );
	}
	return MSGPACK_SUCCESS;
}

MSGPACKF int msgpack_unpack_skip( msgpack_u *m )
{
	uint32_t i, n, r;
	int code = msgpack_unpack_peek( m );
	const byte *ptr = m->p;
	if ( code < 0 ) return code;
	switch ( code ) {
		case MSGPACK_FIX:
		case MSGPACK_NULL:
		case MSGPACK_FALSE:
		case MSGPACK_TRUE:
			++m->p;
			break;
		case MSGPACK_UINT8:
		case MSGPACK_INT8:
			m->p += 2;
			break;
		case MSGPACK_UINT16:
		case MSGPACK_INT16:
			m->p += 3;
			break;
		case MSGPACK_FLOAT:
		case MSGPACK_UINT32:
		case MSGPACK_INT32:
			m->p += 5;
			break;
		case MSGPACK_DOUBLE:
		case MSGPACK_UINT64:
		case MSGPACK_INT64:
			m->p += 9;
			break;
		case MSGPACK_RAW:
			r = msgpack_unpack_raw( m, NULL, &n );
			if ( r < 0 ) return -1;
			break;
		case MSGPACK_ARRAY:
			r = msgpack_unpack_array( m, &n );
			if ( r < 0 ) return -1;
			for ( i = n; i > 0; --i )
			{
				r = msgpack_unpack_skip( m );
				if ( r < 0 ) return -1;
			}
			break;
		case MSGPACK_MAP:
			r = msgpack_unpack_map( m, &n );
			if ( r < 0 ) return -1;
			for ( i = 2*n; i > 0; --i )
			{
				r = msgpack_unpack_skip( m );
				if ( r < 0 ) return -1;
			}
			break;
		default:
			return MSGPACK_TYPEERR;
	}
	return m->p - ptr;
}

MSGPACKF MSGPACK_ERR msgpack_unpack_append( msgpack_u *m, const void* data, const uint32_t n )
{
	byte *buffer;
	uint32_t n0;
	if ( !m || !m->p || !data || !n ) return MSGPACK_ARGERR;
	/* allocate a new buffer to contain appended message */
	n0 = m->end - m->p;
	/* create new buffer */
	buffer = ( byte* )malloc( n0 + n );
	if ( !buffer ) return MSGPACK_MEMERR;
	/* copy the old buffer into the new one */
	memcpy( buffer, m->p, n0 );
	/* deallocate the old buffer if necesary */
	if ( m->flags & 1 ) free(( void* )( m->end - m->max ));
	/* copy the new segment into the new buffer */
	memcpy( buffer + n0, data, n );
	/* update the pointers */
	m->p = buffer;
	m->max = n0 + n;
	m->end = buffer + m->max;
	/* indicate the buffer needs to be free'd */
	m->flags |= 1;
	return MSGPACK_SUCCESS;
}

MSGPACKF int msgpack_unpack_peek( const msgpack_u *m )
{
	byte b;
	if ( !m || ( m->p >= m->end )) return MSGPACK_MEMERR;
	b = *m->p;
	/* check the FIXNUM codes */
	if (( b >> 7 == 0 )||( b >> 5 == 7 )) return MSGPACK_FIX;
	if (( b >> 5 == 5 )||( b == MSGPACK_RAW )||( b == MSGPACK_RAW+1 )) return MSGPACK_RAW;
	if (( b >> 4 == 8 )||( b == MSGPACK_MAP )||( b == MSGPACK_MAP+1 )) return MSGPACK_MAP;
	if (( b >> 4 == 9 )||( b == MSGPACK_ARRAY )||( b == MSGPACK_ARRAY+1 )) return MSGPACK_ARRAY;
	/* must be one of the enumeration */
	return b;
}

#define UNPACK_CHK(m) if (( !m ) || ( m->p >= m->end )) return MSGPACK_MEMERR;

MSGPACKF uint32_t msgpack_unpack_len( msgpack_u *m )
{
	if ( !m || !m->p || ( m->end < m->p )) return 0;
	return m->end - m->p;
}

MSGPACKF MSGPACK_ERR msgpack_unpack_null( msgpack_u *m )
{
	UNPACK_CHK( m );
	if ( *m->p == MSGPACK_NULL ) { ++m->p; return MSGPACK_SUCCESS; }
	return MSGPACK_TYPEERR;
}

MSGPACKF int msgpack_unpack_bool( msgpack_u *m )
{
	UNPACK_CHK( m );
	switch ( *m->p ) {
		case MSGPACK_TRUE:  ++m->p; return 1;
		case MSGPACK_FALSE: ++m->p; return 0;
		default:            return MSGPACK_TYPEERR;
	}
}

uint8_t  msgpack_get_UINT8( msgpack_u *m )  	{ uint8_t x = *( uint8_t* )++m->p; ++m->p; return x; }
uint16_t msgpack_get_UINT16( msgpack_u *m ) 	{ uint16_t x = BYTESWAP16( *( uint16_t* )++m->p ); m->p += 2; return x; }
uint32_t msgpack_get_UINT32( msgpack_u *m ) 	{ uint32_t x = BYTESWAP32( *( uint32_t* )++m->p ); m->p += 4; return x; }
uint64_t msgpack_get_UINT64( msgpack_u *m ) 	{ uint64_t x = BYTESWAP64( *( uint64_t* )++m->p ); m->p += 8; return x; }
int8_t   msgpack_get_INT8( msgpack_u *m )	 	{ return msgpack_get_UINT8( m ); }
int16_t  msgpack_get_INT16( msgpack_u *m )  	{ return msgpack_get_UINT16( m ); }
int32_t  msgpack_get_INT32( msgpack_u *m )  	{ return msgpack_get_UINT32( m ); }
int64_t  msgpack_get_INT64( msgpack_u *m )  	{ return msgpack_get_UINT64( m ); }
int8_t   msgpack_get_FIX( msgpack_u *m ) 		{ return ( *m->p > 128 ) ? *( int8_t* )m->p++ : *m->p++; }

#define FIX_INT 1
#define FIX_UINT 0

#define DEFINE_INT_UNPACK( T, S ) \
	MSGPACKF MSGPACK_ERR msgpack_unpack_##T( msgpack_u *m, T##_t *x ) { \
		const int t = msgpack_unpack_peek( m ); \
		if      (( t == MSGPACK_##S##64 ) && ( sizeof( T##_t ) >= 8 )) *x = ( T##_t )msgpack_get_##S##64( m ); \
		else if (( t == MSGPACK_##S##32 ) && ( sizeof( T##_t ) >= 4 )) *x = ( T##_t )msgpack_get_##S##32( m ); \
		else if (( t == MSGPACK_##S##16 ) && ( sizeof( T##_t ) >= 2 )) *x = ( T##_t )msgpack_get_##S##16( m ); \
		else if ( t == MSGPACK_##S##8 ) *x = msgpack_get_##S##8( m ); \
		else if (( t == MSGPACK_FIX ) && ( FIX_##S || *m->p >> 7 == 0 ))  *x = msgpack_get_FIX( m );\
		else return MSGPACK_TYPEERR; \
		return MSGPACK_SUCCESS; \
	}
DEFINE_INT_UNPACK( int64, INT )
DEFINE_INT_UNPACK( int32, INT )
DEFINE_INT_UNPACK( int16, INT )
DEFINE_INT_UNPACK( int8, INT )
DEFINE_INT_UNPACK( uint64, UINT )
DEFINE_INT_UNPACK( uint32, UINT )
DEFINE_INT_UNPACK( uint16, UINT )
DEFINE_INT_UNPACK( uint8, UINT )

MSGPACKF MSGPACK_ERR msgpack_unpack_fix( msgpack_u *m, int8_t *x )
{
	if ( msgpack_unpack_peek( m ) != MSGPACK_FIX ) return MSGPACK_TYPEERR;
	*x = msgpack_get_FIX( m );
	return MSGPACK_SUCCESS;
}
#undef DEFINE_INT_UNPACK
#undef FIX_UINT
#undef FIX_INT

MSGPACKF MSGPACK_ERR msgpack_unpack_float( msgpack_u *m, float *x )
{
	UNPACK_CHK( m );
	if ( *m->p != MSGPACK_FLOAT )  return MSGPACK_TYPEERR;
	*( uint32_t* )x = BYTESWAP32( *( uint32_t* )++m->p ); m->p += sizeof( float );
	return MSGPACK_SUCCESS;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_double( msgpack_u *m, double *x )
{
	UNPACK_CHK( m );
	if ( *m->p == MSGPACK_DOUBLE ) {
		*( uint64_t* )x = BYTESWAP64( *( uint64_t* )++m->p );
		m->p += sizeof( double );
		return MSGPACK_SUCCESS;
	} else if ( *m->p == MSGPACK_FLOAT ) {
		float y; msgpack_unpack_float( m, &y ); *x = y;
		return MSGPACK_SUCCESS;
	}
	return MSGPACK_TYPEERR;
}

INLINE MSGPACK_ERR msgpack_unpack_arr_head( msgpack_u *m, byte c1, byte nb, byte c2, uint32_t *n )
{
	byte b;
	UNPACK_CHK( m );
	b = *m->p; ++m->p; *n = 0;
	if (( b>>nb )==( c1>>nb ))  { *n = b & ~c1; }
	else if ( b == c2 )         { msgpack_copy_bits( m->p, n, 2 ); m->p += 2; }
	else if ( b == c2+1 )       { msgpack_copy_bits( m->p, n, 4 ); m->p += 4; }
	else                        { --m->p; return MSGPACK_TYPEERR; }
	return MSGPACK_SUCCESS;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_raw( msgpack_u* m, const byte **data, uint32_t *nout )
{
	uint32_t n;
	UNPACK_CHK( m );
	if ( msgpack_unpack_arr_head( m, 0xa0, 5, MSGPACK_RAW, &n )) return MSGPACK_TYPEERR;
	if ( data ) *data = m->p;
	if ( nout ) *nout = n;
	m->p += n;
	return MSGPACK_SUCCESS;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_str( msgpack_u* m, char *dest, uint32_t max )
{
	const byte *ptr; uint32_t n;
	UNPACK_CHK( m );
	if ( msgpack_unpack_raw( m, &ptr, &n )) return MSGPACK_TYPEERR;
	if ( n >= max ) return MSGPACK_MEMERR;
	memcpy( dest, ptr, n );
	dest[n] = 0;
	return MSGPACK_SUCCESS;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_array( msgpack_u* m, uint32_t *n )
{
	UNPACK_CHK( m );
	return msgpack_unpack_arr_head( m, 0x90, 4, MSGPACK_ARRAY, n );
}
MSGPACKF MSGPACK_ERR msgpack_unpack_map( msgpack_u* m, uint32_t *n )
{
	UNPACK_CHK( m );
	return msgpack_unpack_arr_head( m, 0x80, 4, MSGPACK_MAP, n );
}

#undef UNPACK_CHK
#undef PTR_CHK
