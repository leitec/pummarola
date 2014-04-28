#include "libeitec.h"

/**
 * @file lc_list.c
 *
 * \addtogroup lc_list
 * @{
 *
 * Relatively simple linked-list implementation.
 */

/* PROTO */
lc_list_t lc_list_alloc(void)
{
	lc_list_t newlist = (lc_list_t) malloc(sizeof(struct LC_List));

	return newlist;
}

/* PROTO */
lc_node_t lc_list_node_alloc(void)
{
	lc_node_t newnode = (lc_node_t) malloc(sizeof(struct LC_Node));

	return newnode;
}

/**
 * Creates a new linked list.
 */
/* PROTO */
lc_list_t
lc_list_create(lc_createfn_t create_fn, lc_destroyfn_t destroy_fn,
	       lc_comparefn_t compare_fn)
{
	lc_list_t newlist;

	newlist = lc_list_alloc();

	if (newlist == NULL)
		return NULL;

	newlist->head = NULL;
	newlist->tail = NULL;

	newlist->size = 0;

	newlist->create_fn = create_fn;
	newlist->destroy_fn = destroy_fn;
	newlist->compare_fn = compare_fn;

	return newlist;
}

/**
 * Deletes the entire list, destroying each node and item.
 */
/* PROTO */
void lc_list_destroy(lc_list_t list)
{
	lc_node_t trav, tmp;

	trav = list->head;

	while (trav) {
		if (list->destroy_fn)
			list->destroy_fn(trav->item);

		tmp = trav;
		trav = trav->next;

		tmp->prev = NULL;
		tmp->next = NULL;
		free(tmp);
	}

	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	list->create_fn = NULL;
	list->destroy_fn = NULL;
	list->compare_fn = NULL;

	free(list);
}

/**
 * Returns a pointer to the (first) node containing item.
 */
/* PROTO */
lc_node_t lc_list_findnode(lc_list_t list, lc_item_t item)
{
	lc_node_t trav;

	for (trav = list->head; trav != NULL; trav = trav->next)
		if (list->compare_fn(item, trav->item) == 0)
			break;

	return trav;
}

/* PROTO */
lc_item_t lc_list_finditem(lc_list_t list, lc_item_t item)
{
	lc_node_t n;

	n = lc_list_findnode(list, item);
	if (n == NULL)
		return NULL;

	return n->item;
}

/* PROTO */
lc_node_t
lc_list_findnode_custom(lc_list_t list, lc_item_t item, lc_comparefn_t cfn)
{
	lc_node_t trav;

	for (trav = list->head; trav != NULL; trav = trav->next)
		if (cfn(item, trav->item) == 0)
			break;

	return trav;
}

/* PROTO */
lc_item_t
lc_list_finditem_custom(lc_list_t list, lc_item_t item, lc_comparefn_t cfn)
{
	lc_node_t n;

	n = lc_list_findnode_custom(list, item, cfn);
	if (n == NULL)
		return NULL;

	return n->item;
}

/**
 * Deletes one node from a list. Node is referred to by a pointer.
 */
/* PROTO */
void lc_list_delete_bynode(lc_list_t list, lc_node_t node)
{
	list->destroy_fn(node->item);

	if (node == list->head && node == list->tail) {
		list->head = NULL;
		list->tail = NULL;
	} else if (node == list->head) {
		list->head->next->prev = NULL;
		list->head = list->head->next;
	} else if (node == list->tail) {
		list->tail->prev->next = NULL;
		list->tail = list->tail->prev;
	} else {
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}

	free(node);
	list->size--;
}

/**
 * Deletes one node from a list. Node is referenced to by its item.
 */
/* PROTO */
void lc_list_delete_byitem(lc_list_t list, lc_item_t item)
{
	lc_node_t node = lc_list_findnode(list, item);

	if (node != NULL)
		lc_list_delete_bynode(list, node);
}

/* PROTO */
lc_node_t lc_list_insertfirst(lc_list_t list, lc_item_t item)
{
	lc_node_t newnode;

	newnode = lc_list_node_alloc();
	if (newnode == NULL)
		return NULL;

	if (list->create_fn != NULL)
		newnode->item = list->create_fn(item);
	else
		newnode->item = item;

	newnode->prev = NULL;

	if (list->head != NULL)
		list->head->prev = newnode;

	newnode->next = list->head;
	list->head = newnode;

	if (list->tail == NULL)
		list->tail = list->head;

	list->size++;

	return newnode;
}

