#include <stddef.h>

#include "light_resource.h"
#include "hikari_light.h"

#include "slab.h"
#include "slabs/slab_led.h"

#include "slab_event.h"
#include "../lib/slab/events/slab_event_rgb.h"

#include "default_resources.h"

void off_constructor(void)
{
	light_res_err_t res_err = 0;

	res_err = USE_ALL_HIKARI_LIGHT_RESOURCES;
	if (res_err) {
		printk("resource use err %d", res_err);
		k_oops();
	}

	CREATE_ALL_HIKARI_LIGHT_SLABS;

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

	slab_connect(slc[0], slp[0]);
	slab_connect(slsq[0], slp[0]);
	slab_connect(slms[0], slp[0]);
	slab_connect(slts[0], slp[0]);
	slab_connect(sllb[0], slp[0]);
	slab_connect(sllt[0], slp[0]);
	slab_connect(slrb[0], slp[0]);
	slab_connect(slrt[0], slp[0]);
	slab_connect(sllg[0], slp[0]);
	slab_connect(slrg[0], slp[0]);
	slab_connect(slls[0], slp[0]);
	slab_connect(slrs[0], slp[0]);

	struct rgb_value rgb_val = {.r = 0, .g = 0, .b = 0};
	struct slab_event *rgb_evt = slab_event_create(SLAB_EVENT_RGB, rgb_val);
	slab_stim(slp[0], rgb_evt);
}

void off_destructor(void)
{
	light_res_err_t res_err = 0;

	DESTROY_ALL_HIKARI_LIGHT_SLABS;

	res_err = RETURN_ALL_HIKARI_LIGHT_RESOURCES;
	if (res_err) {
		printk("resource return err %d", res_err);
		k_oops();
	}
}

void off_reset(void)
{
	slab_stim(slp[0], slab_event_create(SLAB_EVENT_RESET));
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
