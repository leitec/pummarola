char *lp_pin_auth_begin(lph_t * handle);
char *lp_pin_auth_finish(lph_t * handle, const char *pin);
void lp_verify_credentials(lph_t * handle);
void print_oauth_params(kv_t * i, va_list ap);
void print_extra_params(kv_t * i, va_list ap);
void print_query_str(kv_t * i, va_list ap);
int https_send_direct(oauth_r_t * oreq, http_response * response);
int https_send_proxy(oauth_r_t * oreq, http_response * response);
lph_t *libpummarola_init(const char *oauth_consumer_key,
			 const char *oauth_consumer_secret);
void libpummarola_destroy(lph_t * handle);
void json_show(json_value * mjv, int it);
void lp_get_user_timeline(lph_t * handle, char *user);
int url_parse(const char *url, url_t * components);
void url_free(url_t *);
int url_char(uint8_t * state, const char ch);
json_value *jv_obj_key(json_value * jv, const char *key);
char *jv_obj_key_str(json_value * jv, const char *key);
int lpi_tweet_get(json_value * obj, tweet_t * tweet);
void lpi_tweet_free(tweet_t * tw);
int lp_timeline_get_home(lph_t * handle, lc_list_t * tweetlist, int count);
int lp_timeline_get_user(lph_t * handle, lc_list_t * tweetlist, char *user,
			 int count);
int lp_timeline_get(lph_t * handle, lc_list_t * tweetlist, char *user,
		    int count, int type);
int lp_tweet_send(lph_t * handle, tweet_t * tw, char *text);
