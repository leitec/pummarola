#include <libpummarola/libpummarola.h>

#include "oauth_secrets.h"

void
print_tweet(tweet_t *t)
{
	printf("\n%s\t%s\n\n%s\n\n",
			t->name,
			t->date,
			t->text);
}

int main(void)
{
	lph_t *lph;
	char *url, *token, *token_secret;
	char buf[256];
	FILE *f;
	int count = 5;
	lc_list_t tweets;
	tweet_t tw;

	lph = libpummarola_init(OAUTH_CONSUMER_KEY, OAUTH_CONSUMER_SECRET);

	if ((f = fopen("secrets", "r")) == NULL) {
		url = lp_pin_auth_begin(lph);
		if (url) {
			printf("*** visit %s to authorize client ***\n", url);
			free(url);
		}

		printf("enter PIN: ");
		fflush(stdout);
		if (gets(buf) != NULL) {
			url = lp_pin_auth_finish(lph, buf);
			if (url) {
				printf
				    ("*** successfully authorized for user '%s' ***\n",
				     url);
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

	if(lph->name && lph->screen_name)
		printf("\nPummarola: running as %s (@%s)\n\n",
				lph->name, lph->screen_name);

	for(;;) {
		printf("Pummarola> ");
		fflush(stdout);
		gets(buf);

		if(strncmp(buf, "get ", 4) == 0) {
			lp_timeline_get_user(lph, &tweets, buf+4, count);
			lc_list_foreach(tweets,(lc_foreachfn_t)print_tweet);
			lc_list_destroy(tweets);
		} else if(strncmp(buf, "home", 4) == 0) {
			lp_timeline_get_home(lph, &tweets, count);
			lc_list_foreach(tweets,(lc_foreachfn_t)print_tweet);
			lc_list_destroy(tweets);
		} else if(strncmp(buf, "tweet ", 6) == 0) {
			lp_tweet_send(lph, &tw, buf+6);
			print_tweet(&tw);
		} else if(strncmp(buf, "count ", 6) == 0) {
			count = atoi(buf+6);
			printf("Count set to %d\n", count);
		} else if(strncmp(buf, "quit", 4) == 0) {
			break;
		}
	}

	libpummarola_destroy(lph);
	return 0;
}
