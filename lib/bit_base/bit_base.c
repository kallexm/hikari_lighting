#include <stdint.h>
#include <zephyr.h>

#include "bit_base.h"

struct bit *bit_create(void)
{
	struct bit *new_bit = k_malloc(sizeof(struct bit));
	__ASSERT(new != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	new_bit->type = BIT_TYPE_BASE;
	sys_dlist_init(&new_bit->out_list);
	return new_bit;
}

void bit_destroy(struct bit *bit)
{	
	sys_dnode_t *elem;
	sys_dnode_t *elem_safe;
	struct bit_out_elem *out_elem;

	SYS_DLIST_FOR_EACH_NODE_SAFE(&bit->out_list, elem, elem_safe) {
		sys_dlist_remove(elem);
		out_elem = CONTAINER_OF(elem, struct bit_out_elem, root);
		k_free(out_elem);
	}
	k_free(bit);
	return;
}

void bit_connect(struct bit *bit, struct bit *connect_to)
{
	struct bit_out_elem *new_out_elem = k_malloc(sizeof(struct bit_out_elem));
	__ASSERT(new_out_elem != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	sys_dnode_init(&new_out_elem->root);
	new_out_elem->bit = bit;
	sys_dlist_append(&connect_to->out_list, &new_out_elem->root);
	return;
}

void bit_disconnect(struct bit *bit, struct bit *disconnect_from)
{
	sys_dnode_t *elem;
	sys_dnode_t *elem_safe;
	struct bit_out_elem *out_elem;

	SYS_DLIST_FOR_EACH_NODE_SAFE(&disconnect_from->out_list, elem, elem_safe) {
		out_elem = CONTAINER_OF(elem, struct bit_out_elem, root);
		if (out_elem->bit == bit) {
			sys_dlist_remove(elem);
			k_free(out_elem);
		}
	}
	
	return;
}

static void bit_stimulate_out_bits(struct bit *bit, struct bit_event *evt) 
{
	sys_dnode_t *elem;
	struct bit_out_elem *out_elem;

	SYS_DLIST_FOR_EACH_NODE(&bit->out_list, elem) {
		out_elem = CONTAINER_OF(elem, struct bit_out_elem, root);
		bit_stimulate(out_elem->bit, evt);
	}

	return;
}

static void bit_base_stimulate(struct bit *bit, struct bit_event *evt)
{
	switch (evt->id) {
	case BIT_EVENT_RESET:
		bit_stimulate_out_bits(bit, evt);
		break;
	default:
		bit_stimulate_out_bits(bit, evt);
	}
}

void n_stimulate(struct bit *bit, struct bit_event *evt)
{	
	switch (bit->type) {
	case BIT_TYPE_BASE:
		bit_base_stimulate(bit, evt);
		break;

	case BIT_TYPE_RGB:
		//n_rgb_stimulate((n_rgb *)node, evt);
		break;
	default:
		k_oops();
	}
	
	return;
}