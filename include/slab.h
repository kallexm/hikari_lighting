#ifndef SLAB_H__
#define SLAB_H__

#include <stdint.h>
#include <zephyr.h>

#include "slab_event.h"

/** SLAB: Smart LED Animation Block
 */

enum slab_type {
	SLAB_TYPE_LED,
	SLAB_TYPE_DELAY,
	SLAB_TYPE_GLOWER,
	SLAB_TYPE_HSV2RGB,
	SLAB_TYPE_RGB2HSV,
};

struct slab {
	sys_dlist_t childs;
	enum slab_type type;
};

struct slab *slab_create(enum slab_type type, ...);
void slab_destroy(struct slab *slab);

void slab_connect(struct slab *slab, struct slab *connect_to);
void slab_disconnect(struct slab *slab, struct slab *disconnect_from);

void slab_stim(struct slab *slab, struct slab_event *evt);
void slab_stim_childs(struct slab *slab, struct slab_event *evt);

#endif /* SLAB_H__ */
