int lc_byte_getendian(void);
uint32_t lc_byte_4to32(uint8_t * buf);
uint8_t *lc_byte_32to4(uint32_t orig, uint8_t * buf);
lc_list_t lc_list_alloc(void);
lc_node_t lc_list_node_alloc(void);
lc_list_t lc_list_create(lc_createfn_t create_fn, lc_destroyfn_t destroy_fn,
			 lc_comparefn_t compare_fn);
void lc_list_destroy(lc_list_t list);
lc_node_t lc_list_findnode(lc_list_t list, lc_item_t item);
lc_item_t lc_list_finditem(lc_list_t list, lc_item_t item);
lc_node_t lc_list_findnode_custom(lc_list_t list, lc_item_t item,
				  lc_comparefn_t cfn);
lc_item_t lc_list_finditem_custom(lc_list_t list, lc_item_t item,
				  lc_comparefn_t cfn);
void lc_list_delete_bynode(lc_list_t list, lc_node_t node);
void lc_list_delete_byitem(lc_list_t list, lc_item_t item);
lc_node_t lc_list_insertfirst(lc_list_t list, lc_item_t item);
lc_node_t lc_list_insertlast(lc_list_t list, lc_item_t item);
lc_node_t lc_list_insertafter(lc_list_t list, lc_item_t curitem,
			      lc_item_t newitem);
lc_node_t lc_list_insertbefore_node(lc_list_t list, lc_node_t curnode,
				    lc_item_t newitem);
lc_node_t lc_list_insertbefore(lc_list_t list, lc_item_t curitem,
			       lc_item_t newitem);
lc_node_t lc_list_insert_ordered(lc_list_t list, lc_item_t newitem);
void lc_list_foreach(lc_list_t list, lc_foreachfn_t foreachfn);
void lc_list_foreach_v(lc_list_t list, lc_foreachfn_v_t foreachfn_v, ...);
void lc_list_foreach_rev(lc_list_t list, lc_foreachfn_t foreachfn);
void lc_list_foreach_rev_v(lc_list_t list, lc_foreachfn_v_t foreachfn_v, ...);
lc_size_t lc_list_getsize(lc_list_t list);
void lc_md5_init(lc_md5state_t * state);
void lc_md5_append(lc_md5state_t * state, lc_md5byte_t * message,
		   lc_size_t len);
void lc_md5_compute(lc_md5state_t * state);
void lc_md5_finish(lc_md5state_t * state, lc_md5byte_t digest[16]);
