#include "libeitec.h"

/**
 * @file lc_md5.c
 *
 * \addtogroup lc_md5
 * @{
 *
 * A simple MD5 hash routine.
 */

extern int lc_byte_order;

/*
 * These numbers are from http://en.wikipedia.org/wiki/MD5, as of 9/28/2006.
 * 
 * This is the only reference used in designing this algorithm. Therefore, I do
 * not believe I have to adhere to the RSA copyright.
 * 
 * I think the rather inefficient design of this compared to other MD5 routines
 * will attest to that. (compare this to the pseudocode on the Wikipedia
 * entry.)
 * 
 * The calls are modeled after L. Peter Deutsch's implementation, which I
 * presently use in bsflite. Because I already know how to use those, I used
 * the same interface.
 */

const lc_md5int_t r[64] = {
	7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
	5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
	4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
	6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

/*
 * I calculated these numbers myself, again not from the RFC or another RSA
 * source.
 */

lc_md5int_t k[64] = {
	0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE, 0xF57C0FAF,
	0x4787C62A, 0xA8304613, 0xFD469501, 0x698098D8, 0x8B44F7AF,
	0xFFFF5BB1, 0x895CD7BE, 0x6B901122, 0xFD987193, 0xA679438E,
	0x49B40821, 0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
	0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8, 0x21E1CDE6,
	0xC33707D6, 0xF4D50D87, 0x455A14ED, 0xA9E3E905, 0xFCEFA3F8,
	0x676F02D9, 0x8D2A4C8A, 0xFFFA3942, 0x8771F681, 0x6D9D6122,
	0xFDE5380C, 0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
	0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05, 0xD9D4D039,
	0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665, 0xF4292244, 0x432AFF97,
	0xAB9423A7, 0xFC93A039, 0x655B59C3, 0x8F0CCC92, 0xFFEFF47D,
	0x85845DD1, 0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
	0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
};

/*
 * Unlike the pseudocode, I just calculated g prior, since the values are
 * independent of data.
 */

lc_md5int_t g[64] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x06, 0x0B, 0x00,
	0x05, 0x0A, 0x0F, 0x04, 0x09, 0x0E, 0x03, 0x08, 0x0D, 0x02,
	0x07, 0x0C, 0x05, 0x08, 0x0B, 0x0E, 0x01, 0x04, 0x07, 0x0A,
	0x0D, 0x00, 0x03, 0x06, 0x09, 0x0C, 0x0F, 0x02, 0x00, 0x07,
	0x0E, 0x05, 0x0C, 0x03, 0x0A, 0x01, 0x08, 0x0F, 0x06, 0x0D,
	0x04, 0x0B, 0x02, 0x09
};

/*
 * The initial values.
 */

#define H0_I 0x67452301
#define H1_I 0xEFCDAB89
#define H2_I 0x98BADCFE
#define H3_I 0x10325476

/**
 * Initializes a lc_md5state_t state. state must already be allocated
 * before calling.
 */
/* PROTO */
void lc_md5_init(lc_md5state_t * state)
{
	state->h0 = H0_I;
	state->h1 = H1_I;
	state->h2 = H2_I;
	state->h3 = H3_I;

	state->waiting_len = 0;
	state->total_len = 0;

	if (lc_byte_order == LC_ENDIAN_UNSET)
		lc_byte_getendian();
}

/**
 * Append len bytes of message to be computed into md5 hash. The hash will
 * be computed as chunks of 64 bytes are appended, but appends may be
 * of any length.
 */
/* PROTO */
void lc_md5_append(lc_md5state_t * state, lc_md5byte_t * message, lc_size_t len)
{
	lc_size_t bytesleft = len;

	/*
	 * We can calculate it on the fly as blocks come in, whenever we have an
	 * even 64 bytes.
	 */

	if (len + state->waiting_len >= 64) {
		memcpy(state->buf + state->waiting_len, message,
		       64 - state->waiting_len);
		bytesleft -= (64 - state->waiting_len);
		state->waiting_len = 0;
		lc_md5_compute(state);
	}
	while (bytesleft > 64) {
		memcpy(state->buf, message + (len - bytesleft), 64);
		bytesleft -= 64;
		lc_md5_compute(state);
	}

	memcpy(state->buf + state->waiting_len, message + (len - bytesleft),
	       bytesleft);
	state->waiting_len += bytesleft;
	state->total_len += len;
}

/* PROTO */
void lc_md5_compute(lc_md5state_t * state)
{
	int i;
	lc_md5int_t a, b, c, d, f, temp;
	uint32_t chunks[16];

	a = state->h0;
	b = state->h1;
	c = state->h2;
	d = state->h3;
	f = 0;

	for (i = 0; i < 16; i++) {
		chunks[i] = lc_byte_4to32(state->buf + (i * 4));
		chunks[i] = LC_BYTE_SWAP32(chunks[i]);
	}

	for (i = 0; i < 64; i++) {
		switch (i >> 4) {
		case 0:	/* 0 <= i <= 15 */
			f = d ^ (b & (c ^ d));
			break;
		case 1:	/* 16 <= i <= 31 */
			f = c ^ (d & (b ^ c));
			break;
		case 2:	/* 32 <= i <= 47 */
			f = b ^ c ^ d;
			break;
		case 3:	/* 48 <= i <= 63 */
			f = c ^ (b | (~d));
			break;
		}

		temp = d;
		d = c;
		c = b;

		f += a + k[i] + chunks[g[i]];
		f = (f << r[i]) | (f >> (32 - r[i]));
		f += b;
		b = f;

		a = temp;
	}

	state->h0 += a;
	state->h1 += b;
	state->h2 += c;
	state->h3 += d;
}

/**
 * Finish the MD5 hash and place it in a pre-allocated 16-byte array.
 */
/* PROTO */
void lc_md5_finish(lc_md5state_t * state, lc_md5byte_t digest[16])
{
	uint32_t original_len = (uint32_t) state->total_len;
	uint32_t temp;

	uint8_t firstappend = 0x80;
	uint8_t fourzeros[] = { 0x00, 0x00, 0x00, 0x00 };

	/*
	 * We want to append a "1" bit to the end of the message, then enough "0"
	 * bits so that the length in bits is congruent to 448 (mod 512). In
	 * bytes, that's 56 (mod 64).
	 * 
	 * Then, we append the original length (before doing this) in a
	 * little-endian 64 bit integer. But since we're working with 32-bit
	 * integers, we only append the 32-bit integer in little endian form,
	 * plus 4 zero bytes.
	 */

	lc_md5_append(state, &firstappend, 1);

	while ((state->total_len % 64) != 56)
		lc_md5_append(state, fourzeros, 1);

	/*
	 * We need to append the length in bits, not bytes.
	 */
	original_len *= 8;

	if (lc_byte_order == LC_BIG_ENDIAN)
		temp = LC_BYTE_SWAP32(original_len);
	else
		temp = original_len;

	lc_md5_append(state, (uint8_t *) & temp, 4);
	lc_md5_append(state, fourzeros, 4);

	lc_byte_32to4(LC_BYTE_SWAP32(state->h0), digest);
	lc_byte_32to4(LC_BYTE_SWAP32(state->h1), digest + 4);
	lc_byte_32to4(LC_BYTE_SWAP32(state->h2), digest + 8);
	lc_byte_32to4(LC_BYTE_SWAP32(state->h3), digest + 12);
}
