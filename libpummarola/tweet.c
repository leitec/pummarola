#include "libpummarola.h"

extern oauth_r_t oreq;
extern ssl_context ssl;
extern char error[1024];

int lpi_tweet_get(json_value *obj, tweet_t *tw)
{
	json_value *sub;

	sub = jv_obj_key(obj, "user");
	if(sub) {
		tw->name = strdup(jv_obj_key_str(sub, "name"));
		tw->screen_name = strdup(jv_obj_key_str(sub, "screen_name"));
	} else {
		return -1;
	}

	tw->text = strdup(jv_obj_key_str(obj, "text"));
	tw->date = strdup(jv_obj_key_str(obj, "created_at"));

	return 1;
}

void lpi_tweet_free(tweet_t *tw)
{
	if(tw->name)
		free(tw->name);
	if(tw->screen_name)
		free(tw->screen_name);
	if(tw->text)
		free(tw->text);
	if(tw->date)
		free(tw->date);

	free(tw);
}

int lp_tweet_send(lph_t *handle, tweet_t *tw, char *text)
{
	lc_list_t oauth_plist, body_plist;
	http_response response;
	json_settings settings = { 0 };

	json_value *jv;
	int ret;

	oauth_plist = oauth_prepare(handle->ostate);
	body_plist = lc_list_create((lc_createfn_t) param_list_create,
				       (lc_destroyfn_t) param_list_destroy,
				       (lc_comparefn_t) kv_t_cmp);

	INSERT_KV_T(body_plist, "status", text);

	memset(&oreq, 0, sizeof(oreq));
	oreq.state = handle->ostate;
	oreq.ssl = &ssl;
	oreq.method = "POST";
	oreq.url = STATUS_UPDATE_URL;
	oreq.accept_types = "*/*";
	oreq.body = NULL;
	oreq.body_params = body_plist;
	oreq.oauth_params = oauth_plist;
	oreq.qstring_params = NULL;

	oauth_sign(&oreq);
	ret = https_send(&oreq, &response);
	oauth_free(&oreq);

	if (!ret)
		goto error;

	if (response.code != 200)
		goto error;

	jv = json_parse_ex(&settings, response.body,
					response.body_len, error);
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
	lpi_tweet_get(jv, tw);

	json_value_free(jv);
	return 1;
 jexit:
	json_value_free(jv);
 error:
	return 0;
}
