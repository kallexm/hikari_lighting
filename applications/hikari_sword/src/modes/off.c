#include <stddef.h>

#include "light_resource.h"
#include "hikari_light.h"

#include "slab.h"
#include "slabs/slab_led.h"

#include "slab_event.h"
#include "../lib/slab/events/slab_event_rgb.h"

/* LED resources */
static struct light_resource *l1;
static struct light_resource *l2;
static struct light_resource *l3;
static struct light_resource *l4;

/* Slabs */
static struct slab *sl1;
static struct slab *sl2;
static struct slab *sl3;
static struct slab *sl4;

void off_constructor(void)
{
	light_res_err_t res_err = 0;

	res_err = light_resource_use("L1", &l1)
			| light_resource_use("L2", &l2)
			| light_resource_use("L3", &l3)
			| light_resource_use("L4", &l4);

	if (res_err) {
		printk("resource use err %d", res_err);
		k_oops();
	}

	sl1 = slab_create(SLAB_TYPE_LED, l1->data, LED_TYPE_RGB);
	sl2 = slab_create(SLAB_TYPE_LED, l2->data, LED_TYPE_RGB);
	sl3 = slab_create(SLAB_TYPE_LED, l3->data, LED_TYPE_RGB);
	sl4 = slab_create(SLAB_TYPE_LED, l4->data, LED_TYPE_RGB);

	slab_connect(sl2, sl1);
	slab_connect(sl3, sl2);
	slab_connect(sl4, sl3);

	struct rgb_value rgb_val = {.r = 0, .g = 0, .b = 0};
	struct slab_event *rgb_evt = slab_event_create(SLAB_EVENT_RGB, rgb_val);
	slab_stim(sl1, rgb_evt);
}

void off_destructor(void)
{
	light_res_err_t res_err = 0;
	
	slab_destroy(sl4);
	slab_destroy(sl3);
	slab_destroy(sl2);
	slab_destroy(sl1);

	res_err = light_resource_return(l1)
			| light_resource_return(l2)
			| light_resource_return(l3)
			| light_resource_return(l4);

	if (res_err) {
		printk("resource return err %d", res_err);
		k_oops();
	}
}

void off_reset(void)
{
	slab_stim(sl1, slab_event_create(SLAB_EVENT_RESET));
}

static struct hikari_light_mode_api off_api = {
	.constructor = off_constructor,
	.destructor = off_destructor,
	.tweak_color = NULL,
	.tweak_intensity = NULL,
	.tweak_gain = NULL,
	.tweak_speed = NULL
};

DEFINE_HIKARI_LIGHT_MODE(off, HIKARI_LIGHT_MODE_OFF, off_api);
