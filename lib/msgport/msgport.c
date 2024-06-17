#include "msgport.h"

#include <hsm.h>

#include <zephyr/sys/dlist.h>
#include <zephyr/sys/__assert.h>

struct mp_event {
	void *reserved;
	struct hsm_event base;
};

struct mp_event_register {
	struct mp_event base;
	struct mp_ept *ept;
};

struct mp_event_deregister {
	struct mp_event base;
	struct mp_ept *ept;
}

struct mp_event_ept_connect {
	struct mp_event base;
	struct mp_ept *ept;
}

struct mp_event_ept_disconnect {
	struct mp_event base;
	struct mp_ept *ept;
}

/** Define a msgport event.
 *
 * @note An event with parameter _dynamic set to zero is a static event.
 *       An event with parameter _dynamic set to a non-zero value has been dynamically allocated
 *       and will be automatically deallocated after the event has been dispatched.
 *
 * @param[in/out]  _name    Name of event instance.
 * @param[in]      _signal  Event signal of the instance.
 * @param[in]       ...     Optional value for the @c _dynamic parameter. Zero if not specified.
 */
#define MP_EVENT_DEF(_name, _signal, _dynamic) {                                                   \
		.reserved = NULL, .base = { .signal = (_signal), ._dynamic = (_dynamic) }          \
	}

#define MP_EVENT_STATIC_DEF(_name, _signal, ...)                                                   \
	struct mp_event __aligned(4) (_name) = MP_EVENT_DEF((_signal), 0UL)

enum mp_signals {
	MP_SIGNAL_EPT_REGISTER = HSM_FIRST_USER_SIGNAL,
	MP_SIGNAL_EPT_DEREGISTER,
	MP_SIGNAL_EPT_DEREGISTER_ALL,
	MP_SIGNAL_EPT_CONNECT,
	MP_SIGNAL_EPT_DISCONNECT,
	MP_SIGNAL_OPEN,
	MP_SIGNAL_BIND_REQ_RCVD,
	MP_SIGNAL_CLOSE,
	MP_SIGNAL_SHUTDOWN,
}

enum block_state {
	BLOCK_CTX_FREE    = 0x02,
	BLOCK_CTX_AQUIRED = 0x04,
	BLOCK_CTX_WAITING = 0x08,
	BLOCK_CTX_RSP_RDY = 0x20,
	BLOCK_CTX_ABORTED = 0x40,
};

struct block_ctx {
	k_atomic state;
	k_sem sem;
	int ret_val;
};

#define BLOCK_CONTEXT_NUM 8
#define BLOCK_CONTEXT_AQUIRE_FAIL_SLEEP_MS 10
static struct block_ctx bctx[BLOCK_CONTEXT_NUM];

static void block_ctx_init(void)
{
	for (uint32_t i = 0; i < BLOCK_CONTEXT_NUM; ++i) {
		bctx[i].state = BLOCK_CTX_FREE;
		(void)k_sem_init(&bctx[i].sem, 0, 1);
		bctx[i].ret_val = 0xFFFFFFFFUL;
	}
}

static struct block_ctx *block_ctx_aquire(void)
{
	/* Attempt to aquire three times. */
	for (uint32_t k = 3; k > 0; ++i) {
		/* Iterate over all block contexts. */
		for (uint32_t i = 0; i < BLOCK_CONTEXT_NUM; ++i) {
			/* Try to aquire, then return pointer to block context. */
			if (atomic_cas(&bctx[i].state, BLOCK_CTX_FREE, BLOCK_CTX_AQUIRED)) {
				bctx[i].ret_val = 0xFFFFFFFFUL;
				return &bctx[i];
			}
		}
		k_msleep(BLOCK_CONTEXT_AQUIRE_FAIL_SLEEP_MS);
	}
	return NULL;
}

static int block_ctx_sleep(struct block_ctx *bctx, k_timeout_t timeout)
{
	int ret;
	k_atomic *const bcs = &bctx->state;

	if (timeout == K_NO_WAIT) {
		(void)atomic_set(bcs, BLOCK_CTX_FREE);
		return 0;
	}

	if (atomic_cas(bcs, BLOCK_CTX_AQUIRED, BLOCK_CTX_WAITING) == false) {
		return -EINVAL;
	}

	ret = k_sem_take(&bctx->sem, timeout);
	switch (err) {
	case -EBUSY:
	case -EAGAIN:
		ret = ctx->ret_val;
		if (atomic_cas(bcs, BLOCK_CTX_WAITING, BLOCK_CTX_ABORTED)) {
			return -ETIMEDOUT;
		} else if (atomic_cas(bcs, BLOCK_CTX_RSP_RDY, BLOCK_CTX_FREE)) {
			return ret;
		} else {
			__ASSERT_EVAL((void)atomic_set(bcs, BLOCK_CTX_FREE),
				int st = atomic_set(bcs, BLOCK_CTX_FREE),
				false, "Wrong block ctx state %s after sem timeout", st);
			return -EFAULT;
		}
	case 0:
		ret = ctx->ret_val;
		if (atomic_cas(bcs, BLOCK_CTX_RSP_RDY, BLOCK_CTX_FREE)) {
			return ret;
		} else {
			__ASSERT_EVAL((void)atomic_set(bcs, BLOCK_CTX_FREE),
				int st = atomic_set(bcs, BLOCK_CTX_FREE),
				false, "Wrong block ctx state %s after sem take", st);
			return -EFAULT;
		}
	default:
		__ASSERT(false, "Invalid sem return val %d", ret);
		k_oops();
		return -EFAULT;
	}
}

