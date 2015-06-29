#include "oauth.h"

/* PROTO */
oauth_s_t *oauth_init(int version, int sig_method, char *oauth_consumer_key,
		      char *oauth_consumer_secret)
{
	oauth_s_t *n = malloc(sizeof(struct OAuthState));

	if (n != NULL) {
		n->version = version;
		n->sig_method = sig_method;
		n->oauth_consumer_key = strdup(oauth_consumer_key);
		n->oauth_consumer_secret = strdup(oauth_consumer_secret);

		n->oauth_token = NULL;
		n->oauth_token_secret = NULL;

		n->signing_key = malloc(strlen(oauth_consumer_secret) + 1 + 1);
		snprintf(n->signing_key, strlen(oauth_consumer_secret) + 1 + 1,
			 "%s&", oauth_consumer_secret);
	}

	return n;
}

/* PROTO */
void oauth_destroy(oauth_s_t * state)
{
	free(state->oauth_consumer_key);
	free(state->oauth_consumer_secret);
	free(state->oauth_token);
	free(state->oauth_token_secret);
	free(state->signing_key);
	free(state);
}

/* PROTO */
void oauth_free(oauth_r_t * req)
{
	if (req->body_params)
		lc_list_destroy(req->body_params);
	if (req->oauth_params)
		lc_list_destroy(req->oauth_params);
	if (req->qstring_params)
		lc_list_destroy(req->qstring_params);
	if (req->headers)
		lc_list_destroy(req->headers);
}

/* PROTO */
int oauth_set_token(oauth_s_t * s, char *oauth_token, char *oauth_token_secret)
{
	free(s->oauth_token);
	free(s->oauth_token_secret);

	s->oauth_token = strdup(oauth_token);
	if (s->oauth_token == NULL)
		return 0;

	s->oauth_token_secret = strdup(oauth_token_secret);
	if (s->oauth_token_secret == NULL)
		return 0;

	free(s->signing_key);

	s->signing_key = malloc(strlen(s->oauth_consumer_secret) + 1 +
				strlen(oauth_token_secret) + 1);
	if (s->signing_key == NULL)
		return 0;

	snprintf(s->signing_key, strlen(s->oauth_consumer_secret) + 1 +
		 strlen(oauth_token_secret) + 1, "%s&%s",
		 s->oauth_consumer_secret, oauth_token_secret);

	return 1;
}

/* PROTO */
void
oauth_get_token(oauth_s_t * s, char **oauth_token, char **oauth_token_secret)
{
	*oauth_token = s->oauth_token;
	*oauth_token_secret = s->oauth_token_secret;
}

/* PROTO */
lc_list_t oauth_prepare(oauth_s_t * s)
{
	char timestamp[32], buffer[128];
	char *nonce, *p, *d;
	unsigned char random[32];
	time_t ts;
	size_t x, buflen;
	lc_list_t n;

	n = lc_list_create((lc_createfn_t) param_list_create,
			   (lc_destroyfn_t) param_list_destroy,
			   (lc_comparefn_t) kv_t_cmp);
	if (n != NULL) {
		switch (s->version) {
		case OAUTH_VERSION_1_0:
			INSERT_KV_T(n, "oauth_version", "1.0");
			break;
		default:
			INSERT_KV_T(n, "oauth_version", "unknown");
		}

		switch (s->sig_method) {
		case OAUTH_SIG_HMAC_SHA1:
			INSERT_KV_T(n, "oauth_signature_method", "HMAC-SHA1");
			break;
		default:
			INSERT_KV_T(n, "oauth_signature_method", "unknown");
		}

		INSERT_KV_T(n, "oauth_consumer_key", s->oauth_consumer_key);
		if (s->oauth_token)
			INSERT_KV_T(n, "oauth_token", s->oauth_token);

		/*
		 * Generate a timestamp. This is just UNIX epoch time
		 * as a string. MacOS (not X) stores using a different
		 * epoch so we use our libcompat's implementation.
		 *
		 * Twitter only accepts requests within some reasonable
		 * timeframe after this so an accurate clock is necessary.
		 */
#ifdef macintosh
		ts = __time(NULL);
#else
		ts = time(NULL);
#endif

		snprintf(timestamp, sizeof(timestamp), "%lu",
			 (unsigned long)ts);
		INSERT_KV_T(n, "oauth_timestamp", timestamp);

		/* 
		 * Generate a random string using RNG and base64.
		 * Twitter uses this to identify duplicate requests.
		 *
		 * It's just generating 32 random values, encoding
		 * that with base64, and then strips out non-alphanumeric
		 * characters.
		 *
		 * This is what Twitter uses as an example in their
		 * docs, so I'll use it.
		 */
		for (x = 0; x < 32; x++)
			random[x] = rand() % 256;

		buflen = sizeof(buffer);
		if (base64_encode((unsigned char *)buffer, &buflen,
					random, 32) == 0) {
			nonce = malloc(buflen + 1);
			for (p = buffer, d = nonce; *p; p++)
				if (isalnum(*p))
					*(d++) = *p;

			*(d++) = 0;

			INSERT_KV_T(n, "oauth_nonce", nonce);
			free(nonce);
		}
	}

	return n;
}

