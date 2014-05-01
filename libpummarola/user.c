#include "libpummarola.h"

extern oauth_r_t oreq;
#ifdef LP_SSL
extern ssl_context ssl;
#endif

char error[1024];

/* PROTO */
void
lp_get_user_timeline(lph_t * handle, char *user)
{
	lc_list_t oauth_plist;
	lc_list_t qstring_plist;
	http_response response;
	json_settings settings = { 0 };

	json_value *jv, *curjv, *tojv;
	int ret, i;

	char *tweet_text;
	char *tweet_date;
	char *tweet_user;

	oauth_plist = oauth_prepare(handle->ostate);
	qstring_plist = lc_list_create((lc_createfn_t) param_list_create,
				       (lc_destroyfn_t) param_list_destroy,
				       (lc_comparefn_t) kv_t_cmp);

	INSERT_KV_T(qstring_plist, "screen_name", user);
	INSERT_KV_T(qstring_plist, "count", "5");
	INSERT_KV_T(qstring_plist, "include_rts", "true");

	memset(&oreq, 0, sizeof(oreq));
	oreq.state = handle->ostate;
	oreq.ssl = &ssl;
	oreq.method = "GET";
	oreq.url = USER_TIMELINE_URL;
	oreq.accept_types = "*/*";
	oreq.body = NULL;
	oreq.oauth_params = oauth_plist;
	oreq.qstring_params = qstring_plist;

	oauth_sign(&oreq);
	ret = send_signed_https(&oreq, &response);
	oauth_free(&oreq);

	if (ret == 0) {
		printf("HTTP send failed\n");
		return;
	}

	printf("Response: %d\n", response.code);

	jv = json_parse_ex(&settings, response.body, response.body_len, error);
	free(response.body);
	lc_list_destroy(response.header);

	if (jv == 0) {
		printf("parse failed: %s\n", error);
		goto jexit;
	}

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

	for(i = 0; i < jv->u.array.length; i++) {
		curjv = jv->u.array.values[i];

		if((tojv = jv_obj_key(curjv, "user")))
			tweet_user = jv_obj_key_str(tojv, "name");

		tweet_text = jv_obj_key_str(curjv, "text");
		tweet_date = jv_obj_key_str(curjv, "created_at");

		if (tweet_user && tweet_text && tweet_date) {
			printf("%s\t%s\n\n%s\n\n", tweet_user,
			       tweet_date, tweet_text);
		}
	}

 jexit:
	json_value_free(jv);
}

/* PROTO */
void
lp_get_home_timeline(lph_t * handle)
{
	lc_list_t oauth_plist;
	lc_list_t qstring_plist;
	http_response response;
	json_settings settings = { 0 };

	json_value *jv, *curjv, *tojv;
	int ret, i;

	char *tweet_text;
	char *tweet_date;
	char *tweet_user;

	oauth_plist = oauth_prepare(handle->ostate);
	qstring_plist = lc_list_create((lc_createfn_t) param_list_create,
				       (lc_destroyfn_t) param_list_destroy,
				       (lc_comparefn_t) kv_t_cmp);

	INSERT_KV_T(qstring_plist, "count", "5");

	memset(&oreq, 0, sizeof(oreq));
	oreq.state = handle->ostate;
	oreq.ssl = &ssl;
	oreq.method = "GET";
	oreq.url = HOME_TIMELINE_URL;
	oreq.accept_types = "*/*";
	oreq.body = NULL;
	oreq.oauth_params = oauth_plist;
	oreq.qstring_params = qstring_plist;

	oauth_sign(&oreq);
	ret = send_signed_https(&oreq, &response);
	oauth_free(&oreq);

	if (ret == 0) {
		printf("HTTP send failed\n");
		return;
	}

	printf("Response: %d\n", response.code);

	jv = json_parse_ex(&settings, response.body, response.body_len, error);
	free(response.body);
	lc_list_destroy(response.header);

	if (jv == 0) {
		printf("parse failed: %s\n", error);
		goto jexit;
	}

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

	for(i = 0; i < jv->u.array.length; i++) {
		curjv = jv->u.array.values[i];

		if((tojv = jv_obj_key(curjv, "user")))
			tweet_user = jv_obj_key_str(tojv, "name");

		tweet_text = jv_obj_key_str(curjv, "text");
		tweet_date = jv_obj_key_str(curjv, "created_at");

		if (tweet_user && tweet_text && tweet_date) {
			printf("%s\t%s\n\n%s\n\n", tweet_user,
			       tweet_date, tweet_text);
		}
	}

 jexit:
	json_value_free(jv);
}
