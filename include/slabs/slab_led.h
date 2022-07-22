#ifndef SLAB_LED_H__
#define SLAB_LED_H__

#include "slab.h"
#include "rgb_hsv.h"

enum led_type {
	LED_TYPE_RGB,
	LED_TYPE_GRB,
	LED_TYPE_RED,
	LED_TYPE_GREEN,
	LED_TYPE_BLUE,
};

struct slab_led {
	sys_dlist_t childs;
	enum slab_type type;

	uint8_t *led;
	enum led_type led_type;
};

struct slab *slab_led_create(void *led_buf, enum led_type type);

void slab_led_destroy(struct slab *slab);

void slab_led_stim(struct slab *slab, struct slab_event *evt);

#endif /* SLAB_LED_H__ */
