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
static struct slab *sc;
static struct slab *sl1;
static struct slab *sl2;
static struct slab *sl3;
static struct slab *sl4;

static struct hsv_value sole_color = {.h = 130, .s = 0.9, .v = 0.3};

void sole_constructor(void)
{
	light_res_err_t res_err = 0;

	res_err = light_resource_use("Lguard_1", &l1)
			  | light_resource_use("Lguard_2", &l2)
			  | light_resource_use("Lguard_3", &l3)
			  | light_resource_use("Lguard_4", &l4);
	if (res_err) {
		printk("resource use err %d", res_err);
		k_oops();
	}

	sc = slab_create(SLAB_TYPE_HSV2RGB);
	sl1 = slab_create(SLAB_TYPE_LED, l1->data, LED_TYPE_RGB);
	sl2 = slab_create(SLAB_TYPE_LED, l2->data, LED_TYPE_RGB);
	sl3 = slab_create(SLAB_TYPE_LED, l3->data, LED_TYPE_RGB);
	sl4 = slab_create(SLAB_TYPE_LED, l4->data, LED_TYPE_RGB);

	slab_connect(sl1, sc);
	slab_connect(sl2, sc);
	slab_connect(sl3, sc);
	slab_connect(sl4, sc);

	struct slab_event *hsv_evt = slab_event_create(SLAB_EVENT_HSV, sole_color);
	slab_stim(sc, hsv_evt);
}

void sole_destructor(void)
{
	light_res_err_t res_err = 0;

	slab_destroy(sl4);
	slab_destroy(sl3);
	slab_destroy(sl2);
	slab_destroy(sl1);
	slab_destroy(sc);

	res_err = light_resource_return(l1)
			  | light_resource_return(l2)
			  | light_resource_return(l3)
			  | light_resource_return(l4);

	if (res_err) {
		printk("resource return err %d", res_err);
		k_oops();
	}
}

void sole_reset(void)
{
	slab_stim(sc, slab_event_create(SLAB_EVENT_RESET));
}

/* Tweak function implementation could be dropped.
   In this case, posting a tweak event will do nothing. */

void sole_tweak_color(float hue)
{
	sole_color.h = hue;
	struct slab_event *hsv_evt = slab_event_create(SLAB_EVENT_HSV, sole_color);
	slab_stim(sc, hsv_evt);
}

void sole_tweak_intensity(float saturation)
{
	sole_color.s = saturation;
	struct slab_event *hsv_evt = slab_event_create(SLAB_EVENT_HSV, sole_color);
	slab_stim(sc, hsv_evt);
}

void sole_tweak_gain(float value)
{
	sole_color.v = value;
	struct slab_event *hsv_evt = slab_event_create(SLAB_EVENT_HSV, sole_color);
	slab_stim(sc, hsv_evt);
}

static struct hikari_light_mode_api sole_api = {
	.constructor = sole_constructor,
	.destructor = sole_destructor,
	.tweak_color = sole_tweak_color,
	.tweak_intensity = sole_tweak_intensity,
	.tweak_gain = sole_tweak_gain,
	.tweak_speed = NULL
};

DEFINE_HIKARI_LIGHT_MODE(sole, HIKARI_LIGHT_MODE_SOLE, sole_api);
