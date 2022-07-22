#ifndef SLAB_HSV2RGB_H__
#define SLAB_HSV2RGB_H__

#include "slab.h"
#include "slab_event.h"

struct slab *slab_hsv2rgb_create(void);

void slab_hsv2rgb_destroy(struct slab *slab);

void slab_hsv2rgb_stim(struct slab *slab, struct slab_event *evt);

#endif /* SLAB_HSV2RGB_H__ */
