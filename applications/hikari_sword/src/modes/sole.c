#include <stddef.h>

#include "light_resource.h"
#include "hikari_light.h"

#include "slab.h"
#include "slabs/slab_led.h"

#include "slab_event.h"
#include "../lib/slab/events/slab_event_rgb.h"

#include "default_resources.h"

/* Slabs */
static struct slab *sc;

static struct hsv_value sole_color = {.h = 130, .s = 0.9, .v = 0.3};

void sole_constructor(void)
{
	light_res_err_t res_err = 0;

	res_err = USE_ALL_HIKARI_LIGHT_RESOURCES;
	if (res_err) {
		printk("resource use err %d", res_err);
		k_oops();
	}

	CREATE_ALL_HIKARI_LIGHT_SLABS;

	sc = slab_create(SLAB_TYPE_HSV2RGB);

	slab_connect(slp[1], slp[0]);

	slab_connect(slc[1], slc[0]);
	slab_connect(slc[2], slc[0]);
	slab_connect(slc[3], slc[0]);
	slab_connect(slc[4], slc[0]);
	slab_connect(slc[5], slc[0]);

	slab_connect(slsq[1], slsq[0]);

	slab_connect(slms[1], slms[0]);
	slab_connect(slms[2], slms[0]);
	slab_connect(slms[3], slms[0]);
	slab_connect(slms[4], slms[0]);
	slab_connect(slms[5], slms[0]);

	slab_connect(slts[1], slts[0]);

	slab_connect(sllb[1], sllb[0]);
	slab_connect(sllb[2], sllb[0]);
	slab_connect(sllb[3], sllb[0]);
	slab_connect(sllb[4], sllb[0]);
	slab_connect(sllb[5], sllb[0]);

	slab_connect(sllt[1], sllt[0]);
	slab_connect(sllt[2], sllt[0]);
	slab_connect(sllt[3], sllt[0]);
	slab_connect(sllt[4], sllt[0]);
	slab_connect(sllt[5], sllt[0]);
	slab_connect(sllt[6], sllt[0]);
	slab_connect(sllt[7], sllt[0]);

	slab_connect(slrb[1], sllb[0]);
	slab_connect(slrb[2], sllb[0]);
	slab_connect(slrb[3], sllb[0]);
	slab_connect(slrb[4], sllb[0]);
	slab_connect(slrb[5], sllb[0]);

	slab_connect(slrt[1], slrt[0]);
	slab_connect(slrt[2], slrt[0]);
	slab_connect(slrt[3], slrt[0]);
	slab_connect(slrt[4], slrt[0]);
	slab_connect(slrt[5], slrt[0]);
	slab_connect(slrt[6], slrt[0]);
	slab_connect(slrt[7], slrt[0]);

	slab_connect(sllg[1], sllg[0]);
	slab_connect(sllg[2], sllg[0]);
	slab_connect(sllg[3], sllg[0]);

	slab_connect(slrg[1], slrg[0]);
	slab_connect(slrg[2], slrg[0]);
	slab_connect(slrg[3], slrg[0]);

	slab_connect(slls[1], slls[0]);
	slab_connect(slls[2], slls[0]);

	slab_connect(slrs[1], slrs[0]);
	slab_connect(slrs[2], slrs[0]);

	slab_connect(slc[0], sc);
	slab_connect(slsq[0], sc);
	slab_connect(slms[0], sc);
	slab_connect(slts[0], sc);
	slab_connect(sllb[0], sc);
	slab_connect(sllt[0], sc);
	slab_connect(slrb[0], sc);
	slab_connect(slrt[0], sc);
	slab_connect(sllg[0], sc);
	slab_connect(slrg[0], sc);
	slab_connect(slls[0], sc);
	slab_connect(slrs[0], sc);

	struct slab_event *hsv_evt = slab_event_create(SLAB_EVENT_HSV, sole_color);
	slab_stim(sc, hsv_evt);
}

void sole_destructor(void)
{
	light_res_err_t res_err = 0;

	DESTROY_ALL_HIKARI_LIGHT_SLABS;

	res_err = RETURN_ALL_HIKARI_LIGHT_RESOURCES;
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
