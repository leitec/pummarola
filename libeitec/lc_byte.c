#include "libeitec.h"

/**
 * @file lc_byte.c
 *
 * \addtogroup lc_byte
 * @{
 *
 *  Functions relating to byte order and manipulation.
 */

/**
 *  Needs to be intialized by lc_byte_getendian() before calling
 *  other lc_byte functions.
 */
int lc_byte_order = LC_ENDIAN_UNSET;

/**
 *  Retrieve endianness into a global variable.
 */
/* PROTO */
int
lc_byte_getendian (void)
{
    uint16_t blah = 0x5533;
    uint8_t *ptr = (uint8_t *) & blah;

    if ((*ptr) == 0x55)
	lc_byte_order = LC_BIG_ENDIAN;
    else
	lc_byte_order = LC_LITTLE_ENDIAN;

    return lc_byte_order;
}

/**
 *  Copy 4 bytes from a big-endian byte string into a 32-bit unsigned integer,
 *  adjusting for endianness.
 */
/* PROTO */
uint32_t
lc_byte_4to32 (uint8_t * buf)
{
    uint32_t result;

    memcpy (&result, buf, 4);

    if (lc_byte_order == LC_LITTLE_ENDIAN)
	result = LC_BYTE_SWAP32 (result);

    return result;
}

/**
 *  Copy a host-order 32-bit integer into 4 bytes of a big-endian
 *  byte string, adjusting for endianness. Assumes buf has enough
 *  room for four bytes.
 */
/* PROTO */
uint8_t *
lc_byte_32to4 (uint32_t orig, uint8_t * buf)
{
    uint32_t fixed;

    if (lc_byte_order == LC_LITTLE_ENDIAN)
	fixed = LC_BYTE_SWAP32 (orig);
    else
	fixed = orig;

    memcpy (buf, &fixed, 4);

    return buf;
}

/* @} */