/* PROTO */
lc_node_t lc_list_insertlast(lc_list_t list, lc_item_t item)
{
	lc_node_t newnode;

	newnode = lc_list_node_alloc();
	if (newnode == NULL)
		return NULL;

	if (list->create_fn != NULL)
		newnode->item = list->create_fn(item);
	else
		newnode->item = item;

	newnode->next = NULL;

	if (list->tail != NULL)
		list->tail->next = newnode;

	newnode->prev = list->tail;
	list->tail = newnode;

	if (list->head == NULL)
		list->head = newnode;

	list->size++;

	return newnode;
}

/* PROTO */
lc_node_t
lc_list_insertafter(lc_list_t list, lc_item_t curitem, lc_item_t newitem)
{
	lc_node_t curnode, newnode;

	curnode = lc_list_findnode(list, curitem);

	if (curnode == NULL) {
		return lc_list_insertlast(list, newitem);
	}
	newnode = lc_list_node_alloc();
	if (newnode == NULL)
		return NULL;

	if (list->create_fn != NULL)
		newnode->item = list->create_fn(newitem);
	else
		newnode->item = newitem;

	newnode->prev = curnode;
	newnode->next = curnode->next;
	curnode->next = newnode;

	if (newnode->next != NULL)
		newnode->next->prev = newnode;
	else
		list->tail = newnode;

	list->size++;

	return newnode;
}

/* PROTO */
lc_node_t
lc_list_insertbefore_node(lc_list_t list, lc_node_t curnode, lc_item_t newitem)
{
	lc_node_t newnode;

	if (curnode == NULL)
		return lc_list_insertfirst(list, newitem);

	newnode = lc_list_node_alloc();
	if (newnode == NULL)
		return NULL;

	if (list->create_fn != NULL)
		newnode->item = list->create_fn(newitem);
	else
		newnode->item = newitem;

	newnode->prev = curnode->prev;
	newnode->next = curnode;
	curnode->prev = newnode;

	if (newnode->prev != NULL)
		newnode->prev->next = newnode;
	else
		list->head = newnode;

	list->size++;

	return newnode;
}

/* PROTO */
lc_node_t
lc_list_insertbefore(lc_list_t list, lc_item_t curitem, lc_item_t newitem)
{
	lc_node_t curnode, newnode;

	curnode = lc_list_findnode(list, curitem);

	if (curnode == NULL) {
		return lc_list_insertfirst(list, newitem);
	}
	newnode = lc_list_node_alloc();
	if (newnode == NULL)
		return NULL;

	if (list->create_fn != NULL)
		newnode->item = list->create_fn(newitem);
	else
		newnode->item = newitem;

	newnode->prev = curnode->prev;
	newnode->next = curnode;
	curnode->prev = newnode;

	if (newnode->prev != NULL)
		newnode->prev->next = newnode;
	else
		list->head = newnode;

	list->size++;

	return newnode;
}

/**
 * Inserts a node sorted by order accoding to compare_fn
 */
/* PROTO */
lc_node_t lc_list_insert_ordered(lc_list_t list, lc_item_t newitem)
{
	lc_node_t trav;

	for (trav = list->head; trav != NULL; trav = trav->next)
		if (list->compare_fn(trav->item, newitem) >= 0)
			return lc_list_insertbefore_node(list, trav, newitem);

	return lc_list_insertlast(list, newitem);
}

/* PROTO */
void lc_list_foreach(lc_list_t list, lc_foreachfn_t foreachfn)
{
	lc_node_t trav;

	for (trav = list->head; trav != NULL; trav = trav->next)
		foreachfn(trav->item);
}

/* PROTO */
void lc_list_foreach_v(lc_list_t list, lc_foreachfn_v_t foreachfn_v, ...)
{
	lc_node_t trav;
	va_list ap;

	va_start(ap, foreachfn_v);

	for (trav = list->head; trav != NULL; trav = trav->next)
		foreachfn_v(trav->item, ap);

	va_end(ap);
}

/* PROTO */
void lc_list_foreach_rev(lc_list_t list, lc_foreachfn_t foreachfn)
{
	lc_node_t trav;

	for (trav = list->tail; trav != NULL; trav = trav->prev)
		foreachfn(trav->item);
}

/* PROTO */
lc_size_t lc_list_getsize(lc_list_t list)
{
	return list->size;
}
