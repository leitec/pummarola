#ifndef __LIBEITEC_MD5_H
#define __LIBEITEC_MD5_H

typedef uint32_t lc_md5int_t;
typedef uint8_t lc_md5byte_t;

typedef struct LC_MD5_State {
	uint8_t buf[64];
	lc_size_t waiting_len;
	lc_size_t total_len;

	lc_md5int_t h0;
	lc_md5int_t h1;
	lc_md5int_t h2;
	lc_md5int_t h3;
} lc_md5state_t;

#endif
