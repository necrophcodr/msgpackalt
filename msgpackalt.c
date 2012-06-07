/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
msgpackalt.c : function implementations
----------------------------------------------------------------------
*/
#include "msgpackalt.h"

#include <stdlib.h>
#include <string.h>

/* **************************************** MEMORY FUNCTIONS **************************************** */

MSGPACKF msgpack_p* msgpack_pack_init( )
{
    msgpack_p *m = ( msgpack_p* )malloc( sizeof( msgpack_p ));
    m->max = 256;
    m->p = m->buffer = ( byte* )malloc( m->max );
    return m->p ? m : NULL;
}

static MSGPACK_ERR msgpack_expand( msgpack_p *m, uint32_t num )
{
    if ( m->p + num < m->buffer + m->max )
        return MSGPACK_SUCCESS;
    else
    {
        uint32_t l = m->p - m->buffer;
        byte *p = ( byte* )malloc( 2*m->max );
        if ( !p ) return MSGPACK_MEMERR;
        memcpy( p, m->buffer, l );
        free( m->buffer );
        m->buffer = p;
        m->p = p + l;
        m->max *= 2;
    }
    return MSGPACK_SUCCESS;
}

MSGPACKF MSGPACK_ERR msgpack_pack_free( msgpack_p *m )
{
    if ( m->buffer ) { free( m->buffer ); m->p = NULL; m->buffer = NULL; }
    free( m );
    return MSGPACK_SUCCESS;
}

MSGPACKF uint32_t msgpack_get_len( const msgpack_p *m )
{
    return m->p - m->buffer;
}

MSGPACKF MSGPACK_ERR msgpack_get_buffer( msgpack_p *m, const byte ** data, uint32_t *n )
{
    *data = m->buffer;
    *n = m->p - m->buffer;
    return MSGPACK_SUCCESS;
}

MSGPACKF uint32_t msgpack_copy_to( const msgpack_p *m, byte *data, uint32_t max )
{
    uint32_t l = m->p - m->buffer;
    if ( l > max ) return 0;
    memcpy( data, m->buffer, l );
    return l;
}

static INLINE MSGPACK_ERR msgpack_copy_bits( const void *src, void* dest, byte n )
{
    if ( src && dest )
        switch ( n ) {
            case 0:     break;
            case 1:     *( byte* )dest = *( byte* )src; break;
            case 2:     *( uint16_t* )dest = BYTESWAP16( *( uint16_t* )src ); break;
            case 4:     *( uint32_t* )dest = BYTESWAP32( *( uint32_t* )src ); break;
            case 8:     *( uint64_t* )dest = BYTESWAP64( *( uint64_t* )src ); break;
            default:    return MSGPACK_ARGERR;
        }
    return MSGPACK_SUCCESS;
}


/* **************************************** PACKING FUNCTIONS **************************************** */
static INLINE MSGPACK_ERR msgpack_pack_internal( msgpack_p *m, byte code, const void* p, byte n )
{
    if ( msgpack_expand( m, n + 1 )) return MSGPACK_MEMERR;
    *m->p = code; ++m->p;
    if ( msgpack_copy_bits( p, m->p, n )) return MSGPACK_ARGERR; else m->p += n;
    return MSGPACK_SUCCESS;
}


#define fix_t int8_t
#define DEFINE_INT_PACK( T, MT, chk, P ) \
    MSGPACKF MSGPACK_ERR msgpack_pack_##T( msgpack_p *m, T##_t x ) \
        { return msgpack_pack_internal( m, MSGPACK_##MT, &x, sizeof( x )); } //if ( chk ) return msgpack_pack_##P( m, ( P##_t )x ); 
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

static INLINE MSGPACK_ERR msgpack_pack_arr_head( msgpack_p *m, byte c1, byte c2, uint32_t n )
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
    MSGPACK_TYPE_CODES t = msgpack_unpack_peek( m );
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
    }
    return MSGPACK_TYPEERR;
}

/* **************************************** UNPACKING FUNCTIONS **************************************** */
MSGPACKF msgpack_u* msgpack_unpack_init( const byte* data, const uint32_t n )
{
    msgpack_u *m = ( msgpack_u* )malloc( sizeof( msgpack_u ));
    m->p = data;
    m->end = data + n;
    m->max = n;
    return m;
}

MSGPACKF void msgpack_unpack_free( msgpack_u *m )
{
    free( m );
}

