#ifndef SLAB_GLOWER_H__
#define SLAB_GLOWER_H__

#include "slab.h"
#include "rgb_hsv.h"
#include "glow_func.h"

struct slab_glower_config {
	float hue; /* [0,360] */
	float sat; /* [0,1] */
	struct glow_func_conf val;
};

struct slab_glower {
	sys_dlist_t childs;
	enum slab_type type;

	/* Specific data */
	void *gen;
	struct hsv_value data;
};

struct slab *slab_glower_create(struct slab_glower_config *config);

void slab_glower_destroy(struct slab *slab);

void slab_glower_stim(struct slab *slab, struct slab_event *evt);

#endif /* SLAB_GLOWER_H__ */