/* PROTO */
void make_param_str(lc_item_t * i, va_list ap)
{
	va_list ap2;
	kv_t *ki = (kv_t *) i;
	char *buf;
	size_t bufsize;

	va_copy(ap2, ap);
	buf = va_arg(ap2, char *);
	bufsize = va_arg(ap2, size_t);
	va_end(ap2);

	if (buf[0] != 0)
		strlcat(buf, "&", bufsize);

	strlcat(buf, ki->key, bufsize);
	strlcat(buf, "=", bufsize);
	strlcat(buf, ki->value, bufsize);
}

/* PROTO */
void make_query_str(kv_t * i, va_list ap)
{
	va_list ap2;
	char *buf, *ekey, *evalue;
	size_t bufsize;

	va_copy(ap2, ap);
	buf = va_arg(ap2, char *);
	bufsize = va_arg(ap2, size_t);
	va_end(ap2);

	if (buf[0] == 0)
		strlcat(buf, "?", bufsize);
	else
		strlcat(buf, "&", bufsize);

	ekey = urlencode((unsigned char *)i->key, strlen(i->key));
	evalue = urlencode((unsigned char *)i->value, strlen(i->value));

	strlcat(buf, ekey, bufsize);
	free(ekey);
	strlcat(buf, "=", bufsize);
	strlcat(buf, evalue, bufsize);
	free(evalue);
}

/* PROTO */
void make_oauth_header(kv_t * i, va_list ap)
{
	char *buf, *ekey, *evalue;
	size_t bufsize;
	va_list ap2;

	va_copy(ap2, ap);
	buf = va_arg(ap2, char *);
	bufsize = va_arg(ap2, size_t);
	va_end(ap2);

	if (buf[0] != 0)
		strlcat(buf, ", ", bufsize);
	else
		strlcat(buf, "OAuth ", bufsize);

	ekey = urlencode((unsigned char *)i->key, strlen(i->key));
	evalue = urlencode((unsigned char *)i->value, strlen(i->value));

	strlcat(buf, ekey, bufsize);
	free(ekey);
	strlcat(buf, "=\"", bufsize);
	strlcat(buf, evalue, bufsize);
	free(evalue);
	strlcat(buf, "\"", bufsize);
}

/* PROTO */
int oauth_sign(oauth_r_t * oreq)
{
	char buf[4096], final_key_buf[8192];
	char *e_param_str, *e_url;
	lc_list_t final_param_list;
	size_t buflen;

	unsigned char sha1_out[20];

	/*
	 * "final list" is an ordered list of all parameters, including
	 * all oauth_ params and anything specified in the query string
	 */

	final_param_list = lc_list_create((lc_createfn_t) final_list_create,
					  (lc_destroyfn_t) final_list_free,
					  (lc_comparefn_t) kv_t_cmp);

	lc_list_foreach_v(oreq->oauth_params,
			  (lc_foreachfn_v_t) final_list_iterator,
			  final_param_list);

	if (oreq->body_params != NULL)
		lc_list_foreach_v(oreq->body_params,
				  (lc_foreachfn_v_t) final_list_iterator,
				  final_param_list);

	if (oreq->qstring_params != NULL)
		lc_list_foreach_v(oreq->qstring_params,
				  (lc_foreachfn_v_t) final_list_iterator,
				  final_param_list);

	memset(buf, 0, sizeof(buf));
	lc_list_foreach_v(final_param_list, (lc_foreachfn_v_t) make_param_str,
			  buf, sizeof(buf));
	lc_list_destroy(final_param_list);

	e_param_str = urlencode((unsigned char *)buf, strlen(buf));
	e_url = urlencode((unsigned char *)oreq->url, strlen(oreq->url));

	memset(final_key_buf, 0, sizeof(final_key_buf));
	snprintf(final_key_buf, sizeof(final_key_buf), "%s&%s&%s", oreq->method,
		 e_url, e_param_str);

	free(e_param_str);
	free(e_url);

	sha1_hmac((unsigned char *)oreq->state->signing_key,
		  strlen(oreq->state->signing_key),
		  (unsigned char *)final_key_buf, strlen(final_key_buf),
		  sha1_out);

	buflen = sizeof(buf);
	if (base64_encode((unsigned char *)buf, &buflen, sha1_out, 20) == 0)
		INSERT_KV_T(oreq->oauth_params, "oauth_signature", buf);

	memset(buf, 0, sizeof(buf));
	lc_list_foreach_v(oreq->oauth_params,
			  (lc_foreachfn_v_t) make_oauth_header, buf,
			  sizeof(buf));

	if (oreq->headers == NULL)
		oreq->headers =
		    lc_list_create((lc_createfn_t) param_list_create,
				   (lc_destroyfn_t) param_list_destroy,
				   (lc_comparefn_t) kv_t_cmp);

	INSERT_KV_T(oreq->headers, "Authorization", buf);

	return 1;
}
