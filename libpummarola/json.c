#include "libpummarola.h"

extern char error[1024];

#define print_spaces for(i = 0; i < it*2; i++) putchar(' ')

/* PROTO */
void json_show(json_value * mjv, int it)
{
	int x, i;

	switch (mjv->type) {
	case json_object:
		print_spaces;
		printf("obj=object, length=%d\n", mjv->u.object.length);
		for (x = 0; x < mjv->u.object.length; x++) {
			print_spaces;
			printf("object num=%d, name='%s'\n", x,
			       mjv->u.object.values[x].name);
			json_show(mjv->u.object.values[x].value, it + 1);
		}
		break;
	case json_array:
		print_spaces;
		printf("obj=array, length=%d\n", mjv->u.array.length);
		for (x = 0; x < mjv->u.array.length; x++)
			json_show(mjv->u.array.values[x], it + 1);
		break;
	case json_integer:
		print_spaces;
		printf("obj=integer, value=%d\n", mjv->u.integer);
		break;
	case json_double:
		print_spaces;
		printf("obj=double, value=%lf\n", mjv->u.dbl);
		break;
	case json_string:
		print_spaces;
		printf("obj=string, len=%d, value='%s'\n",
		       mjv->u.string.length, mjv->u.string.ptr);
		break;
	case json_boolean:
		print_spaces;
		printf("obj=bool, value=%d\n", mjv->u.boolean);
		break;
	case json_none:
		print_spaces;
		printf("obj=none\n");
		break;
	case json_null:
		print_spaces;
		printf("obj=NULL\n");
		break;
	}
}

/* PROTO */
json_value *jv_obj_key(json_value * jv, const char *key)
{
	int i;

	if (jv->type != json_object)
		return NULL;

	for (i = 0; i < jv->u.object.length; i++)
		if (strcmp(jv->u.object.values[i].name, key) == 0)
			return jv->u.object.values[i].value;

	return NULL;
}

/* PROTO */
char *jv_obj_key_str(json_value * jv, const char *key)
{
	json_value *jo = jv_obj_key(jv, key);

	if (jo)
		return jo->u.string.ptr;
	else
		return NULL;
}
