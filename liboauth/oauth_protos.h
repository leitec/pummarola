kv_t *make_kv_t(char *key, char *value);
int kv_t_cmp(lc_item_t a, lc_item_t b);
int kv_t_cmp_i(const char *key, lc_item_t a);
void print_kv_t(lc_item_t i);
lc_item_t param_list_create(lc_item_t i);
lc_item_t param_list_create_urldecode(lc_item_t i);
lc_item_t param_list_create_nodup(lc_item_t i);
void param_list_destroy(lc_item_t i);
lc_item_t final_list_create(lc_item_t i);
void final_list_free(lc_item_t i);
void final_list_iterator(lc_item_t i, va_list ap);
oauth_s_t *oauth_init(int version, int sig_method, char *oauth_consumer_key,
		      char *oauth_consumer_secret);
void oauth_destroy(oauth_s_t * state);
void oauth_free(oauth_r_t * req);
int oauth_set_token(oauth_s_t * s, char *oauth_token, char *oauth_token_secret);
void oauth_get_token(oauth_s_t * s, char **oauth_token,
		     char **oauth_token_secret);
lc_list_t oauth_prepare(oauth_s_t * s);
void make_param_str(lc_item_t * i, va_list ap);
void make_query_str(kv_t * i, va_list ap);
void make_oauth_header(kv_t * i, va_list ap);
int oauth_sign(oauth_r_t * oreq);
char *urlencode(const unsigned char *original, size_t origlen);
unsigned char *urldecode(const char *encoded);
lc_list_t param_list_from_qstring(const char *qstring);
