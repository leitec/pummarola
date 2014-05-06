#include "oauth.h"

static inline unsigned long decode_hex_char(const char *);

/* PROTO */
char *urlencode(const unsigned char *original, size_t origlen)
{
	const unsigned char *pp;
	char *nn, *encoded;
	size_t len;

	for (len = 0, pp = original; pp < original + origlen; pp++) {
		if(isalnum(*pp) || *pp == '-' || *pp == '_'
				|| *pp == '.' || *pp == '~')
			len += 1;
		else
			len += 3;
	}

	encoded = malloc(len + 1);
	if (encoded != NULL) {
		nn = encoded;

		for (pp = original; pp < original + origlen; pp++) {
			if (isalnum(*pp) || *pp == '-' || *pp == '_'
					 || *pp == '.' || *pp == '~') {
				*(nn++) = *pp;
			} else {
				snprintf((char *)nn, 4, "%%%02X", *pp);
				nn += 3;
			}
		}

		encoded[len] = 0;
	}

	return encoded;
}

static inline unsigned long decode_hex_char(const char *c)
{
	unsigned long lc;

	if (*c >= 'a')
		lc = *c - 'a' + 10;
	else if (*c >= 'A')
		lc = *c - 'A' + 10;
	else
		lc = *c - '0';

	return lc;
}

/* PROTO */
unsigned char *urldecode(const char *encoded)
{
	unsigned char *decoded;
	unsigned long curchar = 0;
	const char *pp;
	size_t e_len, d_len;
	int state;

	e_len = strlen(encoded);
	decoded = malloc(e_len + 1);

	for (state = 0, pp = encoded, d_len = 0; *pp; pp++) {
		if (state == 0) {
			if (*pp == '%') {
				state = 1;
				continue;
			} else {
				decoded[d_len++] = *pp;
			}
		} else if (state == 1) {
			curchar = decode_hex_char(pp) << 4;

			state = 2;
			continue;
		} else if (state == 2) {
			curchar += decode_hex_char(pp);

			decoded[d_len++] = (unsigned char)curchar;
			state = 0;
			continue;
		}
	}

	if (d_len < e_len)
		decoded = realloc(decoded, d_len + 1);

	decoded[d_len] = 0;

	return decoded;
}

/*
 * need to determine what makes the most sense
 * for real-life stuff
 */
#define _L_TMP_BUF_SIZE 64

/* PROTO */
lc_list_t param_list_from_qstring(const char *qstring)
{
	int state, i, j, k, len;
	char *buf[2];
	size_t bsiz[2];
	lc_list_t n;

	len = strlen(qstring);
	n = lc_list_create((lc_createfn_t) param_list_create_urldecode,
			   (lc_destroyfn_t) param_list_destroy,
			   (lc_comparefn_t) kv_t_cmp);

	buf[0] = malloc(_L_TMP_BUF_SIZE + 1);
	buf[1] = malloc(_L_TMP_BUF_SIZE + 1);
	bsiz[0] = _L_TMP_BUF_SIZE;
	bsiz[1] = _L_TMP_BUF_SIZE;

	/*
	 * j = current position in buffer
	 * state = determines which buffer it's acting on
	 *  0 - building key string
	 *  1 - building value string
	 */

	for (i = 0, j = 0, k = 0, state = 0; i <= len; i++) {
		switch (qstring[i]) {
		case '&':
		case '\0':
			if (state != 1) {
				printf("error: '&' when state != 1\n");
				k = -1;
				break;
			}

			/*
			 * we should always have 1 more than
			 * bsiz so we can null-terminate the value
			 */
			buf[1][j] = '\0';
#ifdef OAUTH_DEBUG
			printf("value='%s'\n", buf[1]);
#endif

			INSERT_KV_T(n, buf[0], buf[1]);

			j = 0;
			state = 0;
			break;
		case '=':
			if (state != 0) {
				printf("error: '=' when state != 0\n");
				k = -1;
				break;
			}

			/*
			 * ditto
			 */
			buf[0][j] = '\0';
#ifdef OAUTH_DEBUG
			printf("key='%s'\n", buf[0]);
#endif

			j = 0;
			state = 1;
			break;
		default:
			if (j >= bsiz[state]) {
				bsiz[state] += _L_TMP_BUF_SIZE;
				buf[state] = realloc(buf[state], bsiz[state]);
			}

			buf[state][j++] = qstring[i];
		}
	}

	free(buf[0]);
	free(buf[1]);

	if (k == -1) {
		lc_list_destroy(n);
		return NULL;
	} else
		return n;
}
