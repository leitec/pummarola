#include "libpummarola.h"

int lp_tweet_get(json_value *obj, tweet_t *tw)
{
	json_value *sub;

	sub = jv_obj_key(obj, "user");
	if(sub) {
		tw->name = strdup(jv_obj_key_str(sub, "name"));
		tw->screen_name = strdup(jv_obj_key_str(sub, "screen_name"));
	} else {
		return -1;
	}

	tw->text = strdup(jv_obj_key_str(obj, "text"));
	tw->date = strdup(jv_obj_key_str(obj, "created_at"));

	return 1;
}

void lp_tweet_free(tweet_t *tw)
{
	if(tw->name)
		free(tw->name);
	if(tw->screen_name)
		free(tw->screen_name);
	if(tw->text)
		free(tw->text);
	if(tw->date)
		free(tw->date);

	free(tw);
}
