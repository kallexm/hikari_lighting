#include <ztest.h>
#include <kernel.h>

#include "slab.h"
#include "slab_event.h"

/*==============================[slab_ticker test suit]==============================*/
struct slab_ticker_suit_fixture {
	struct slab *st;
	struct slab *sn;
	int notifications_received;
	bool reset_evt_received;
};

static void slab_ticker_notifier_hook(struct slab_event *evt, void *fixture)
{
	struct slab_ticker_suit_fixture *f = (struct slab_ticker_suit_fixture *)fixture;

	switch (evt->id) {
		case SLAB_EVENT_TICK: {
			f->notifications_received += 1;
			break;
		}
		case SLAB_EVENT_RESET: {
			f->reset_evt_received = true;
			break;
		}
		default:
			zassert_unreachable("Only reset and tick events should be received");
			break;
	}
}

static void *slab_ticker_suit_setup(void)
{
	struct slab_ticker_suit_fixture *f = k_malloc(sizeof(struct slab_ticker_suit_fixture));
	
	f->st = NULL;
	f->sn = NULL;
	f->notifications_received = 0;
	f->reset_evt_received = false;

	return f;
}

static void slab_ticker_suit_before(void *fixture)
{
	struct slab_ticker_suit_fixture *f = (struct slab_ticker_suit_fixture *)fixture;

	f->st = slab_create(SLAB_TYPE_TICKER, K_MSEC(100));
	f->sn = slab_create(SLAB_TYPE_NOTIFIER, slab_ticker_notifier_hook, f);

	f->notifications_received = 0;
	f->reset_evt_received = false;
}

static void slab_ticker_suit_after(void *fixture)
{
	struct slab_ticker_suit_fixture *f = (struct slab_ticker_suit_fixture *)fixture;

	slab_destroy(f->st);
	slab_destroy(f->sn);
}

static void slab_ticker_suit_teardown(void *fixture)
{
	struct slab_ticker_suit_fixture *f = (struct slab_ticker_suit_fixture *)fixture;

	k_free(f);
}

ZTEST_SUITE(slab_ticker_suit, NULL,
			slab_ticker_suit_setup,
			slab_ticker_suit_before,
			slab_ticker_suit_after,
			slab_ticker_suit_teardown);

ZTEST_F(slab_ticker_suit, test_tick_evt)
{	
	struct slab_ticker_suit_fixture *f = this;

	zassert_not_null(f->st, "slab_create failed");
	zassert_not_null(f->sn, "slab_create failed");

	slab_connect(f->sn, f->st);

	k_sleep(K_MSEC(550));

	zassert_equal(f->notifications_received, 5,
		"Wrong number of tick events received (%d, expected %d)",
		f->notifications_received, 5);
}

ZTEST_F(slab_ticker_suit, test_reset_evt)
{
	struct slab_ticker_suit_fixture *f = this;

	zassert_not_null(f->st, "slab_create failed");
	zassert_not_null(f->sn, "slab_create failed");

	slab_connect(f->sn, f->st);

	struct slab_event *reset_evt = slab_event_create(SLAB_EVENT_RESET);
	zassert_not_null(reset_evt, "slab_event_create failed");

	k_sleep(K_MSEC(50));

	slab_stim(f->st, reset_evt);

	k_sleep(K_MSEC(50));

	zassert_true(f->reset_evt_received, "No reset event received");
	zassert_equal(f->notifications_received, 0, 
		"Expected no tick events at this point in time");

	k_sleep(K_MSEC(100));

	zassert_equal(f->notifications_received, 1, 
		"Expected one tick event at this point in time");
}
