#ifndef OAUTH_H
#define OAUTH_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef macintosh
#include "libcompat.h"
#else
#include <sys/types.h>
#endif

#include <libeitec/libeitec.h>

#include <polarssl/config.h>
#include <polarssl/base64.h>
#include <polarssl/sha1.h>
#include <polarssl/net.h>
#include <polarssl/ssl.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>
#include <polarssl/certs.h>
#include <polarssl/havege.h>

#ifdef macintosh
#include <polarssl/net_mactcp.h>
#endif

#include "kv.h"
#include "strl.h"

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

enum {
    OAUTH_SIG_HMAC_SHA1
};

enum {
    OAUTH_VERSION_1_0
};

typedef struct OAuthState {
    int version;
    int sig_method;
    char *oauth_consumer_key;
    char *oauth_consumer_secret;

    char *oauth_token;
    char *oauth_token_secret;

    char *signing_key;
} oauth_s_t;

typedef struct OAuthRequest {
    oauth_s_t *state;
    ssl_context *ssl;
    const char *method;
    const char *url;
    const char *accept_types;
    const char *body;
    lc_list_t headers;
    lc_list_t body_params;
    lc_list_t oauth_params;
    lc_list_t qstring_params;
    http_response *response;
} oauth_r_t;

#include "oauth_protos.h"

#define INSERT_KV_T(X,Y,Z) { kv_t *nkvt = make_kv_t(Y, Z); \
    lc_list_insertfirst(X, (lc_item_t)nkvt); \
    free(nkvt); }

#define FIND_KEY(X, Y) lc_list_finditem_custom(X, Y, (lc_comparefn_t)kv_t_cmp_i)

#ifndef va_copy
#define va_copy(x,y) ((x) = (y))
#endif

#endif
