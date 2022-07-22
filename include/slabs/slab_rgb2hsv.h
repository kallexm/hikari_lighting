#ifndef SLAB_RGB2HSV_H__
#define SLAB_RGB2HSV_H__

#include "slab.h"
#include "slab_event.h"

struct slab *slab_rgb2hsv_create(void);

void slab_rgb2hsv_destroy(struct slab *slab);

void slab_rgb2hsv_stim(struct slab *slab, struct slab_event *evt);

#endif /* SLAB_RGB2HSV_H__ */
