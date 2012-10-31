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

further documentation and examples are available at:
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


/* **************************************** MSGPACK DEFINITIONS **************************************** */
typedef enum { 				/* -ve values indicate error */
	MSGPACK_SUCCESS = 0,	/* no problem */
	MSGPACK_TYPEERR = -1,	/* type code did not match expected value */
	MSGPACK_MEMERR = -2,	/* out of memory error */
	MSGPACK_ARGERR = -3		/* received unexpected argument */
} MSGPACK_ERR;

typedef enum { /* byte codes indicating types: NB fix, raw, array and map have multiple codes */
	MSGPACK_FIX     = 0x7f,		/* fixnums are integers between (-32, 128) */
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
	uint32_t max;           /* size of allocated buffer */
	byte *p, *buffer;       /* pointer to current location, start of buffer */
} msgpack_p;

typedef struct {
	uint32_t max;           /* size of allocated buffer */
	const byte *p, *end;    /* pointer to current location, end of buffer */
	byte flags;
} msgpack_u;


/* **************************************** MEMORY FUNCTIONS **************************************** */
MSGPACKF msgpack_p* msgpack_pack_init( );
/* create a packer (msgpack_p) object, allocate some memory and return a pointer */

MSGPACKF MSGPACK_ERR msgpack_pack_free( msgpack_p *m );
/* free the packer object and its associated memory. do not reference *m after calling this function */

MSGPACKF uint32_t msgpack_get_len( const msgpack_p *m );
/* return the current length of the packed buffer */

MSGPACKF MSGPACK_ERR msgpack_get_buffer( msgpack_p *m, const byte ** data, uint32_t *n );
/* stores a pointer to the buffer memory in "data" and the length of the packed buffer in "n". do not modify the buffer directly */

MSGPACKF uint32_t msgpack_copy_to( const msgpack_p *m, void *data, uint32_t max );
/* copies the internal buffer into the user-specified buffer "data" with length "max", returning the number of bytes copied.
	note that if the buffer is to small to contain the packed message, no copy is performed.  */

/* **************************************** PACKING FUNCTIONS **************************************** */
/* the packing function pack the given variable into the buffer. if the value can be stored in a smaller
	representation, it is packed in the smallest form that does not produce loss of data
	e.g. uint32_t x = 64 gets automatically stored as a fixnum (1 byte) not uint32 (5 bytes)
	similar automatic conversion is performed on unpacking to ensure seamless conversion
*/
/* base types ------------------------- */
MSGPACKF MSGPACK_ERR msgpack_pack_null( msgpack_p* m );
MSGPACKF MSGPACK_ERR msgpack_pack_bool( msgpack_p* m, bool x );
MSGPACKF MSGPACK_ERR msgpack_pack_fix( msgpack_p* m, int8_t x );   /* -32 < x < 128 */
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
/* array types ------------------------- */
MSGPACKF MSGPACK_ERR msgpack_pack_raw( msgpack_p* m, const byte *data, uint32_t n );
MSGPACKF MSGPACK_ERR msgpack_pack_str( msgpack_p* m, const char *str );   /* convenience wrapper for msgpack_pack_raw taking n=strlen */
MSGPACKF MSGPACK_ERR msgpack_pack_array( msgpack_p* m, uint32_t n );
MSGPACKF MSGPACK_ERR msgpack_pack_map( msgpack_p* m, uint32_t n );

MSGPACKF MSGPACK_ERR msgpack_pack_append( msgpack_p *m, const void* data, uint32_t n );
MSGPACKF MSGPACK_ERR msgpack_pack_header( msgpack_p *m );
/* EXTENSION: packs a unsigned int value to the start of the message specifying the length of the buffer.
provides a way to check whether a given binary string is a msgpack'd buffer or not */

/* **************************************** UNPACKING FUNCTIONS **************************************** */
MSGPACKF msgpack_u* msgpack_unpack_init( const void* data, uint32_t n, const int flags );
/* creates an unpacker (msgpack_u) object, to unpack the "n" byte buffer pointed to by "data"
if "flags" is non-zero, a copy of the data is made, else the data pointer is used directly and should not
be free'd until after msgpack_unpack_free is called */

MSGPACKF MSGPACK_ERR msgpack_unpack_free( msgpack_u *m );
/* frees the unpacker object. the data buffer that was being unpacked can now be safely free'd */

MSGPACKF int msgpack_unpack_peek( const msgpack_u *m );
/* returns the type code of the next object stored in the buffer */

MSGPACKF uint32_t msgpack_unpack_len( msgpack_u *m );
/* return the number of bytes in the buffer remaining to be unpacked */

MSGPACKF MSGPACK_ERR msgpack_unpack_append( msgpack_u *m, const void* data, const uint32_t n );
/* appends more data to the end of the buffer for unpacking */


/* the unpacking functions check whether the next object in the buffer can be unpacked
into the specified data type. if possible without data loss, relevant conversion is performed.
If conversion is not possible, the MSGPACK_TYPEERR code is returned and the buffer is 
not advanced, so another attempt can be made to unpack it.
up-conversion of numeric types (e.g. float to double, int8 to int64) is automatic.
*/
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

MSGPACKF int msgpack_unpack_skip( msgpack_u *m );

MSGPACKF int msgpack_unpack_header( msgpack_u *m );
/* EXTENSION: unpacks an unsigned int from the buffer and checks that it equals the length of the buffer.
this provides a way to check whether arbitrary data is indeed a msgpack'd buffer */

#ifdef MSGPACK_INLINE	/* compiling inline so include the source code */
	#include "msgpackalt.c"
#endif

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* MSGPACK_H */
