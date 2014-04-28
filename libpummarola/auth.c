#include "libpummarola.h"

extern oauth_r_t oreq;
#ifdef LP_SSL
extern ssl_context ssl;
#endif

extern char error[1024];

/* PROTO */
char *lp_pin_auth_begin(lph_t * handle)
{
	lc_list_t oauth_plist, qstring_plist;
	http_response response;
	char *returl = NULL;
	size_t returl_size;

	oauth_plist = oauth_prepare(handle->ostate);
	INSERT_KV_T(oauth_plist, "oauth_callback", "oob");

	memset(&oreq, 0, sizeof(oreq));
	oreq.state = handle->ostate;
	oreq.ssl = &ssl;
	oreq.method = "POST";
	oreq.url = REQUEST_TOKEN_URL;
	oreq.accept_types = "*/*";
	oreq.body = NULL;
	oreq.oauth_params = oauth_plist;
	oreq.qstring_params = NULL;

	oauth_sign(&oreq);
	send_signed_https(&oreq, &response);
	oauth_free(&oreq);

	if (response.code == 200) {
		kv_t *t, *ts;

		qstring_plist = param_list_from_qstring(response.body);

		t = FIND_KEY(qstring_plist, "oauth_token");
		ts = FIND_KEY(qstring_plist, "oauth_token_secret");
		if (t && ts) {
			oauth_set_token(handle->ostate, t->value, ts->value);

			returl_size =
			    strlen(AUTHORIZE_URL) + 1 +
			    strlen(t->key) + 1 + strlen(t->value) + 1;
			returl = malloc(returl_size);
			snprintf(returl, returl_size, "%s?%s=%s",
				 AUTHORIZE_URL, t->key, t->value);
		} else
			printf
			    ("didn't get oauth_token and/or oauth_token_secret");

		lc_list_destroy(qstring_plist);
	}

	lc_list_destroy(response.header);
	free(response.body);
	return returl;
}

/* PROTO */
char *lp_pin_auth_finish(lph_t * handle, const char *pin)
{
	lc_list_t oauth_plist, qstring_plist;
	http_response response;
	char *retname = NULL;

	oauth_plist = oauth_prepare(handle->ostate);
	INSERT_KV_T(oauth_plist, (char *)"oauth_verifier", (char *)pin);

	memset(&oreq, 0, sizeof(oreq));
	oreq.state = handle->ostate;
	oreq.ssl = &ssl;
	oreq.method = "POST";
	oreq.url = ACCESS_TOKEN_URL;
	oreq.accept_types = "*/*";
	oreq.body = NULL;
	oreq.oauth_params = oauth_plist;
	oreq.qstring_params = NULL;

	oauth_sign(&oreq);
	send_signed_https(&oreq, &response);
	oauth_free(&oreq);

	if (response.code == 200) {
		kv_t *t, *ts, *sn, *uid;

		qstring_plist = param_list_from_qstring(response.body);

		t = FIND_KEY(qstring_plist, "oauth_token");
		ts = FIND_KEY(qstring_plist, "oauth_token_secret");
		if (t && ts)
			oauth_set_token(handle->ostate, t->value, ts->value);
		else
			printf
			    ("didn't get oauth_token and/or oauth_token_secret");

		sn = FIND_KEY(qstring_plist, "screen_name");
		if (sn) {
			handle->screen_name = strdup(sn->value);
			retname = handle->screen_name;
		}
		uid = FIND_KEY(qstring_plist, "user_id");
		if (uid)
			handle->user_id = atoi(uid->value);

		lc_list_destroy(qstring_plist);
	}

	lc_list_destroy(response.header);
	free(response.body);

	return retname;
}

/* PROTO */
void lp_verify_credentials(lph_t * handle)
{
	lc_list_t oauth_plist;
	http_response response;
	json_settings settings = { 0 };

	json_value *jv;
	int i;

	char *user_name = NULL;
	char *user_sn = NULL;

	oauth_plist = oauth_prepare(handle->ostate);

	memset(&oreq, 0, sizeof(oreq));
	oreq.state = handle->ostate;
	oreq.ssl = &ssl;
	oreq.method = "GET";
	oreq.url = VERIFY_CREDS_URL;
	oreq.accept_types = "*/*";
	oreq.body = NULL;
	oreq.oauth_params = oauth_plist;
	oreq.qstring_params = NULL;

	oauth_sign(&oreq);
	send_signed_https(&oreq, &response);
	oauth_free(&oreq);
	jv = json_parse_ex(&settings, response.body, response.body_len, error);
	free(response.body);
	lc_list_destroy(response.header);

	if (jv == 0) {
		printf("parse failed: %s\n", error);
		goto jexit;
	}
	if (jv->type != json_object) {
		printf("error: expecting object, got %d\n", jv->type);
		goto jexit;
	}

	for (i = 0; i < jv->u.object.length; i++) {
		char *toname = jv->u.object.values[i].name;
		json_value *tojv = jv->u.object.values[i].value;

		if (strcmp(toname, "name") == 0)
			user_name = tojv->u.string.ptr;

		if (strcmp(toname, "screen_name") == 0)
			user_sn = tojv->u.string.ptr;
	}

	if (user_name && user_sn)
		printf("\nPummarola: running as %s (@%s)\n\n", user_name,
		       user_sn);

 jexit:
	json_value_free(jv);
}
