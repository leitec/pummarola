#ifndef LIBPUMMAROLA_H
#define LIBPUMMAROLA_H
#define LP_SSL
/*
 * these two should include pretty much everything we need
 */
#include <libeitec/libeitec.h>
#include <liboauth/oauth.h>

#include <polarssl/ssl_cache.h>

#include <json-parser/json.h>

typedef struct libPummarolaHandle {
#ifdef macintosh
    /* going to make mactcp_inst a global */
    mactcp_conn mc;
#else
    int sock;
#endif

    oauth_s_t   *ostate;

    char *screen_name;
    int user_id;
} lph_t;

#include "lp_protos.h"
#endif
