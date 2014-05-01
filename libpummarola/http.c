#include "libpummarola.h"

#include <string.h>

#define USER_AGENT_STR "User-Agent: Pummarola crappy HTTP 1.0\n"

#ifdef macintosh
extern mactcp_inst mi;
#endif

/*
 * Stuff for tinyhttp to parse the responses
 *
 * adapted for plain C from tinyhttp's example.cpp
 */

static void *response_realloc(void *opaque, void *ptr, int size)
{
	return realloc(ptr, size);
}

/*
 * body is just a long string
 */
static void response_body(void *opaque, const char *data, int size)
{
	http_response *response = (http_response *) opaque;
	size_t len;

	if (response->body != NULL)
		len = response->body_len;
	else
		len = 0;

	response->body = realloc(response->body, len + size + 1);
	memcpy(response->body + len, data, size);
	response->body[len + size] = 0;

	response->body_len = len + size;
}

/*
 * headers are a kv_t list
 */
static void
response_header(void *opaque, const char *ckey, int nkey, const char *cvalue,
		int nvalue)
{
	http_response *response = (http_response *) opaque;
	char *key = malloc(nkey + 1);
	char *value = malloc(nvalue + 1);

	memcpy(key, ckey, nkey);
	memcpy(value, cvalue, nvalue);

	key[nkey] = 0;
	value[nvalue] = 0;

	if (response->header == NULL)
		response->header = lc_list_create((lc_createfn_t)
						  param_list_create_nodup,
						  (lc_destroyfn_t)
						  param_list_destroy,
						  (lc_comparefn_t) kv_t_cmp);

	/*
	 * this list's create function doesn't use strdup()
	 * so we don't need to free anything after inserting
	 */
	INSERT_KV_T(response->header, key, value);
}

static void response_code(void *opaque, int code)
{
	http_response *response = (http_response *) opaque;
	response->code = code;
}

static const struct http_funcs responseFuncs = {
	response_realloc,
	response_body,
	response_header,
	response_code,
};

/*
 * Stuff for preparing and sending signed HTTP(S) requests.
 *
 * TODO: rewrite this in less-nasty manner
 */

/*
 * Generate the "Authorization:" header with the OAuth
 * parameters in order
 *
 * The order is determined by the list, not this function.
 * We use a sorting insert function on the list for that.
 *
 * Params (via stdarg)
 *  m - list size
 *  c - current index
 *  request - string buffer
 *  req_size - buffer size
 */
/* PROTO */
void print_oauth_params(kv_t * i, va_list ap)
{
	int *c, m;
	char *ekey, *evalue;
	char *request;
	size_t req_size;
	va_list ap2;

	va_copy(ap2, ap);

	m = va_arg(ap2, int);
	c = va_arg(ap2, int *);
	request = va_arg(ap2, char *);
	req_size = va_arg(ap2, size_t);

	va_end(ap2);

	if (*c == 0)
		strlcat(request, "Authorization: OAuth ", req_size);

	ekey = urlencode((unsigned char *)i->key, strlen(i->key));
	evalue = urlencode((unsigned char *)i->value, strlen(i->value));

	strlcat(request, ekey, req_size);
	free(ekey);
	strlcat(request, "=\"", req_size);
	strlcat(request, evalue, req_size);
	free(evalue);
	strlcat(request, "\"", req_size);

	*c += 1;

	if (*c < m)
		strlcat(request, ", ", req_size);
	else
		strlcat(request, "\n", req_size);
}

/*
 * Append additional headers via key/value pairs
 *
 * Parameters (via stdarg)
 *  request - string buffer
 *  req_size - buffer size
 */
/* PROTO */
void print_extra_params(kv_t * i, va_list ap)
{
	char *request;
	size_t req_size;
	va_list ap2;

	va_copy(ap2, ap);
	request = va_arg(ap2, char *);
	req_size = va_arg(ap2, size_t);
	va_end(ap2);

	strlcat(request, i->key, req_size);
	strlcat(request, ": ", req_size);
	strlcat(request, i->value, req_size);
	strlcat(request, "\n", req_size);
}

/*
 * Append query string parameters via key/value pairs
 *
 * Note that this'd break with any URL that already
 * has part of a query string, due to the "?" marker
 * being repeated.
 *
 * Parameters (via stdarg)
 *  request - string buffer
 *  req_size - buffer size
 */
