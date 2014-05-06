#include <libpummarola/libpummarola.h>

#include "oauth_secrets.h"

void print_tweet(tweet_t * t)
{
	printf("\n** %s\t%s\n\n%s\n\n", t->user.name, t->date, t->text);
}

int main(void)
{
	lph_t *lph;
	char *url, *token, *token_secret;
	char buf[256];
	FILE *f;
	int count = 5;
	lc_list_t tweets;
	tweet_t tw, *twp;

	lph = libpummarola_init(OAUTH_CONSUMER_KEY, OAUTH_CONSUMER_SECRET);

	if ((f = fopen("secrets", "r")) == NULL) {
		printf("Didn't find a secrets file, beginning PIN authentication.\n\n");
		url = lp_pin_auth_begin(lph);
		if (url) {
			printf("Please visit the following URL to authorize:\n%s\n", url);
			free(url);
		}

		printf("\nEnter PIN: ");
		fflush(stdout);
		fgets(buf, sizeof(buf), stdin);
		buf[strlen(buf)-1] = '\0';
		if (buf[0] != '\0') {
			url = lp_pin_auth_finish(lph, buf);
			if (url) {
				printf ("Successfully authorized user '%s'\n", url);
				free(url);
			}
		}

		oauth_get_token(lph->ostate, &token, &token_secret);
		f = fopen("secrets", "w");
		fprintf(f, "%s\n%s\n", token, token_secret);
		fclose(f);
	} else {
		fgets(buf, sizeof(buf), f);
		buf[strlen(buf) - 1] = '\0';
		token = strdup(buf);
		fgets(buf, sizeof(buf), f);
		buf[strlen(buf) - 1] = '\0';
		token_secret = strdup(buf);

		oauth_set_token(lph->ostate, token, token_secret);
		free(token);
		free(token_secret);
		fclose(f);
	}

	lp_verify_credentials(lph);

	if (lph->me.name && lph->me.screen_name)
		printf("\n%s (@%s)\n\n",
		       lph->me.name, lph->me.screen_name);
	if (lph->me.description)
		if(strlen(lph->me.description) > 0)
			printf("%s\n\n", lph->me.description);

	for (;;) {
		printf("Pummarola> ");
		fflush(stdout);
		fgets(buf, sizeof(buf), stdin);
		buf[strlen(buf)-1] = '\0';

		if (strncmp(buf, "get ", 4) == 0) {
			lp_timeline_get_user(lph, &tweets, buf + 4, count);

			twp = (tweet_t *) LC_LIST_FIRST(tweets);
			printf("\n\n%s (@%s)\n", twp->user.name,
					twp->user.screen_name);
			if(strlen(twp->user.description))
				printf("\n%s\n", twp->user.description);

			printf("\n");

			lc_list_foreach(tweets, (lc_foreachfn_t) print_tweet);
			lc_list_destroy(tweets);
		} else if (strncmp(buf, "home", 4) == 0) {
			lp_timeline_get_home(lph, &tweets, count);
			lc_list_foreach(tweets, (lc_foreachfn_t) print_tweet);
			lc_list_destroy(tweets);
		} else if (strncmp(buf, "tweet ", 6) == 0) {
			lp_tweet_send(lph, &tw, buf + 6);
			print_tweet(&tw);
		} else if (strncmp(buf, "count ", 6) == 0) {
			count = atoi(buf + 6);
			printf("Count set to %d\n", count);
		} else if (strncmp(buf, "quit", 4) == 0) {
			break;
		}
	}

	libpummarola_destroy(lph);
#ifdef macintosh
	printf("\nDone.\n");
#endif
	return 0;
}
