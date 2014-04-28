#include "libpummarola.h"

extern oauth_r_t oreq;
#ifdef LP_SSL
extern ssl_context ssl;
#endif

char error[1024];

#define print_spaces for(i = 0; i < it*2; i++) putchar(' ')

/* PROTO */
void
json_show(json_value *mjv, int it)
{
    int x, i;

    switch(mjv->type) {
	case json_object:
		print_spaces;
		printf("obj=object, length=%d\n", mjv->u.object.length);
		for(x = 0; x < mjv->u.object.length; x++) {
			print_spaces;
			printf("object num=%d, name='%s'\n", x, mjv->u.object.values[x].name);
			json_show(mjv->u.object.values[x].value, it+1);
		}
		break;
	case json_array:
		print_spaces;
		printf("obj=array, length=%d\n", mjv->u.array.length);
		for(x = 0; x < mjv->u.array.length; x++)
			json_show(mjv->u.array.values[x], it+1);
		break;
	case json_integer:
		print_spaces;
		printf("obj=integer, value=%ld\n", mjv->u.integer);
		break;
	case json_double:
		print_spaces;
		printf("obj=double, value=%lf\n", mjv->u.dbl);
		break;
	case json_string:
		print_spaces;
		printf("obj=string, len=%d, value='%s'\n", mjv->u.string.length, mjv->u.string.ptr);
		break;
	case json_boolean:
		print_spaces;
		printf("obj=bool, value=%d\n", mjv->u.boolean);
    }
}

/* PROTO */
void
get_user_timeline(lph_t *handle, char *user)
{
    lc_list_t oauth_plist;
    lc_list_t qstring_plist;
    http_response response;
    json_settings settings = { 0 };

    json_value *jv;
    int ret, i, j;

    char *tweet_text;
    char *tweet_date;

    oauth_plist = oauth_prepare(handle->ostate);
    qstring_plist = lc_list_create((lc_createfn_t)param_list_create,
                            (lc_destroyfn_t)param_list_destroy,
                            (lc_comparefn_t)kv_t_cmp);

    INSERT_KV_T(qstring_plist, "screen_name", user);
    INSERT_KV_T(qstring_plist, "trim_user", "true");
    INSERT_KV_T(qstring_plist, "count", "5");
    INSERT_KV_T(qstring_plist, "include_rts", "true");

    oreq.state = handle->ostate;
    oreq.ssl = &ssl;
    oreq.method = "GET";
    oreq.url = "https://api.twitter.com/1.1/statuses/user_timeline.json";
    oreq.accept_types = "*/*";
    oreq.body = NULL;
    oreq.oauth_params = oauth_plist;
    oreq.qstring_params = qstring_plist;

    oauth_sign(&oreq);
    ret = send_signed_https(&oreq, &response);
    oauth_free(&oreq);

	if(ret == 0) {
		printf("HTTP send failed\n");
		return;
	}
	
    printf("Response: %d\n", response.code);

    jv = json_parse_ex(&settings, response.body, response.body_len, error);
    free(response.body);
    lc_list_destroy(response.header);

	if(jv == 0) {
		printf("parse failed: %s\n", error);
		goto jexit;
	}
	
    /* 
     * parse through a list of tweets.
     *
     * each tweet is an object with a bunch
     * of parameters
     */
    if(jv->type != json_array) {
	    printf("error: expecting array, got %d\n", jv->type);
	    goto jexit;
    }

    for(i = 0; i < jv->u.array.length; i++) {
	    json_value *tojv = jv->u.array.values[i];
	    tweet_text = NULL;
	    tweet_date = NULL;

	    for(j = 0; j < tojv->u.object.length; j++) {
		    char *name = tojv->u.object.values[j].name;
		    json_value *ttjv = tojv->u.object.values[j].value;

		    if(strcmp(name, "text") == 0)
			    tweet_text = ttjv->u.string.ptr;
		    if(strcmp(name, "created_at") == 0)
			    tweet_date = ttjv->u.string.ptr;
	    }

	    if(tweet_text && tweet_date) {
		    printf("@%s\t%s\n\n%s\n\n", user, tweet_date,
				    tweet_text);
	    }
    }

jexit:
    json_value_free(jv);
}
