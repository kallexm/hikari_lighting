#include <stddef.h>

#include "light_resource.h"
#include "hikari_light.h"

#include "slab.h"
#include "slabs/slab_glower.h"
#include "slabs/slab_led.h"
#include "slabs/slab_notifier.h"

#include "slab_event.h"
#include "../lib/slab/events/slab_event_rgb.h"

/* LED resources */
static struct light_resource *l1;
static struct light_resource *l2;
static struct light_resource *l3;
static struct light_resource *l4;

/* Slabs */
static struct slab *st;
static struct slab *sg;
static struct slab *sc;
static struct slab *sd1;
static struct slab *sd2;
static struct slab *sd3;
static struct slab *sl1;
static struct slab *sl2;
static struct slab *sl3;
static struct slab *sl4;

static struct slab *cb1;

static void print_callback(struct slab_event *evt, void *ctx)
{
	/*struct slab_event_rgb *rgb_evt = (struct slab_event_rgb *)evt;

	if (evt->id == SLAB_EVENT_RGB) {
		printk("rgb: [%d, %d, %d]\n", rgb_evt->r, rgb_evt->g, rgb_evt->b);
	}*/
}

static void glow_constructor(void)
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

	struct slab_glower_config sg_config = {
		.hue = 130, .sat = 0.9,
		.val = {.a = 0.25, .b = 0.25, .ym = 0.3, .yd = 4.0}
	};

	st = slab_create(SLAB_TYPE_TICKER, K_MSEC(25));
	sg = slab_create(SLAB_TYPE_GLOWER, &sg_config);
	sc = slab_create(SLAB_TYPE_HSV2RGB);
	sd1 = slab_create(SLAB_TYPE_DELAY, 20);
	sd2 = slab_create(SLAB_TYPE_DELAY, 20);
	sd3 = slab_create(SLAB_TYPE_DELAY, 20);
	sl1 = slab_create(SLAB_TYPE_LED, l1->data, LED_TYPE_RGB);
	sl2 = slab_create(SLAB_TYPE_LED, l2->data, LED_TYPE_RGB);
	sl3 = slab_create(SLAB_TYPE_LED, l3->data, LED_TYPE_RGB);
	sl4 = slab_create(SLAB_TYPE_LED, l4->data, LED_TYPE_RGB);

	cb1 = slab_create(SLAB_TYPE_NOTIFIER, print_callback, NULL);
	slab_connect(cb1, sl3);

	slab_connect(sg, st);
	slab_connect(sc, sg);

	slab_connect(sd1, sc);
	slab_connect(sl1, sc);

	slab_connect(sd2, sd1);
	slab_connect(sl2, sd1);
	
	slab_connect(sd3, sd2);
	slab_connect(sl3, sd2);

	slab_connect(sl4, sd3);
}

static void glow_destructor(void)
{
	light_res_err_t res_err = 0;

	slab_destroy(sl4);

	slab_destroy(sl3);
	slab_destroy(sd3);

	slab_destroy(sl2);
	slab_destroy(sd2);

	slab_destroy(sl1);
	slab_destroy(sd1);

	slab_destroy(sc);
	slab_destroy(sg);
	slab_destroy(st);

	res_err = light_resource_return(l1)
			  | light_resource_return(l2)
			  | light_resource_return(l3)
			  | light_resource_return(l4);

	if (res_err) {
		printk("resource return err %d", res_err);
		k_oops();
	}
}

 void glow_reset(void)
{
	slab_stim(st, slab_event_create(SLAB_EVENT_RESET));
}

/* Tweak function implementation could be dropped.
   In this case, posting a tweak event will do nothing. */

/*void glow_tweak_color(float hue)
{

}

void glow_tweak_intensity(float saturation)
{

}

void glow_tweak_gain(float value)
{

}

void glow_tweak_speed(float speed)
{

}*/

static struct hikari_light_mode_api glow_api = {
	.constructor = glow_constructor,
	.destructor = glow_destructor,
	.tweak_color = NULL,
	.tweak_intensity = NULL,
	.tweak_gain = NULL,
	.tweak_speed = NULL
};

DEFINE_HIKARI_LIGHT_MODE(glow, HIKARI_LIGHT_MODE_GLOW, glow_api);
