#define API_URL_BASE "https://api.twitter.com"

/* auth related */
#define REQUEST_TOKEN_URL API_URL_BASE "/oauth/request_token"
#define AUTHORIZE_URL API_URL_BASE "/oauth/authorize"
#define ACCESS_TOKEN_URL API_URL_BASE "/oauth/access_token"
#define VERIFY_CREDS_URL API_URL_BASE "/1.1/account/verify_credentials.json"

/* user related */
#define USER_TIMELINE_URL API_URL_BASE "/1.1/statuses/user_timeline.json"
#define HOME_TIMELINE_URL API_URL_BASE "/1.1/statuses/home_timeline.json"

typedef struct _url {
	char *protocol;
	char *hostname;
	uint16_t port;
	char *path;
	char *query_string;
} url_t;
