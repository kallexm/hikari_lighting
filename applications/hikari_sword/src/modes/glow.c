#include <stddef.h>

#include "light_resource.h"
#include "hikari_light.h"

#include "slab.h"
#include "slabs/slab_glower.h"
#include "slabs/slab_led.h"

/* LED resources */
struct light_resource *l1;
struct light_resource *l2;
struct light_resource *l3;

/* Slabs */
struct slab *st;
struct slab *sg;
struct slab *sc;
struct slab *sd1;
struct slab *sd2;
struct slab *sl1;
struct slab *sl2;
struct slab *sl3;

void glow_constructor(void)
{
	light_res_err_t res_err = 0;

	res_err |= light_resource_use("L1", &l1);
	res_err |= light_resource_use("L2", &l2);
	res_err |= light_resource_use("L3", &l3);
	if (res_err) {
		k_oops();
	}

	struct slab_glower_config sg_config = {
		.hue = 150, .sat = 0.5,
		.val = {.a = 0.1, .b = 2, .ym = 0.5, .yd = 0.5}
	};

	st = slab_create(SLAB_TYPE_TICKER, K_MSEC(100));
	sg = slab_create(SLAB_TYPE_GLOWER, &sg_config);
	sc = slab_create(SLAB_TYPE_HSV2RGB);
	sd1 = slab_create(SLAB_TYPE_DELAY, 10);
	sd2 = slab_create(SLAB_TYPE_DELAY, 10);
	sl1 = slab_create(SLAB_TYPE_LED, l1->data, LED_TYPE_RGB);
	sl2 = slab_create(SLAB_TYPE_LED, l2->data, LED_TYPE_RGB);
	sl3 = slab_create(SLAB_TYPE_LED, l3->data, LED_TYPE_RGB);

	slab_connect(sg, st);
	slab_connect(sc, sg);

	slab_connect(sd1, sc);
	slab_connect(sl1, sc);

	slab_connect(sd2, sd1);
	slab_connect(sl2, sd1);
	
	slab_connect(sl3, sd2);
}

void glow_destructor(void)
{
	light_res_err_t res_err = 0;

	slab_destroy(sl3);

	slab_destroy(sl2);
	slab_destroy(sd2);

	slab_destroy(sl1);
	slab_destroy(sd1);

	slab_destroy(sc);
	slab_destroy(sg);
	slab_destroy(st);

	res_err |= light_resource_return(l1);
	res_err |= light_resource_return(l2);
	res_err |= light_resource_return(l3);
	if (res_err) {
		k_oops();
	}
}

void glow_reset(void)
{
	slab_stim(st, slab_event_create(SLAB_EVENT_RESET));
}

/* Tweak function implementation could be dropped.
   In this case, posting a tweak event will do nothing. */

/*void glow_tweak_color(uint8_t hue)
{

}

void glow_tweak_intensity(uint8_t saturation)
{

}

void glow_tweak_gain(uint8_t value)
{

}

void glow_tweak_speed(uint8_t speed)
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
