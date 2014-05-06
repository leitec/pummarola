#ifndef LIBPUMMAROLA_H
#define LIBPUMMAROLA_H
#define LP_SSL

/*
 * these two should include pretty much everything we need
 */
#include <libeitec/libeitec.h>
#include <liboauth/oauth.h>

#include <polarssl/ssl_cache.h>

#ifdef macintosh
#include <polarssl/net_mactcp.h>
#endif

#include <json-parser/json.h>

typedef struct libPummarolaUser {
	char *name;
	char *screen_name;
	char *description;
} lpuser_t;

typedef struct libPummarolaHandle {
#ifdef macintosh
	/* going to make mactcp_inst a global */
	mactcp_conn mc;
#else
	int sock;
#endif
	oauth_s_t *ostate;

	lpuser_t me;
	int user_id;
} lph_t;

typedef struct libPummarolaTweet {
	lpuser_t user;
	char *text;
	char *date;
} tweet_t;

/*
 * TinyHTTP
 */
#include <tinyhttp/http.h>

typedef struct HttpResponse {
	int code;
	char *body;
	size_t body_len;
	lc_list_t header;
} http_response;

#ifdef LP_SSL_PROXY
/* also define LP_SSL_PROXY_HOST and LP_SSL_PROXY_PORT */
#define https_send https_send_proxy
#else
#define https_send https_send_direct
#endif

#include "lp_url.h"

#include "lp_protos.h"
#endif
