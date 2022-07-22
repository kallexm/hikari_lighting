#include <stdint.h>
#include <zephyr.h>

#include "slab.h"
#include "slab_event.h"

#include "slabs/slab_led.h"
#include "slabs/slab_delay.h"
#include "slabs/slab_glower.h"
#include "slabs/slab_hsv2rgb.h"
#include "slabs/slab_rgb2hsv.h"

struct slab_child {
	sys_dnode_t root;
	struct slab *child;
};

struct slab *slab_create(enum slab_type type, ...)
{
	struct slab *new_slab;
	va_list args;

	va_start(args, type);

	switch(type) {
	case SLAB_TYPE_LED: {
		struct rgb_value *val = va_arg(args, struct rgb_value *);
		enum led_type led_type = va_arg(args, enum led_type);
		new_slab = slab_led_create(val, led_type);
		break;
	}
	case SLAB_TYPE_DELAY: {
		uint32_t delay_periods = va_arg(args, uint32_t);
		new_slab = slab_delay_create(delay_periods);
		break;
	}
	case SLAB_TYPE_GLOWER: {
		struct slab_glower_config *conf = va_arg(args, struct slab_glower_config *);
		new_slab = slab_glower_create(conf);
		break;
	}
	case SLAB_TYPE_HSV2RGB:
		new_slab = slab_hsv2rgb_create();
		break;

	case SLAB_TYPE_RGB2HSV:
		new_slab = slab_rgb2hsv_create();
		break;

	default:
		new_slab = NULL;
		goto clean_exit;
	}

	new_slab->type = type;
	sys_dlist_init(&new_slab->childs);

clean_exit:
	va_end(args);
	return new_slab;
}

void slab_destroy(struct slab *slab)
{	
	sys_dnode_t *elem;
	sys_dnode_t *elem_safe;
	struct slab_child *child_elem;

	if (slab == NULL) {
		return;
	}

	/* De-allocate list of child pointers */
	SYS_DLIST_FOR_EACH_NODE_SAFE(&slab->childs, elem, elem_safe) {
		sys_dlist_remove(elem);
		child_elem = CONTAINER_OF(elem, struct slab_child, root);
		k_free(child_elem);
	}

	/* De-allocate memory specific for this slab type */
	switch (slab->type) {
	case SLAB_TYPE_LED:
		slab_led_destroy(slab);
		break;

	case SLAB_TYPE_DELAY:
		slab_delay_destroy(slab);
		break;

	case SLAB_TYPE_GLOWER:
		slab_glower_destroy(slab);
		break;

	case SLAB_TYPE_HSV2RGB:
		slab_hsv2rgb_destroy(slab);
		break;

	case SLAB_TYPE_RGB2HSV:
		slab_rgb2hsv_destroy(slab);
		break;

	default:
		/* Silently ignore */
		break;
	}
}

void slab_connect(struct slab *slab, struct slab *connect_to)
{
	sys_dnode_t *elem;
	struct slab_child *child_elem;
	struct slab_child *new_child_elem;

	if (slab == NULL || slab->type == 0 ||
		connect_to == NULL || connect_to->type == 0) {
		return;
	}

	SYS_DLIST_FOR_EACH_NODE(&connect_to->childs, elem) {
		child_elem = CONTAINER_OF(elem, struct slab_child, root);
		if (child_elem->child == slab) {
			return;
		}
	}

	new_child_elem = k_malloc(sizeof(struct slab_child));
	__ASSERT(new_child_elem != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	sys_dnode_init(&new_child_elem->root);
	new_child_elem->child = slab;
	sys_dlist_append(&connect_to->childs, &new_child_elem->root);
}

void slab_disconnect(struct slab *slab, struct slab *disconnect_from)
{
	sys_dnode_t *elem;
	sys_dnode_t *elem_safe;
	struct slab_child *child_elem;

	if (slab == NULL || slab->type == 0 ||
		disconnect_from == NULL || disconnect_from->type == 0) {
		return;
	}

	SYS_DLIST_FOR_EACH_NODE_SAFE(&disconnect_from->childs, elem, elem_safe) {
		child_elem = CONTAINER_OF(elem, struct slab_child, root);
		if (child_elem->child == slab) {
			sys_dlist_remove(elem);
			k_free(child_elem);
		}
	}
}

void slab_stim_childs(struct slab *slab, struct slab_event *evt)
{
	sys_dnode_t *elem;
	struct slab_child *child_elem;

	SYS_DLIST_FOR_EACH_NODE(&slab->childs, elem) {
		child_elem = CONTAINER_OF(elem, struct slab_child, root);
		slab_stim(child_elem->child, evt);
	}

	slab_event_release(evt);
}

void slab_stim(struct slab *slab, struct slab_event *evt)
{
	if (slab == NULL || evt == NULL) {
		return;
	}

	slab_event_acquire(evt);

	switch (slab->type) {
	case SLAB_TYPE_LED:
		slab_led_stim(slab, evt);
		break;

	case SLAB_TYPE_DELAY:
		slab_delay_stim(slab, evt);
		break;

	case SLAB_TYPE_GLOWER:
		slab_glower_stim(slab, evt);
		break;

	case SLAB_TYPE_HSV2RGB:
		slab_hsv2rgb_stim(slab, evt);
		break;

	case SLAB_TYPE_RGB2HSV:
		slab_rgb2hsv_stim(slab, evt);
		break;

	default:
		k_oops();
	}
}
