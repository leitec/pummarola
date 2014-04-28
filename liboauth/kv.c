#include "oauth.h"

/* PROTO */
kv_t *make_kv_t(char *key, char *value)
{
	kv_t *n = malloc(sizeof(struct KeyValue));

	if (n != NULL) {
		n->key = key;
		n->value = value;
	}

	return n;
}

/* PROTO */
int kv_t_cmp(lc_item_t a, lc_item_t b)
{
	return strcmp(((kv_t *) a)->key, ((kv_t *) b)->key);
}

/*
 * that order is due to how it is in lc_list_findnode
 */
/* PROTO */
int kv_t_cmp_i(const char *key, lc_item_t a)
{
	return strcmp(((kv_t *) a)->key, key);
}

/* PROTO */
void print_kv_t(lc_item_t i)
{
	printf("%s=%s\n", ((kv_t *) i)->key, ((kv_t *) i)->value);
}

/* PROTO */
lc_item_t param_list_create(lc_item_t i)
{
	kv_t *a = (kv_t *) i;
	kv_t *n = malloc(sizeof(struct KeyValue));

	if (n != NULL) {
		n->key = strdup(a->key);
		n->value = strdup(a->value);
	}

	return n;
}

/* PROTO */
lc_item_t param_list_create_urldecode(lc_item_t i)
{
	kv_t *a = (kv_t *) i;
	kv_t *n = malloc(sizeof(struct KeyValue));

	if (n != NULL) {
		n->key = (char *)urldecode(a->key);
		n->value = (char *)urldecode(a->value);
	}

	return n;
}

/* PROTO */
lc_item_t param_list_create_nodup(lc_item_t i)
{
	kv_t *a = (kv_t *) i;
	kv_t *n = malloc(sizeof(struct KeyValue));

	if (n != NULL) {
		n->key = a->key;
		n->value = a->value;
	}

	return n;
}

/* PROTO */
void param_list_destroy(lc_item_t i)
{
	kv_t *a = (kv_t *) i;

	free(a->key);
	free(a->value);
	free(a);
}

/* PROTO */
lc_item_t final_list_create(lc_item_t i)
{
	kv_t *kni, *ki;

	ki = (kv_t *) i;
	kni = malloc(sizeof(struct KeyValue));

	if (kni != NULL) {
		kni->key = urlencode((unsigned char *)ki->key, strlen(ki->key));
		kni->value =
		    urlencode((unsigned char *)ki->value, strlen(ki->value));
	}

	return kni;
}

/* PROTO */
void final_list_free(lc_item_t i)
{
	kv_t *kv = (kv_t *) i;

	free(kv->key);
	free(kv->value);
	free(kv);
}

/* PROTO */
void final_list_iterator(lc_item_t i, va_list ap)
{
	va_list ap2;
	lc_list_t fl;

	va_copy(ap2, ap);
	fl = va_arg(ap2, lc_list_t);
	va_end(ap2);

	lc_list_insert_ordered(fl, i);
}