/* PROTO */
void print_query_str(kv_t * i, va_list ap)
{
	va_list ap2;
	char *request, *ekey, *evalue;
	int *index;
	size_t req_size;

	va_copy(ap2, ap);
	request = va_arg(ap2, char *);
	req_size = va_arg(ap2, size_t);
	index = va_arg(ap2, int *);
	va_end(ap2);

	/* see note in comment above */
	if ((*index)++ == 0)
		strlcat(request, "?", req_size);
	else
		strlcat(request, "&", req_size);

	ekey = urlencode((unsigned char *)i->key, strlen(i->key));
	evalue = urlencode((unsigned char *)i->value, strlen(i->value));

	strlcat(request, ekey, req_size);
	strlcat(request, "=", req_size);
	strlcat(request, evalue, req_size);

	free(ekey);
	free(evalue);
}

/* PROTO */
int send_signed_https_direct(oauth_r_t * oreq, http_response * response)
{
	int c = 0, m, q_idx, ret, mret = 1, needmore;
	struct http_roundtripper rt;
	size_t req_size, len, req_pos;
	char request[8192];
#ifdef macintosh
	mactcp_conn mc;
#else
	int server_fd;
#endif
	uint16_t port;
	url_t murl = { 0 };

	req_size = sizeof(request);

	url_parse(oreq->url, &murl);

	snprintf(request, req_size, "%s /%s", oreq->method, murl.path);
	if(murl.query_string) {
		strlcat(request, murl.query_string, req_size);

		/*
		 * the '?' has already been added, so use
		 * '&' from now on 
		 */
		q_idx = 1;
	} else
		q_idx = 0;

	if (oreq->qstring_params)
		lc_list_foreach_v(oreq->qstring_params,
				  (lc_foreachfn_v_t) print_query_str,
				  request, req_size, &q_idx);

	req_pos = strlen(request);

	snprintf(request + req_pos, req_size - req_pos, " HTTP/1.1\n"
		 "Content-Length: %lu\nHost: ",
		 ((oreq->body != NULL) ? strlen(oreq->body) : 0L));

	strlcat(request, murl.hostname, req_size);
	strlcat(request, "\nAccept: ", req_size);
	strlcat(request, oreq->accept_types, req_size);
	strlcat(request, "\nConnection: close\n"
		USER_AGENT_STR
		"Content-Type: application/x-www-form-urlencoded\n", req_size);

	m = lc_list_getsize(oreq->oauth_params);
	lc_list_foreach_v(oreq->oauth_params,
			  (lc_foreachfn_v_t) print_oauth_params, m, &c,
			  request, req_size);

#ifdef HTTP_DEBUG
	printf("%s\n", request);
#endif

	strlcat(request, "\n", req_size);

	if(murl.port)
		port = murl.port;
	else {
		if(strcmp(murl.protocol, "https") == 0)
			port = 443;
		else
			port = 80;
	}

#ifdef macintosh
	ret = mactcp_connect(&mi, &mc, murl.hostname, port);
#else
	ret = net_connect(&server_fd, murl.hostname, port);
#endif

	url_free(&murl);

	if (ret != 0) {
		printf("could not connect\n");
		mret = 0;
		goto finish;
	}

	ssl_session_reset(oreq->ssl);
#ifdef macintosh
	ssl_set_bio(oreq->ssl, mactcp_recv, &mc, mactcp_send, &mc);
#else
	ssl_set_bio(oreq->ssl, net_recv, &server_fd, net_send, &server_fd);
#endif

#ifdef HTTP_DEBUG
	printf("Handshake init\n");
#endif

	ret = ssl_handshake(oreq->ssl);
	if (ret != 0) {
		printf("SSL handshake failed\n");
		mret = 0;
		goto finish;
	}
#ifdef HTTP_DEBUG
	printf("\nHandshake finish\n");
#endif

	ret = ssl_write(oreq->ssl, (unsigned char *)request, strlen(request));

	response->code = 0;
	response->body = NULL;
	response->header = NULL;

	http_init(&rt, responseFuncs, response);
	needmore = 1;

	do {
		const char *data = request;

		len = sizeof(request) - 1;
		ret = ssl_read(oreq->ssl, (unsigned char *)request, len);

		if (ret == POLARSSL_ERR_NET_WANT_READ ||
		    ret == POLARSSL_ERR_NET_WANT_WRITE)
			continue;

		if (ret == POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY)
			break;

		if (ret <= 0) {
			printf("read failed: ssl_read returned %d\n", ret);
			http_free(&rt);
			mret = 0;
			goto finish;
		}

		while (needmore && ret) {
			int nread;

			needmore = http_data(&rt, data, ret, &nread);

			ret -= nread;
			data += nread;
		}
	}
	while (1);

	http_free(&rt);

	ssl_close_notify(oreq->ssl);
 finish:
#ifdef macintosh
	mactcp_close(&mc);
#else
	net_close(server_fd);
#endif
	return mret;
}

