char *lp_pin_auth_begin(lph_t * handle);
char *lp_pin_auth_finish(lph_t * handle, const char *pin);
void lp_verify_credentials(lph_t * handle);
void print_oauth_params(kv_t * i, va_list ap);
void print_extra_params(kv_t * i, va_list ap);
void print_query_str(kv_t * i, va_list ap);
int send_signed_https_direct(oauth_r_t * oreq, http_response * response);
int send_signed_https_proxy(oauth_r_t * oreq, http_response * response);
lph_t *libpummarola_init(const char *oauth_consumer_key,
			 const char *oauth_consumer_secret);
void libpummarola_destroy(lph_t * handle);
void json_show(json_value * mjv, int it);
void lp_get_user_timeline(lph_t * handle, char *user);
void lp_get_home_timeline(lph_t * handle);
int url_parse(const char *url, url_t * components);
void url_free(url_t *);
int url_char(uint8_t * state, const char ch);
json_value * jv_obj_key(json_value *jv, const char *key);
char * jv_obj_key_str(json_value *jv, const char *key);
