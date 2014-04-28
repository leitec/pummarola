#include "libeitec.h"

typedef struct keyvalue {
	char *key;
	char *value;
} kv_t;

int item_cmp(kv_t * a, kv_t * b)
{
	return strcmp(a->key, b->key);
}

void print_kv_t(kv_t * i)
{
	printf("%s=%s\n", i->key, i->value);
}

void print_item(lc_item_t item)
{
	printf("%s\n", (char *)item);
}

int main(void)
{
	lc_list_t mylist;

	kv_t kv = { "method", "POST" };
	kv_t kv2 = { "status", "test" };
	kv_t kv3 = { "broken", "false" };

	char *item0 = "Zero";
	char *item1 = "One";
	char *item2 = "Two";
	char *item3 = "Three";
	char *newitem = "Four";

	lc_md5state_t state;
	lc_md5byte_t digest[16];
	char *message =
	    "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
	int i;

	printf("*** Testing linked lists...\n");

	mylist =
	    lc_list_create((lc_createfn_t) strdup, (lc_destroyfn_t) free,
			   (lc_comparefn_t) strcmp);

	lc_list_insertfirst(mylist, (lc_item_t) item0);
	lc_list_insertlast(mylist, (lc_item_t) item2);
	lc_list_insertlast(mylist, (lc_item_t) item3);

	lc_list_insertafter(mylist, (lc_item_t) item3, (lc_item_t) newitem);
	lc_list_insertbefore(mylist, (lc_item_t) item2, (lc_item_t) item1);

	lc_list_foreach(mylist, (lc_foreachfn_t) print_item);
	printf("\n");
	lc_list_foreach_rev(mylist, (lc_foreachfn_t) print_item);

	printf("list has %d items.\n", (int)lc_list_getsize(mylist));

	lc_list_delete_byitem(mylist, (lc_item_t) newitem);

	lc_list_foreach(mylist, (lc_foreachfn_t) print_item);

	printf("\nList has %d items.\n", (int)lc_list_getsize(mylist));

	lc_list_destroy(mylist);

	printf("*** Done.\n");
	printf("*** Testing MD5 module...\n");

	lc_byte_getendian();

	lc_md5_init(&state);
	lc_md5_append(&state, (lc_md5byte_t *) message,
		      (lc_size_t) strlen(message));
	lc_md5_finish(&state, digest);

	for (i = 0; i < 16; i++) {
		printf("%02x", digest[i]);
	}

	printf("\n*** Done.\n");

	mylist =
	    lc_list_create((lc_createfn_t) NULL, (lc_destroyfn_t) NULL,
			   (lc_comparefn_t) item_cmp);
	lc_list_insert_ordered(mylist, (lc_item_t) & kv3);
	lc_list_insert_ordered(mylist, (lc_item_t) & kv2);
	lc_list_insert_ordered(mylist, (lc_item_t) & kv);

	lc_list_foreach(mylist, (lc_foreachfn_t) print_kv_t);

	lc_list_destroy(mylist);

	return 0;
}