MSGPACKF MSGPACK_TYPE_CODES msgpack_unpack_peek( const msgpack_u *m )
{
    const byte b = *m->p;
    // *** check the FIXNUMS ***
    if (( b >> 7 == 0 )||( b >> 5 == 7 )) return MSGPACK_FIX;
    if (( b >> 5 == 5 )||( b == 0xda )||( b == 0xdb )) return MSGPACK_RAW;
    if (( b >> 4 == 8 )||( b == 0xdc )||( b == 0xdd )) return MSGPACK_MAP;
    if (( b >> 4 == 9 )||( b == 0xde )||( b == 0xdf )) return MSGPACK_ARRAY;
    // must be one of the enumeration
    return ( MSGPACK_TYPE_CODES )b;
}

MSGPACKF uint32_t msgpack_unpack_len( msgpack_u *m )
{
    return m->end > m->p ? m->end - m->p : 0;
}

MSGPACKF MSGPACK_ERR msgpack_unpack_null( msgpack_u *m )
{
    return *m->p++ == MSGPACK_NULL ? MSGPACK_SUCCESS : MSGPACK_TYPEERR;
}

MSGPACKF int msgpack_unpack_bool( msgpack_u *m )
{
    switch ( *m->p ) {
        case MSGPACK_TRUE:  return 1;
        case MSGPACK_FALSE: return 0;
        default:            return MSGPACK_TYPEERR;
    }
}

#define DO_UNPACK( m, type, x, sw ) { *x = sw( *(type*)++m->p ); m->p += sizeof(type); return MSGPACK_SUCCESS; }
#define DO_UNPACK_FIX( m, x )       { *x = ( *m->p > 128 ) ? ( int8_t )*m->p : *m->p; ++m->p; return MSGPACK_SUCCESS; }
#define DO_UNPACK_UFIX( m, x )      if ( *m->p >> 7 == 0 ) { *x = *m->p; ++m->p; return MSGPACK_SUCCESS; }

MSGPACKF MSGPACK_ERR msgpack_unpack_fix( msgpack_u *m, int8_t *x )  {
    if (  msgpack_unpack_peek( m ) != MSGPACK_FIX ) return MSGPACK_TYPEERR;
    DO_UNPACK_FIX( m,x );
}