#ifdef LP_SSL_PROXY
/*
 * Use a proxy that just relays unencrypted HTTP to HTTPS,
 * using e.g. socat on another machine
 *
 * Useful on old 68k Macs where polarssl is a bit too slow
 */
/* PROTO */
int send_signed_https_proxy(oauth_r_t * oreq, http_response * response)
{
	int c = 0, m, q_idx, ret, mret = 1, needmore;
#ifdef macintosh
	mactcp_conn mc;
#else
	int server_fd;
#endif

	char *tptr1, *tptr2, *murl;
	char request[8192];
	size_t req_size, len, req_pos;

	struct http_roundtripper rt;

	req_size = sizeof(request);

	/*
	 * well, this is _one_ way to parse an URL
	 *
	 * XXX need to add support for port numbers and protocol
	 */
	murl = strdup(oreq->url);

	tptr1 = strstr(murl, "://") + 3;
	tptr2 = strchr(tptr1, '/');

	snprintf(request, req_size, "%s %s", oreq->method, tptr2);
	if (oreq->qstring_params) {
		q_idx = 0;
		lc_list_foreach_v(oreq->qstring_params,
				  (lc_foreachfn_v_t) print_query_str,
				  request, req_size, &q_idx);
	}

	req_pos = strlen(request);

	snprintf(request + req_pos, req_size - req_pos, " HTTP/1.1\n"
		 "Content-Length: %lu\nHost: ",
		 ((oreq->body != NULL) ? strlen(oreq->body) : 0L));

	*tptr2 = 0;
	strlcat(request, tptr1, req_size);
	strlcat(request, "\nAccept: ", req_size);
	strlcat(request, oreq->accept_types, req_size);
	strlcat(request, "\nConnection: close\n"
		USER_AGENT_STR
		"Content-Type: application/x-www-form-urlencoded\n", req_size);

	m = lc_list_getsize(oreq->oauth_params);
	lc_list_foreach_v(oreq->oauth_params,
			  (lc_foreachfn_v_t) print_oauth_params, m, &c,
			  request, req_size);

#ifdef HTTP_DEBUG
	printf("%s\n", request);
#endif

	strlcat(request, "\n", req_size);

#ifdef macintosh
	ret = mactcp_connect(&mi, &mc, LP_SSL_PROXY_HOST, LP_SSL_PROXY_PORT);
#else
	ret = net_connect(&server_fd, LP_SSL_PROXY_HOST, LP_SSL_PROXY_PORT);
#endif

	free(murl);

	if (ret != 0) {
		printf("could not connect\n");
		mret = 0;
		goto finish;
	}
#ifdef macintosh
	ret = mactcp_send(&mc, (unsigned char *)request, strlen(request));
#else
	ret = net_send(&server_fd, (unsigned char *)request, strlen(request));
#endif

	response->code = 0;
	response->body = NULL;
	response->header = NULL;

	http_init(&rt, responseFuncs, response);
	needmore = 1;

	do {
		const char *data = request;

		len = sizeof(request) - 1;
#ifdef macintosh
		ret = mactcp_read(&mc, (unsigned char *)request, len);
#else
		ret = net_recv(&server_fd, (unsigned char *)request, len);
#endif

		if (ret == POLARSSL_ERR_NET_WANT_READ ||
		    ret == POLARSSL_ERR_NET_WANT_WRITE)
			continue;

		if (ret == POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY)
			break;

		if (ret < 0) {
			printf("read failed: ssl_read returned %d\n", ret);
			http_free(&rt);
			mret = 0;
			goto finish;
		}

		if (ret == 0) {
			http_free(&rt);
			goto finish;
		}

		while (needmore && ret) {
			int nread;

			needmore = http_data(&rt, data, ret, &nread);

			ret -= nread;
			data += nread;
		}
	}
	while (1);

	http_free(&rt);

 finish:
#ifdef macintosh
	mactcp_close(&mc);
#else
	net_close(server_fd);
#endif
	return mret;
}
#endif
