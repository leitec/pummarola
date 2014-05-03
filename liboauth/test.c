#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#ifndef macintosh
#include <libgen.h>
#endif
#include <time.h>

#include <libeitec/libeitec.h>

#include "oauth.h"
char *pers = "oauth_test";

/*
 * CodeWarrior doesn't build this when these are in main()
 * due to being >32K data in the function
 */
ssl_context ssl;
entropy_context entropy;
ctr_drbg_context ctr_drbg;
oauth_r_t oreq;

int main(int argc, char *argv[])
{
	lc_list_t oauth_plist;
	oauth_s_t *ostate;

	int ret;

	srand(time(NULL) * getpid());
	memset(&ssl, 0, sizeof(ssl_context));

	entropy_init(&entropy);
	if ((ret = ctr_drbg_init(&ctr_drbg, entropy_func, &entropy,
				 (unsigned char *)pers, strlen(pers))) != 0) {
		printf("ctr_drbg_init failed: %d\n", ret);
		return -1;
	}

	ret = ssl_init(&ssl);
	if (ret != 0) {
		printf("ssl_init failed\n");
		return -1;
	}

	ssl_set_endpoint(&ssl, SSL_IS_CLIENT);
	ssl_set_authmode(&ssl, SSL_VERIFY_NONE);
	ssl_set_rng(&ssl, ctr_drbg_random, &ctr_drbg);

	printf(">>> Initializing OAuth\n");
	
	/*
	 * these values come from Twitter's example:
	 * https://dev.twitter.com/docs/auth/authorizing-request
	 */
	ostate = oauth_init(OAUTH_VERSION_1_0, OAUTH_SIG_HMAC_SHA1,
			"xvz1evFS4wEEPTGEFPHBog", /* consumer key */
			"kAcSOqF21Fu85e7zjz7ZN2U4ZRhfV3WpwPAoE3Z7kBw");	/* consumer secret */

	oauth_set_token(ostate,
			"370773112-GmHxMAgYyLbNEtIKZeRNFsMKPR9EyMZeS9weJAEb", /* token */
			"LswwdoUaIvS8ltyTt5jkRh4J50vUPVVHtR2YPi5kE"); /* token secret */

	printf(">>> Preparing OAuth plist\n");
	oauth_plist = oauth_prepare(ostate);
	INSERT_KV_T(oauth_plist, "oauth_callback", "oob");

	printf(">>> Signing request\n");

	oreq.state = ostate;
	oreq.ssl = &ssl;
	oreq.method = "POST";
	oreq.url = "https://api.twitter.com/1/statuses/update.json";
	oreq.accept_types = "*/*";
	oreq.body = NULL;
	oreq.headers = NULL;
	oreq.body_params = lc_list_create((lc_createfn_t) param_list_create,
					  (lc_destroyfn_t) param_list_destroy,
					  (lc_comparefn_t) kv_t_cmp);
	oreq.oauth_params = oauth_plist;
	oreq.qstring_params = lc_list_create((lc_createfn_t) param_list_create,
					     (lc_destroyfn_t)
					     param_list_destroy,
					     (lc_comparefn_t) kv_t_cmp);

	INSERT_KV_T(oreq.qstring_params, "include_entities", "true");
	INSERT_KV_T(oreq.body_params, "status",
		    "Hello Ladies + Gentlemen, a signed OAuth request!");

	oauth_sign(&oreq);
	lc_list_foreach(oreq.headers, (lc_foreachfn_t) print_kv_t);

	printf("\n>>> Freeing memory structures\n");

	oauth_free(&oreq);

	oauth_destroy(ostate);

	ssl_free(&ssl);

	{
		char *or =
		    "Mr. Test P. User, tpuser@internet.com; 123-321-1029; http://internet.com/~tpuser";
		char *ue, *ud;
		ue = urlencode((unsigned char *)or, strlen(or));
		ud = (char *)urldecode(ue);

		printf(">>> Testing urlencode/decode explicitly\n");

		printf("orig: '%s'\nenc:  '%s'\ndec:  '%s'\n", or, ue, ud);
		if (strcmp(or, ud) == 0)
			printf(">>> OK; match\n");
		else
			printf(">>> Error!\n");

		free(ue);
		free(ud);
	}

	{
		lc_list_t a;

		printf(">>> Testing qstring parser explicitly\n");
		a = param_list_from_qstring
		    ("abcdefghijklmnopqrstuvwxyz012345=012345abcdefghijklmnopqrstuvwxyz");
		lc_list_foreach(a, (lc_foreachfn_t) print_kv_t);
		lc_list_destroy(a);
	}

	printf(">>> All done.\n");
	return 0;
}
