#ifndef __LIBEITEC_BYTE_H
#define __LIBEITEC_BYTE_H

#define LC_BYTE_SWAP16(x) (((x & 0xFF00) >> 8) | \
		                         ((x & 0x00FF) << 8))

#define LC_BYTE_SWAP32(x) (((x & 0xFF000000) >> 24) | \
		                         ((x & 0x00FF0000) >> 8) | \
		                         ((x & 0x0000FF00) << 8) | \
		                         ((x & 0x000000FF) << 24))

enum
{
    LC_BIG_ENDIAN,
    LC_LITTLE_ENDIAN,
    LC_ENDIAN_UNSET
};

#endif
