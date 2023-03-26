#ifndef SLAB_WAVER_H__
#define SLAB_WAVER_H__

#include "slab.h"
#include "rgb_hsv.h"
#include "wave_func.h"

struct slab_waver_config {
	float hue; /* [0,360] */
	float sat; /* [0,1] */
	struct wave_func_conf val;
};

struct slab_waver {
	sys_dlist_t childs;
	enum slab_type type;

	/* Specific data */
	void *gen;
	struct hsv_value data;
};

struct slab *slab_waver_create(struct slab_waver_config *config);

void slab_waver_destroy(struct slab *slab);

void slab_waver_stim(struct slab *slab, struct slab_event *evt);

#endif /* SLAB_WAVER_H__ */
