#include <stddef.h>

#include "light_resource.h"
#include "hikari_light.h"

#include "slab.h"
#include "slabs/slab_glower.h"
#include "slabs/slab_led.h"
#include "slabs/slab_notifier.h"

#include "slab_event.h"
#include "../lib/slab/events/slab_event_rgb.h"

#include "default_resources.h"

/* Slabs */
static struct slab *st;
static struct slab *sg;
static struct slab *sc;

static struct slab *cb1;

static struct slab *sd1;
static struct slab *sd2;
static struct slab *sd3;
static struct slab *sd4;
static struct slab *sd5;
static struct slab *sd6;
static struct slab *sd7;
static struct slab *sd8;
static struct slab *sd9;
static struct slab *sd10;
static struct slab *sd11;
static struct slab *sd12;
static struct slab *sd13;

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

	res_err = USE_ALL_HIKARI_LIGHT_RESOURCES;
	if (res_err) {
		printk("resource use err %d", res_err);
		k_oops();
	}

	CREATE_ALL_HIKARI_LIGHT_SLABS;

	/* Source Generator */
	struct slab_glower_config sg_config = {
		.hue = 130, .sat = 0.9,
		.val = {.a = 0.25, .b = 0.25, .ym = 0.3, .yd = 4.0}
	};
	st = slab_create(SLAB_TYPE_TICKER, K_MSEC(25));
	sg = slab_create(SLAB_TYPE_GLOWER, &sg_config);
	sc = slab_create(SLAB_TYPE_HSV2RGB);
	slab_connect(sg, st);
	slab_connect(sc, sg);

	/* Debug callback */
	cb1 = slab_create(SLAB_TYPE_NOTIFIER, print_callback, NULL);
	slab_connect(cb1, sc);

	/* Start glow from core */
	slab_connect(slc[0], sc);
	slab_connect(slc[1], slc[0]);
	slab_connect(slc[2], slc[0]);
	slab_connect(slc[3], slc[0]);
	slab_connect(slc[4], slc[0]);
	slab_connect(slc[5], slc[0]);

	/* Pommel glow */
	sd1 = slab_create(SLAB_TYPE_DELAY, 80);
	slab_connect(sd1, sc);
	slab_connect(slp[0], sd1);
	slab_connect(slp[1], sd1);

	/* Inner blade glow stage 1 */
	sd2 = slab_create(SLAB_TYPE_DELAY, 20);
	slab_connect(sd2, sc);
	slab_connect(slsq[0], sd2);
	slab_connect(slsq[1], sd2);

	/* Inner blade glow stage 2 */
	sd3 = slab_create(SLAB_TYPE_DELAY, 40);
	slab_connect(sd3, sd2);
	slab_connect(slms[0], sd3);
	slab_connect(slms[1], sd3);
	slab_connect(slms[2], sd3);
	slab_connect(slms[3], sd3);
	slab_connect(slms[4], sd3);
	slab_connect(slms[5], sd3);

	/* Inner blade glow stage 3 */
	sd4 = slab_create(SLAB_TYPE_DELAY, 40);
	slab_connect(sd4, sd3);
	slab_connect(slts[0], sd4);
	slab_connect(slts[1], sd4);

	/* Outer blade glow stage 1 */
	sd5 = slab_create(SLAB_TYPE_DELAY, 20);
	slab_connect(sd5, sc);
	slab_connect(sllb[0], sd5);
	slab_connect(sllb[1], sd5);
	slab_connect(slrb[0], sd5);
	slab_connect(slrb[1], sd5);

	/* Outer blade glow stage 2 */
	sd6 = slab_create(SLAB_TYPE_DELAY, 20);
	slab_connect(sd6, sd5);
	slab_connect(sllb[2], sd6);
	slab_connect(sllb[3], sd6);
	slab_connect(slrb[2], sd6);
	slab_connect(slrb[3], sd6);

	/* Outer blade glow stage 3 */
	sd7 = slab_create(SLAB_TYPE_DELAY, 20);
	slab_connect(sd7, sd6);
	slab_connect(sllb[4], sd7);
	slab_connect(sllb[5], sd7);
	slab_connect(slrb[4], sd7);
	slab_connect(slrb[5], sd7);

	/* Outer blade glow stage 4 */
	sd8 = slab_create(SLAB_TYPE_DELAY, 20);
	slab_connect(sd8, sd7);
	slab_connect(sllt[2], sd8);
	slab_connect(sllt[3], sd8);
	slab_connect(sllt[6], sd8);
	slab_connect(sllt[7], sd8);
	slab_connect(slrt[2], sd8);
	slab_connect(slrt[3], sd8);
	slab_connect(slrt[6], sd8);
	slab_connect(slrt[7], sd8);

	/* Outer blade glow stage 5 */
	sd9 = slab_create(SLAB_TYPE_DELAY, 20);
	slab_connect(sd9, sd8);
	slab_connect(sllt[0], sd9);
	slab_connect(sllt[1], sd9);
	slab_connect(sllt[4], sd9);
	slab_connect(sllt[5], sd9);
	slab_connect(slrt[0], sd9);
	slab_connect(slrt[1], sd9);
	slab_connect(slrt[4], sd9);
	slab_connect(slrt[5], sd9);

	/* Spike glow stage 1 */
	sd10 = slab_create(SLAB_TYPE_DELAY, 20);
	slab_connect(sd10, sc);
	slab_connect(slls[0], sd10);
	slab_connect(slls[1], sd10);
	slab_connect(slrs[0], sd10);
	slab_connect(slrs[1], sd10);

	/* Spike glow stage 2 */
	sd11 = slab_create(SLAB_TYPE_DELAY, 20);
	slab_connect(sd11, sd10);
	slab_connect(slls[2], sd11);
	slab_connect(slrs[2], sd11);

	/* Guard glow stage 1 */
	sd12 = slab_create(SLAB_TYPE_DELAY, 20);
	slab_connect(sd12, sc);
	slab_connect(sllg[0], sd12);
	slab_connect(sllg[3], sd12);
	slab_connect(slrg[0], sd12);
	slab_connect(slrg[3], sd12);

	/* Guard glow stage 2 */
	sd13 = slab_create(SLAB_TYPE_DELAY, 20);
	slab_connect(sd13, sc);
	slab_connect(sllg[1], sd13);
	slab_connect(sllg[2], sd13);
	slab_connect(slrg[1], sd13);
	slab_connect(slrg[2], sd13);
}

