#include <ztest.h>
#include <kernel.h>

#include "slab.h"
#include "slabs/slab_ticker.h"
#include "slabs/slab_glower.h"
#include "slabs/slab_hsv2rgb.h"
#include "slabs/slab_led.h"

#include "slab_event.h"
#include "../lib/slab/events/slab_event_tick.h"
#include "../lib/slab/events/slab_event_hsv.h"
#include "../lib/slab/events/slab_event_rgb.h"

#define _D(s) printk(#s "\n");

/*==============================[slab_glower test suit]==============================*/
struct slab_glower_suit_fixture {
	struct slab *st;
	struct slab *sg;
	struct slab *sc;
	struct slab *sl;
	struct slab *sn;

	struct rgb_value val1;
};

static void slab_glower_suit_hook(struct slab_event *evt, void *ctx)
{
	struct slab_event_tick *et;
	struct slab_event_hsv *eh;
	struct slab_event_rgb *er;

	switch (evt->id) {
		case SLAB_EVENT_TICK:
			et = (struct slab_event_tick *)evt;
			printk("SLAB_EVENT_TICK: %d\n", et->time);
			break;
		case SLAB_EVENT_HSV:
			eh = (struct slab_event_hsv *)evt;
			printk("SLAB_EVENT_HSV: %f, %f, %f\n", eh->h, eh->s, eh->v);
			break;

		case SLAB_EVENT_RGB:
			er = (struct slab_event_rgb *)evt;
			printk("SLAB_EVENT_RGB: %d, %d, %d\n", er->r, er->g, er->b);
			break;

		case SLAB_EVENT_RESET:
			printk("SLAB_EVENT_RESET\n");
			break;

		default:
			printk("Unknown event!\n");
			break;
	}
}

static void *slab_glower_suit_setup(void)
{
	struct slab_glower_suit_fixture *f = k_malloc(sizeof(struct slab_glower_suit_fixture));
	
	f->st = NULL;
	f->sg = NULL;
	f->sc = NULL;
	f->sl = NULL;
	f->sn = NULL;

	memset(&f->val1, 0, sizeof(struct rgb_value));
	
	return f;
}

static void slab_glower_suit_before(void *fixture)
{
	struct slab_glower_suit_fixture *f = (struct slab_glower_suit_fixture *)fixture;

	struct slab_glower_config config = {
		.hue = 150,
		.sat = 0.5,
		.val = {
			.a = 0.1, .b = 2,
			.ym = 0.5, .yd = 0.5,
		}
	};

	f->st = slab_create(SLAB_TYPE_TICKER, K_MSEC(100));
	f->sg = slab_create(SLAB_TYPE_GLOWER, &config);
	f->sc = slab_create(SLAB_TYPE_HSV2RGB);
	f->sl = slab_create(SLAB_TYPE_LED, &f->val1, LED_TYPE_RGB);
	f->sn = slab_create(SLAB_TYPE_NOTIFIER, slab_glower_suit_hook, fixture);

	memset(&f->val1, 0, sizeof(struct rgb_value));
}

static void slab_glower_suit_after(void *fixture)
{
	struct slab_glower_suit_fixture *f = (struct slab_glower_suit_fixture *)fixture;

	slab_destroy(f->st);
	slab_destroy(f->sg);
	slab_destroy(f->sc);
	slab_destroy(f->sl);
	slab_destroy(f->sn);
}

static void slab_glower_suit_teardown(void *fixture)
{
	struct slab_glower_suit_fixture *f = (struct slab_glower_suit_fixture *)fixture;

	k_free(f);
}

ZTEST_SUITE(slab_glower_suit, NULL,
			slab_glower_suit_setup,
			slab_glower_suit_before,
			slab_glower_suit_after,
			slab_glower_suit_teardown);

ZTEST_F(slab_glower_suit, test_glower)
{	
	struct slab_glower_suit_fixture *f = this;

	zassert_not_null(f->st, "slab_create failed");
	zassert_not_null(f->sg, "slab_create failed");
	zassert_not_null(f->sc, "slab_create failed");
	zassert_not_null(f->sl, "slab_create failed");
	zassert_not_null(f->sn, "slab_create failed");

	slab_connect(f->sg, f->st);
	slab_connect(f->sc, f->sg);
	slab_connect(f->sl, f->sc);
	slab_connect(f->sn, f->sc);
	

	k_sleep(K_MSEC(2000));

	zassert_not_equal(f->val1.r, 0, "Red part of val1 should not be zero");
	zassert_not_equal(f->val1.g, 0, "Green part of val1 should not be zero");
	zassert_not_equal(f->val1.b, 0, "blue part of val1 should not be zero");

	printk("red: %d\n", f->val1.r);
	printk("green: %d\n", f->val1.g);
	printk("blue: %d\n", f->val1.b);
}
