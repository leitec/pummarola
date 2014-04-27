#include <libpummarola/libpummarola.h>

#include "oauth_secrets.h"

int main(void)
{
    lph_t *lph;
    char *url, *token, *token_secret;
    char buf[256];
    FILE *f;

    lph = libpummarola_init(OAUTH_CONSUMER_KEY, OAUTH_CONSUMER_SECRET);

    if((f = fopen("secrets", "r")) == NULL) {
        url = twitter_pin_auth_begin(lph);
        if(url) {
            printf("*** visit %s to authorize client ***\n", url);
            free(url);
        }

        printf("enter PIN: ");
        fflush(stdout);
        if(gets(buf) != NULL) {
            url = twitter_pin_auth_finish(lph, buf);
            if(url) {
                printf("*** successfully authorized for user '%s' ***\n", url);
                free(url);
            }
        }

        oauth_get_token(lph->ostate, &token, &token_secret);
        f = fopen("secrets", "w");
        fprintf(f, "%s\n%s\n", token, token_secret);
        fclose(f);
    } else {
        fgets(buf, sizeof(buf), f);
        buf[strlen(buf)-1] = '\0';
        token = strdup(buf);
        fgets(buf, sizeof(buf), f);
        buf[strlen(buf)-1] = '\0';
        token_secret = strdup(buf);

        oauth_set_token(lph->ostate, token, token_secret);
        free(token);
        free(token_secret);
        fclose(f);
    }

    printf("Enter a twitter ID: ");
    fflush(stdout);
    gets(buf);

    printf("\n");

    get_user_timeline(lph, buf);
    libpummarola_destroy(lph);
    return 0;
}
