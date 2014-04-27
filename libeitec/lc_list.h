/**
 ** PRINCIPLE:
 **
 ** Keep a list header of sorts in a lc_list_t. This is not
 ** a linked list itself, but rather contains pointers to
 ** the head and tail of the list, and the number of items
 ** in the list. The head and tail are lc_node_t types, which
 ** are doubly linked.
 **
 ** Each lc_node_t contains a pointer to the actual item of
 ** data. A pointer to this is what is returned when a user
 ** looks up an entry in the list. As mentioned before, this
 ** is the actual linked list portion of things, with next
 ** and prev forming the links.
 **
 ** Because these contain no information about the list itself,
 ** we don't have to worry about copying values (such as head, tail)
 ** between lc_node_t types.
 **/

#ifndef __LIBEITEC_LIST_H
#define __LIBEITEC_LIST_H

typedef size_t lc_size_t;
typedef void *lc_item_t;

/*
 * We'd be passing two items to compare.
 */
typedef int (*lc_comparefn_t) (lc_item_t, lc_item_t);

/*
 * We'd just be passing the item to create or destroy here.
 */
typedef lc_item_t (*lc_createfn_t) (lc_item_t);
typedef void (*lc_destroyfn_t) (lc_item_t);
typedef void (*lc_foreachfn_t) (lc_item_t);
typedef void (*lc_foreachfn_v_t) (lc_item_t, va_list);

typedef struct LC_Node
{
    lc_item_t item;

    struct LC_Node *next;
    struct LC_Node *prev;
} *lc_node_t;

typedef struct LC_List
{
    lc_node_t head;
    lc_node_t tail;

    lc_size_t size;

    lc_createfn_t create_fn;
    lc_destroyfn_t destroy_fn;
    lc_comparefn_t compare_fn;
} *lc_list_t;

#define LC_LIST_HEAD(x) x->head
#define LC_LIST_TAIL(x) x->tail
#endif
