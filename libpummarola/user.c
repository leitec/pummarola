#include "libpummarola.h"

extern oauth_r_t oreq;
#ifdef LP_SSL
extern ssl_context ssl;
#endif

char error[1024];

enum {
	TIMELINE_HOME,
	TIMELINE_USER
};

int lp_timeline_get_home(lph_t * handle, lc_list_t * tweetlist, int count)
{
	return lp_timeline_get(handle, tweetlist, NULL, count, TIMELINE_HOME);
}

int lp_timeline_get_user(lph_t * handle, lc_list_t * tweetlist, char *user,
			 int count)
{
	return lp_timeline_get(handle, tweetlist, user, count, TIMELINE_USER);
}

int lp_timeline_get(lph_t * handle, lc_list_t * tweetlist, char *user,
		    int count, int type)
{
	lc_list_t oauth_plist, qstring_plist, twl;
	http_response response;
	json_settings settings = { 0 };

	char numbuf[32];
	json_value *jv;
	tweet_t *tw;
	int ret, i;

	oauth_plist = oauth_prepare(handle->ostate);
	qstring_plist = lc_list_create((lc_createfn_t) param_list_create,
				       (lc_destroyfn_t) param_list_destroy,
				       (lc_comparefn_t) kv_t_cmp);

	snprintf(numbuf, sizeof(numbuf), "%d", count);

	INSERT_KV_T(qstring_plist, "count", numbuf);

	if (type == TIMELINE_USER) {
		INSERT_KV_T(qstring_plist, "screen_name", user);
		INSERT_KV_T(qstring_plist, "include_rts", "true");
	}

	memset(&oreq, 0, sizeof(oreq));
	oreq.state = handle->ostate;
	oreq.ssl = &ssl;
	oreq.method = "GET";
	oreq.accept_types = "*/*";
	oreq.body = NULL;
	oreq.oauth_params = oauth_plist;
	oreq.qstring_params = qstring_plist;

	if (type == TIMELINE_HOME)
		oreq.url = HOME_TIMELINE_URL;
	else
		oreq.url = USER_TIMELINE_URL;

	oauth_sign(&oreq);
	ret = https_send(&oreq, &response);
	oauth_free(&oreq);

	if (!ret)
		goto error;

	if (response.code != 200)
		goto error;

	jv = json_parse_ex(&settings, response.body, response.body_len, error);
	free(response.body);
	lc_list_destroy(response.header);

	if (jv == 0) {
		printf("parse failed: %s\n", error);
		goto jexit;
	}

	twl =
	    lc_list_create((lc_createfn_t) NULL,
			   (lc_destroyfn_t) lpi_tweet_free,
			   (lc_comparefn_t) NULL);

	/* 
	 * parse through a list of tweets.
	 *
	 * each tweet is an object with a bunch
	 * of parameters
	 */
	if (jv->type != json_array) {
		printf("error: expecting array, got %d\n", jv->type);
		goto jexit;
	}

	for (i = 0; i < jv->u.array.length; i++) {
		tw = malloc(sizeof(tweet_t));
		lpi_tweet_get(jv->u.array.values[i], tw);
		lc_list_insertlast(twl, (lc_item_t) tw);
	}

	*tweetlist = twl;
	json_value_free(jv);
	return 1;
 jexit:
	json_value_free(jv);
 error:
	return 0;
}
