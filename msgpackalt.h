/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
prototypes of msgpackalt library functions implementing the
"Message Pack" protocol, available at
http://wiki.msgpack.org/display/MSGPACK/Format+specification

use preprocessor definitions
	MSGPACK_INLINE to include definitions and compile inline
	MSGPACK_BUILDDLL to export functions to dll

requires one of __BYTE_ORDER__, __LITTLE_ENDIAN__ or __BIG_ENDIAN__
to be defined to determine host byte order for byte swapping

documentation and examples are available at:
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
*/
#ifndef MSGPACK_H
#define MSGPACK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
    #include "stdint_msc.h" /* from http://code.google.com/p/msinttypes */
#else
    #include <stdint.h>
#endif
#define INLINE __inline

typedef uint8_t byte;
#ifndef __cplusplus
typedef byte bool;
#endif

#ifdef MSGPACK_BUILDDLL
    #define MSGPACKF __declspec( dllexport )
    #ifdef MSGPACK_INLINE
        #error Cannot compile both inline AND to DLL
    #endif
#else
    #ifdef MSGPACK_INLINE
        #define MSGPACKF INLINE
    #else
        #define MSGPACKF 
    #endif
#endif

/* **************************************** ENDIANNESS **************************************** */
#ifndef __LITTLE_ENDIAN__
    #define __LITTLE_ENDIAN__ __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#endif
#ifndef __BIG_ENDIAN__
    #define __BIG_ENDIAN__ __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#endif
#if __LITTLE_ENDIAN__           /* have to swap for network-endian */
    #ifdef _MSC_VER
        // MSVC builtins, http://msdn.microsoft.com/en-us/library/a3140177.aspx
        #define BYTESWAP16   _byteswap_ushort
        #define BYTESWAP32   _byteswap_ulong
        #define BYTESWAP64   _byteswap_uint64
    #elif ( __GNUC__*100+__GNUC_MINOR__ >= 403 )
        // GCC v4.3+ builtins, http://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
        //http://old.nabble.com/-Bug-target-52624--New%3A-Missing-__builtin_bswap16-td33533370.html
        static inline unsigned short __builtin_bswap16( unsigned short a ) { return (a<<8)|(a>>8); } 
        #define BYTESWAP16   __builtin_bswap16
        #define BYTESWAP32   __builtin_bswap32
        #define BYTESWAP64   __builtin_bswap64
    #else
        // attempt generic functions
        #include <byteswap.h>
        #define BYTESWAP16   bswap_16
        #define BYTESWAP32   bswap_32
        #define BYTESWAP64   bswap_64
    #endif
#elif __BIG_ENDIAN__ /* already network-endian */
    #define BYTESWAP16
    #define BYTESWAP32
    #define BYTESWAP64
#else
    #error Unsupported endian-ness
#endif


/* **************************************** MSGPACK DEFINITIONS **************************************** */
typedef enum { /* error codes are -ve */
    MSGPACK_SUCCESS = 0,
    MSGPACK_TYPEERR = -1,
    MSGPACK_MEMERR = -2,
    MSGPACK_ARGERR = -3
} MSGPACK_ERR;

typedef enum { /* byte codes: NB fix, raw, array and map have multiple possibilities */
    MSGPACK_FIX     = 0x7f,
    MSGPACK_NULL    = 0xc0,
    MSGPACK_FALSE   = 0xc2,
    MSGPACK_TRUE    = 0xc3,
    MSGPACK_FLOAT   = 0xca,
    MSGPACK_DOUBLE  = 0xcb,
    MSGPACK_UINT8   = 0xcc,
    MSGPACK_UINT16  = 0xcd,
    MSGPACK_UINT32  = 0xce,
    MSGPACK_UINT64  = 0xcf,
    MSGPACK_INT8    = 0xd0,
    MSGPACK_INT16   = 0xd1,
    MSGPACK_INT32   = 0xd2,
    MSGPACK_INT64   = 0xd3,
    MSGPACK_RAW     = 0xda,
    MSGPACK_ARRAY   = 0xdc,
    MSGPACK_MAP     = 0xde
} MSGPACK_TYPE_CODES;

typedef struct {
    uint32_t max;           // size of allocated buffer
    byte *p, *buffer;       // pointer to current location, start of buffer
} msgpack_p;

typedef struct {
    uint32_t max;           // size of allocated buffer
    const byte *p, *end;    // pointer to current location, start of buffer
} msgpack_u;


/* **************************************** MEMORY FUNCTIONS **************************************** */
MSGPACKF msgpack_p* msgpack_pack_init( );
MSGPACKF MSGPACK_ERR msgpack_pack_free( msgpack_p *m );
MSGPACKF uint32_t msgpack_get_len( const msgpack_p *m );
MSGPACKF MSGPACK_ERR msgpack_get_buffer( msgpack_p *m, const byte ** data, uint32_t *n );
MSGPACKF uint32_t msgpack_copy_to( const msgpack_p *m, byte *data, uint32_t max );