MSGPACKF MSGPACK_ERR msgpack_unpack_int64( msgpack_u *m, int64_t *x )
{
    switch ( msgpack_unpack_peek( m )) {
        case MSGPACK_INT64:     DO_UNPACK( m, int64_t, x, BYTESWAP64 );
        case MSGPACK_INT32:     DO_UNPACK( m, int32_t, x, BYTESWAP32 );
        case MSGPACK_INT16:     DO_UNPACK( m, int16_t, x, BYTESWAP16 );
        case MSGPACK_INT8:      DO_UNPACK( m, int8_t, x, );
        case MSGPACK_FIX:       DO_UNPACK_FIX( m,x );
        default: break;
    }
    return MSGPACK_TYPEERR;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_int32( msgpack_u *m, int32_t *x )
{
    switch ( msgpack_unpack_peek( m )) {
        case MSGPACK_INT32:     DO_UNPACK( m, int32_t, x, BYTESWAP32 );
        case MSGPACK_INT16:     DO_UNPACK( m, int16_t, x, BYTESWAP16 );
        case MSGPACK_INT8:      DO_UNPACK( m, int8_t, x, );
        case MSGPACK_FIX:       DO_UNPACK_FIX( m,x );
        default: break;
    }
    return MSGPACK_TYPEERR;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_int16( msgpack_u *m, int16_t *x )
{
    switch ( msgpack_unpack_peek( m )) {
        case MSGPACK_INT16:     DO_UNPACK( m, int16_t, x, BYTESWAP16 );
        case MSGPACK_INT8:      DO_UNPACK( m, int8_t, x, );
        case MSGPACK_FIX:       DO_UNPACK_FIX( m,x );
        default: break;
    }
    return MSGPACK_TYPEERR;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_int8( msgpack_u *m, int8_t *x )
{
    switch ( msgpack_unpack_peek( m )) {
        case MSGPACK_INT8:      DO_UNPACK( m, int8_t, x, );
        case MSGPACK_FIX:       DO_UNPACK_FIX( m,x );
        default: break;
    }
    return MSGPACK_TYPEERR;
}

MSGPACKF MSGPACK_ERR msgpack_unpack_uint64( msgpack_u *m, uint64_t *x )
{
    switch ( msgpack_unpack_peek( m )) {
        case MSGPACK_UINT64:    DO_UNPACK( m, uint64_t, x, BYTESWAP64 );
        case MSGPACK_UINT32:    DO_UNPACK( m, uint32_t, x, BYTESWAP32 );
        case MSGPACK_UINT16:    DO_UNPACK( m, uint16_t, x, BYTESWAP16 );
        case MSGPACK_UINT8:     DO_UNPACK( m, uint8_t, x, );
        case MSGPACK_FIX:       DO_UNPACK_UFIX( m,x );
        default: break;
    }
    return MSGPACK_TYPEERR;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_uint32( msgpack_u *m, uint32_t *x )
{
    switch ( msgpack_unpack_peek( m )) {
        case MSGPACK_UINT32:    DO_UNPACK( m, uint32_t, x, BYTESWAP32 );
        case MSGPACK_UINT16:    DO_UNPACK( m, uint16_t, x, BYTESWAP16 );
        case MSGPACK_UINT8:     DO_UNPACK( m, uint8_t, x, );
        case MSGPACK_FIX:       DO_UNPACK_UFIX( m,x );
        default: break;
    }
    return MSGPACK_TYPEERR;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_uint16( msgpack_u *m, uint16_t *x )
{
    switch ( msgpack_unpack_peek( m )) {
        case MSGPACK_UINT16:    DO_UNPACK( m, uint16_t, x, BYTESWAP16 );
        case MSGPACK_UINT8:     DO_UNPACK( m, uint8_t, x, );
        case MSGPACK_FIX:       DO_UNPACK_UFIX( m,x );
        default: break;
    }
    return MSGPACK_TYPEERR;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_uint8( msgpack_u *m, uint8_t *x )
{
    switch ( msgpack_unpack_peek( m )) {
        case MSGPACK_UINT8:     DO_UNPACK( m, uint8_t, x, );
        case MSGPACK_FIX:       DO_UNPACK_UFIX( m,x );
        default: break;
    }
    return MSGPACK_TYPEERR;
}
#undef DO_UNPACK
#undef DO_UNPACK_FIX
#undef DO_UNPACK_UFIX

MSGPACKF MSGPACK_ERR msgpack_unpack_float( msgpack_u *m, float *x )
{
    if ( *m->p != MSGPACK_FLOAT )  return MSGPACK_TYPEERR;
    *x = *( float* )++m->p; m->p += sizeof( float );
    return MSGPACK_SUCCESS;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_double( msgpack_u *m, double *x )
{
    switch ( *m->p ) {
        case MSGPACK_DOUBLE:
            *x = *( double* )++m->p; m->p += sizeof( double ); return MSGPACK_SUCCESS;
        case MSGPACK_FLOAT:
            *x = *( float* )++m->p; m->p += sizeof( float ); return MSGPACK_SUCCESS;
    }
    return MSGPACK_TYPEERR;
}

static INLINE MSGPACK_ERR msgpack_unpack_arr_head( msgpack_u *m, byte c1, byte nb, byte c2, uint32_t *n )
{
    byte b = *m->p; ++m->p; *n = 0;
    if (( b>>nb )==( c1>>nb ))  { *n = b & ~c1; }
    else if ( b == c2 )         { msgpack_copy_bits( m->p, n, 2 ); m->p += 2; }
    else if ( b == c2+1 )       { msgpack_copy_bits( m->p, n, 4 ); m->p += 4; }
    else                        { --m->p; return MSGPACK_TYPEERR; }
    return MSGPACK_SUCCESS;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_raw( msgpack_u* m, const byte **data, uint32_t *n )
{
    if ( msgpack_unpack_arr_head( m, 0xa0, 5, MSGPACK_RAW, n )) return MSGPACK_TYPEERR;
    *data = m->p;
    m->p += *n;
    return MSGPACK_SUCCESS;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_str( msgpack_u* m, char *dest, uint32_t max )
{
    const byte *ptr; uint32_t n;
    if ( msgpack_unpack_raw( m, &ptr, &n )) return MSGPACK_TYPEERR;
    if ( n >= max ) return MSGPACK_MEMERR;
    memcpy( dest, ptr, n );
    dest[n] = 0;
    return MSGPACK_SUCCESS;
}
MSGPACKF MSGPACK_ERR msgpack_unpack_array( msgpack_u* m, uint32_t *n )
{
    return msgpack_unpack_arr_head( m, 0x90, 4, MSGPACK_ARRAY, n );
}
MSGPACKF MSGPACK_ERR msgpack_unpack_map( msgpack_u* m, uint32_t *n )
{
    return msgpack_unpack_arr_head( m, 0x80, 4, MSGPACK_MAP, n );
}
