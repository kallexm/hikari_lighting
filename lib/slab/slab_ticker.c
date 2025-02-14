#include <zephyr/kernel.h>

#include "slab_event.h"
#include "events/slab_event_tick.h"

#include "slabs/slab_ticker.h"

K_THREAD_STACK_DEFINE(ticker_workqueue_stack, TICKER_WORKQUEUE_STACK_SIZE);
static struct k_work_q ticker_work_q;
static bool work_q_initialized = false;


static void timer_expired(struct k_timer *timer)
{
	struct slab_ticker *ticker = CONTAINER_OF(timer, struct slab_ticker, tick_timer);

	k_work_submit_to_queue(&ticker_work_q, &ticker->tick_work);
}

static void work_tick(struct k_work *work)
{
	struct slab_ticker *ticker = CONTAINER_OF(work, struct slab_ticker, tick_work);
	struct slab *slab = (struct slab *)ticker;

	uint32_t current_time = k_uptime_get_32();
	struct slab_event *tick_evt = slab_event_create(SLAB_EVENT_TICK, current_time);

	slab_event_acquire(tick_evt);
	slab_stim_childs(slab, tick_evt);
}

struct slab *slab_ticker_create(k_timeout_t tick_period)
{
	struct slab_ticker *new_slab = k_malloc(sizeof(struct slab_ticker));
	__ASSERT(new_slab != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	if (!work_q_initialized) {
		k_work_queue_init(&ticker_work_q);
		k_work_queue_start(&ticker_work_q, ticker_workqueue_stack,
						   K_THREAD_STACK_SIZEOF(ticker_workqueue_stack),
						   TICKER_WORKQUEUE_THREAD_PRIO, NULL);

		work_q_initialized = true;
	}

	new_slab->period = tick_period;

	k_work_init(&new_slab->tick_work, work_tick);
	k_timer_init(&new_slab->tick_timer, timer_expired, NULL);
	k_timer_start(&new_slab->tick_timer, new_slab->period, new_slab->period);

	return ((struct slab *)new_slab);
}

void slab_ticker_destroy(struct slab *slab)
{
	struct slab_ticker *ticker = (struct slab_ticker *)slab;
	
	k_timer_stop(&ticker->tick_timer);
	k_work_cancel(&ticker->tick_work);

	k_free(slab);
}

void slab_ticker_stim(struct slab *slab, struct slab_event *evt)
{
	struct slab_ticker *ticker = (struct slab_ticker *)slab;

	switch (evt->id) {
	case SLAB_EVENT_RESET:
		k_timer_stop(&ticker->tick_timer);
		k_work_cancel(&ticker->tick_work);

		slab_stim_childs(slab, evt);

		k_timer_start(&ticker->tick_timer, ticker->period, ticker->period);
		break;

	case SLAB_EVENT_TICK:
		/* Stop propagation */
		slab_event_release(evt);
		break;
	
	default:
		slab_stim_childs(slab, evt);
		break;
	}
}