/* **************************************** PACKING FUNCTIONS **************************************** */
MSGPACKF MSGPACK_ERR msgpack_pack_null( msgpack_p* m );
MSGPACKF MSGPACK_ERR msgpack_pack_bool( msgpack_p* m, bool x );
MSGPACKF MSGPACK_ERR msgpack_pack_fix( msgpack_p* m, int8_t x );
MSGPACKF MSGPACK_ERR msgpack_pack_int8( msgpack_p *m, int8_t x );
MSGPACKF MSGPACK_ERR msgpack_pack_int16( msgpack_p *m, int16_t x );
MSGPACKF MSGPACK_ERR msgpack_pack_int32( msgpack_p *m, int32_t x );
MSGPACKF MSGPACK_ERR msgpack_pack_int64( msgpack_p *m, int64_t x );
MSGPACKF MSGPACK_ERR msgpack_pack_uint8( msgpack_p *m, uint8_t x );
MSGPACKF MSGPACK_ERR msgpack_pack_uint16( msgpack_p *m, uint16_t x );
MSGPACKF MSGPACK_ERR msgpack_pack_uint32( msgpack_p *m, uint32_t x );
MSGPACKF MSGPACK_ERR msgpack_pack_uint64( msgpack_p *m, uint64_t x );
MSGPACKF MSGPACK_ERR msgpack_pack_float( msgpack_p *m, float x );
MSGPACKF MSGPACK_ERR msgpack_pack_double( msgpack_p *m, double x );
MSGPACKF MSGPACK_ERR msgpack_pack_raw( msgpack_p* m, const byte *data, uint32_t n );
MSGPACKF MSGPACK_ERR msgpack_pack_str( msgpack_p* m, const char *str );
MSGPACKF MSGPACK_ERR msgpack_pack_array( msgpack_p* m, uint32_t n );
MSGPACKF MSGPACK_ERR msgpack_pack_map( msgpack_p* m, uint32_t n );
MSGPACKF MSGPACK_ERR msgpack_pack_header( msgpack_p *m );


/* **************************************** UNPACKING FUNCTIONS **************************************** */
MSGPACKF msgpack_u* msgpack_unpack_init( const byte* data, const uint32_t n );
MSGPACKF void msgpack_unpack_free( msgpack_u *m );
MSGPACKF MSGPACK_TYPE_CODES msgpack_unpack_peek( const msgpack_u *m );
MSGPACKF uint32_t msgpack_unpack_len( msgpack_u *m );
MSGPACKF int msgpack_unpack_header( msgpack_u *m );

MSGPACKF MSGPACK_ERR msgpack_unpack_null( msgpack_u *m );
MSGPACKF int msgpack_unpack_bool( msgpack_u *m );
MSGPACKF MSGPACK_ERR msgpack_unpack_fix( msgpack_u *m, int8_t *x );
MSGPACKF MSGPACK_ERR msgpack_unpack_int8( msgpack_u *m, int8_t *x );
MSGPACKF MSGPACK_ERR msgpack_unpack_int16( msgpack_u *m, int16_t *x );
MSGPACKF MSGPACK_ERR msgpack_unpack_int32( msgpack_u *m, int32_t *x );
MSGPACKF MSGPACK_ERR msgpack_unpack_int64( msgpack_u *m, int64_t *x );
MSGPACKF MSGPACK_ERR msgpack_unpack_uint8( msgpack_u *m, uint8_t *x );
MSGPACKF MSGPACK_ERR msgpack_unpack_uint16( msgpack_u *m, uint16_t *x );
MSGPACKF MSGPACK_ERR msgpack_unpack_uint32( msgpack_u *m, uint32_t *x );
MSGPACKF MSGPACK_ERR msgpack_unpack_uint64( msgpack_u *m, uint64_t *x );
MSGPACKF MSGPACK_ERR msgpack_unpack_float( msgpack_u *m, float *x );
MSGPACKF MSGPACK_ERR msgpack_unpack_double( msgpack_u *m, double *x );
MSGPACKF MSGPACK_ERR msgpack_unpack_raw( msgpack_u* m, const byte **data, uint32_t *n );
MSGPACKF MSGPACK_ERR msgpack_unpack_str( msgpack_u* m, char *dest, uint32_t max );
MSGPACKF MSGPACK_ERR msgpack_unpack_array( msgpack_u* m, uint32_t *n );
MSGPACKF MSGPACK_ERR msgpack_unpack_map( msgpack_u* m, uint32_t *n );

#ifdef MSGPACK_INLINE
    #include "msgpackalt.c"
#endif

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* MSGPACK_H */
