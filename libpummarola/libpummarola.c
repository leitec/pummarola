#include "libpummarola.h"

/*
 * since this is single-threaded and
 * every oreq parameter is set at each use,
 * it'd be redundant to have one in every
 * function.
 */
oauth_r_t oreq;

#ifdef LP_SSL
ssl_context ssl;
ssl_cache_context cache;
entropy_context entropy;
ctr_drbg_context ctr_drbg;
char *pers = "libpummarola";
#endif

#ifdef macintosh
mactcp_inst mi;
#endif

/* PROTO */
lph_t *libpummarola_init(const char *oauth_consumer_key,
			 const char *oauth_consumer_secret)
{
	lph_t *nh;
	int ret;

	nh = malloc(sizeof(lph_t));
	memset(nh, 0, sizeof(lph_t));

#ifdef macintosh
	mactcp_init(&mi);
#else
	nh->sock = -1;
#endif

	/*
	 * rand() is only used for oauth_nonce,
	 * not for SSL
	 *
	 * we should probably use random numbers
	 * from PolarSSL, though
	 */
	srand(time(NULL) * getpid());

#ifdef LP_SSL
	entropy_init(&entropy);
	if ((ret = ctr_drbg_init(&ctr_drbg, entropy_func, &entropy,
				 (unsigned char *)pers, strlen(pers))) != 0) {
		printf("ctr_drbg_init failed: %d\n", ret);
		return NULL;
	}

	memset(&ssl, 0, sizeof(ssl_context));

	ret = ssl_init(&ssl);
	if (ret != 0) {
		printf("ssl_init failed: ret=%d\n", ret);
		return NULL;
	}

	ssl_set_endpoint(&ssl, SSL_IS_CLIENT);
	ssl_set_authmode(&ssl, SSL_VERIFY_NONE);
	ssl_set_rng(&ssl, ctr_drbg_random, &ctr_drbg);
#endif

	nh->ostate = oauth_init(OAUTH_VERSION_1_0, OAUTH_SIG_HMAC_SHA1,
				(char *)oauth_consumer_key,
				(char *)oauth_consumer_secret);

	return nh;
}

/* PROTO */
void libpummarola_destroy(lph_t * handle)
{
	oauth_destroy(handle->ostate);
#ifdef LP_SSL
	ssl_free(&ssl);
	ssl_cache_free(&cache);
#endif
#ifdef macintosh
	mactcp_shutdown(&mi);
#endif
	if(handle->screen_name)
		free(handle->screen_name);
	if(handle->name)
		free(handle->name);

	free(handle);
}