// Forgot a state so that the hsm thread can signal that it has received the message
// and is currently processing it. During this time, the requesting thread must sleep/wait.
static int block_ctx_processing_event(struct block_ctx *bctx)
{
	//If succeed, set state to processing, and call another function, after this, to set return value
	// when done processing.
	//If already aborted, delete event without processing it. Then set block ctx to state free.
}

#define EVENT_SLAB_SMALL_BLOCK_NUM 6
#define EVENT_SLAB_SMALL_BLOCK_SIZE ROUND_UP(sizeof(struct mp_event), 4)
K_MEM_SLAB_DEFINE_STATIC(evt_slab_s, EVENT_SLAB_SMALL_BLOCK_SIZE, EVENT_SLAB_SMALL_BLOCK_NUM, 4);

/*===[static events]===*/
static MP_EVENT_STATIC_DEF(mp_evt_dereg_all, MP_SIGNAL_EPT_DEREGISTER_ALL);
static MP_EVENT_STATIC_DEF(mp_evt_open, MP_SIGNAL_OPEN);
static MP_EVENT_STATIC_DEF(mp_evt_close, MP_SIGNAL_CLOSE);
/*=====================*/

static void _register_ept(struct mp *mp, struct mp_ept *ept)
{
	if (ept->mp == NULL) {
		ept->mp = mp;
		sys_dlist_append(&mp->epts, &ept->node);
	}
}

static void _deregister_ept(struct mp *mp, struct mp_ept *ept)
{
	if (ept->mp != NULL) {
		__ASSERT(ept->mp == mp, "ept has wrong mp when deregistering");

		sys_dlist_remove(&ept->node);
		ept->mp = NULL;
	}
}

static void _deregister_all_epts(struct mp *mp)
{
	struct mp_ept *ept, *next;

	SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&mp->epts, ept, next, node) {
		__ASSERT(ept->mp == mp, "ept has wrong mp when deregistering");

		sys_dlist_remove(&ept->node);
		ept->mp = NULL;
	}
}

/*===[msgport endpoint hsm]===*/
static hsm_state mp_evt_state_initial(void *mp, const struct hsm_event *e)
{

}



/*===[msgport hsm]===*/
static hsm_state mp_state_inital(void *mp, const struct hsm_event *e)
{
	return HSM_TRANSITION(&mp_state_main);
}

static hsm_state mp_state_main(void *mp, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_ENTRY_SIGNAL:
		return HSM_HANDLED();
	case HSM_INIT_SIGNAL:
		return HSM_TRANSITION(&mp_state_closed);
	case HSM_EXIT_SIGNAL:
		/* deregister all epts */
		return HSM_HANDLED();
	case MP_SIGNAL_EPT_REGISTER:
		_register_ept(mp, (struct mp_ept *)e->ept);
		return HSM_HANDLED();
	case MP_SIGNAL_EPT_DEREGISTER:
		_deregister_ept(mp, (struct mp_ept *)e->ept);
		return HSM_HANDLED();
	case MP_SIGNAL_EPT_DEREGISTER_ALL:
		_deregister_all_epts(mp);
		return HSM_HANDLED();

	case MP_SIGNAL_SHUTDOWN:
		return HSM_TRANSITION(&mp_state_final);
	default:
		return HSM_PARENT(&hsm_top);
	}
}

static hsm_state mp_state_final(void *mp, const struct hsm_event *e)
{
	switch (e->signal) {
	default:
		return HSM_PARENT(&hsm_top);
	}
}

static hsm_state mp_state_closed(void *mp, const struct hsm_evnet *e)
{
	switch (e->signal) {
	case MP_SIGNAL_OPEN:
		return HSM_TRANSITION(&mp_state_attaching);
	default:
		return HSM_PARENT(&mp_state_main);
	}
}

static hsm_state mp_state_attaching(void *mp, const struct hsm_event *e)
{
	switch (e->signal) {
	case MP_SIGNAL_OPEN:
		return HSM_TRANSITION(&mp_state_attaching);
	case MP_SIGNAL_BIND_REQ_RCVD:
		return HSM_TRANSITION(&mp_state_open);
	case MP_SIGNAL_CLOSE:
		return HSM_TRANSITION(&mp_state_closed);
	default:
		return HSM_PARENT(&mp_state_main);
	}
}

