#include <stdint.h>
#include <zephyr.h>

#include "slab_base.h"

struct slab *slab_create(void)
{
	struct slab *new_slab = k_malloc(sizeof(struct slab));
	__ASSERT(new_slab != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	new_slab->type = SLAB_TYPE_BASE;
	sys_dlist_init(&new_slab->out_list);
	return new_slab;
}

void slab_destroy(struct slab *slab)
{	
	sys_dnode_t *elem;
	sys_dnode_t *elem_safe;
	struct slab_out_elem *out_elem;

	SYS_DLIST_FOR_EACH_NODE_SAFE(&slab->out_list, elem, elem_safe) {
		sys_dlist_remove(elem);
		out_elem = CONTAINER_OF(elem, struct slab_out_elem, root);
		k_free(out_elem);
	}
	k_free(slab);
	return;
}

void block_connect(struct slab *slab, struct slab *connect_to)
{
	struct slab_out_elem *new_out_elem = k_malloc(sizeof(struct slab_out_elem));
	__ASSERT(new_out_elem != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	sys_dnode_init(&new_out_elem->root);
	new_out_elem->slab = slab;
	sys_dlist_append(&connect_to->out_list, &new_out_elem->root);
	return;
}

void block_disconnect(struct slab *slab, struct slab *disconnect_from)
{
	sys_dnode_t *elem;
	sys_dnode_t *elem_safe;
	struct slab_out_elem *out_elem;

	SYS_DLIST_FOR_EACH_NODE_SAFE(&disconnect_from->out_list, elem, elem_safe) {
		out_elem = CONTAINER_OF(elem, struct slab_out_elem, root);
		if (out_elem->slab == slab) {
			sys_dlist_remove(elem);
			k_free(out_elem);
		}
	}
	
	return;
}

static void block_stimulate_out_slabs(struct slab *slab, struct slab_event *evt)
{
	sys_dnode_t *elem;
	struct slab_out_elem *out_elem;

	SYS_DLIST_FOR_EACH_NODE(&slab->out_list, elem) {
		out_elem = CONTAINER_OF(elem, struct slab_out_elem, root);
		slab_stimulate(out_elem->slab, evt);
	}

	return;
}

static void block_base_stimulate(struct slab *slab, struct slab_event *evt)
{
	switch (evt->id) {
	case SLAB_EVENT_RESET:
		slab_stimulate_out_slabs(slab, evt);
		break;
	default:
		slab_stimulate_out_slabs(slab, evt);
	}
}

void n_stimulate(struct slab *slab, struct slab_event *evt)
{	
	switch (slab->type) {
	case SLAB_TYPE_BASE:
		slab_base_stimulate(slab, evt);
		break;

	case SLAB_TYPE_RGB:
		//n_rgb_stimulate((n_rgb *)node, evt);
		break;
	default:
		k_oops();
	}
	
	return;
}