static void glow_destructor(void)
{
	light_res_err_t res_err = 0;

	DESTROY_ALL_HIKARI_LIGHT_SLABS;

	slab_destroy(sc);
	slab_destroy(sg);
	slab_destroy(st);

	slab_destroy(cb1);

	slab_destroy(sd1);
	slab_destroy(sd2);
	slab_destroy(sd3);
	slab_destroy(sd4);
	slab_destroy(sd5);
	slab_destroy(sd6);
	slab_destroy(sd7);
	slab_destroy(sd8);
	slab_destroy(sd9);
	slab_destroy(sd10);
	slab_destroy(sd11);
	slab_destroy(sd12);
	slab_destroy(sd13);

	res_err = RETURN_ALL_HIKARI_LIGHT_RESOURCES;
	if (res_err) {
		printk("resource return err %d", res_err);
		k_oops();
	}
}

 void glow_reset(void)
{
	slab_stim(st, slab_event_create(SLAB_EVENT_RESET));
}

static struct hikari_light_mode_api glow_api = {
	.constructor = glow_constructor,
	.destructor = glow_destructor,
	.tweak_color = NULL,
	.tweak_intensity = NULL,
	.tweak_gain = NULL,
	.tweak_speed = NULL
};

DEFINE_HIKARI_LIGHT_MODE(glow, HIKARI_LIGHT_MODE_GLOW, glow_api);