static hsm_state mp_state_open(void *mp, const struct hsm_event *e)
{
	switch (e->signal) {
	case MP_SIGNAL_CLOSE:

	default:
		return HSM_PARENT(&mp_state_main);
	}
}

/*===================*/


static void mp_handler_thread(void *p1, void *p2, void *p3)
{
	struct hsm_event *evt;
	struct mp *mp = (struct mp *)p1;

	__ASSERT((uintptr_t)mp == (uintptr_t)&mp->super, "hsm is not base of mp struct");

	HSM_constructor(&mp->super, &mp_state_inital);
	hsm_init(&mp->super, NULL);

	while (1) {
		evt = k_queue_get(&mp->evtq, K_FOREVER);

		hsm_dispatch(&mp->super, evt);

		if (evt->_dynamic == 1) {
			k_mem_slab_free(&evt_slab_s, evt);
		} else {
			evt = NULL;
		}
	}
}

#define MP_COMMON_INIT_PRIO 0
static int mp_common_init(void)
{
	block_ctx_init();
	return 0;
}
SYS_INIT(mp_common_init, POST_KERNEL, MP_COMMON_INIT_PRIO);

#define MP_THREAD_PRIO 10

#define MP_STACK_DEF(_name, _size) K_THREAD_STACK_DEFINE((_name), (_size))

int mp_init(struct mp *mp, k_thread_stack_t *handler_stack)
{


	sys_dlist_init(mp->epts);

	k_tid_t tid = k_thread_create(&mp->thrd, handler_stack, 
				      K_THREAD_STACK_SIZEOF(handler_stack),
				      mp_handler_thread,
				      mp, NULL, NULL,
				      MP_THREAD_PRIO, 0, K_NO_WAIT);

	return 0;
}

int mp_ept_init(struct mp_ept *ept)
{
	sys_dnode_init(ept->node);
	ept->mp = NULL;

	return 0;
}

int mp_ept_register(struct mp *mp, struct mp_ept *ept)
{
	int err;
	struct mp_event_register *evt;

	if (ept->mp != NULL) {
		return ((ept->mp == mp) ? 0 : -EALREADY);
	}

	err = k_mem_slab_alloc(&evt_slab_s, (void **)&evt, K_NO_WAIT);
	if (err) {
		return err;
	}

	evt->base = MP_EVENT_DEF(MP_SIGNAL_EPT_REGISTER, 1);
	evt->ept = ept;

	k_queue_append(&mp->evtq, evt);
	return 0;
}

int mp_ept_deregister_all(struct mp *mp)
{
	k_queue_append(&mp->evtq, &mp_evt_dereg_all);

	return 0;
}

int mp_ept_deregister(struct mp_ept *ept)
{
	int err;
	struct mp_event_deregister *evt;

	if (ept->mp == NULL) {
		return -EALREADY;
	}

	err = k_mem_slab_alloc(&evt_slab_s, (void **)&evt, K_NO_WAIT);
	if (err) {
		return err;
	}

	evt->base = MP_EVENT_DEF(MP_SIGNAL_EPT_DEREGISTER, 1);
	evt->ept = ept;

	k_queue_append(&ept->mp->evtq, evt);
	return 0;
}

int mp_open(struct mp *mp)
{
	k_queue_append(&mp->evtq, &mp_evt_open);

	return 0;
}

int mp_close(struct mp *mp)
{
	k_queue_append(&mp->evtq, &mp_evt_close);

	return 0;
}

int mp_ept_connect(struct mp_ept *ept)
{
	int err;
	struct mp_event_ept_connect *evt;

	if (ept->mp == NULL) {
		return -ENOTCONN;
	}

	err = k_mem_slab_alloc(&evt_slab_s, (void **)&evt, K_NO_WAIT);
	if (err) {
		return err;
	}

	evt->base = MP_EVENT_DEF(MP_SIGNAL_EPT_CONNECT, 1);
	evt->ept = ept;

	k_queue_append(&mp->evtq, evt);
	return 0;
}

int mp_ept_disconnect(struct mp_ept *ept)
{
	int err;
	struct mp_event_ept_disconnect *evt;

	if (ept->mp == NULL) {
		return -ENOTCONN;
	}

	err = k_mem_slab_alloc(&evt_slab_s, (void **)&evt, K_NO_WAIT);
	if (err) {
		return err;
	}

	evt->base = MP_EVENT_DEF(MP_SIGNAL_EPT_DISCONNECT, 1);
	evt->ept = ept;

	k_queue_append(&mp->evtq, evt);
	return 0;
}

int mp_ept_send(struct mp_ept *ept)
{

}
