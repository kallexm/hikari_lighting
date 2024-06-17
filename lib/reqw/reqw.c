#include <reqw.h>

#include <refctr.h>

#define LIST_FLAG_INITIALIZED (0x40000000UL)

#define EVT_PROCESSING_STARTED (0x00000010UL)
#define EVT_PROCESSING_DONE (0x00000080UL)

void reqw_init(struct reqw_list *l)
{
	int err;

	__ASSERT(l != NULL, "NULL pointer");

	/* Ensure thread safe access to reqw list. */
	err = k_sem_take(l->sem, K_FOREVER);
	if (err) {
		__ASSERT(false, "Semaphore take failed");
		k_oops();
	}

	/* Check if already initialized. If not, set initialized flag. */
	if (l->flags & LIST_FLAG_INITIALIZED) {
		goto exit;
	}
	l->flags |= LIST_FLAG_INITIALIZED;

	sys_slist_init(&l->list);

	__ASSERT(l->_nodes != NULL, "NULL pointer");
	__ASSERT(l->_num_nodes > 0, "Must be more than zero nodes");

	/* Prepare nodes and append them to list. */
	for (int i = l->_num_nodes-1, struct reqw *b = &l->_nodes[i]; i >= 0; --i) {
		b->ctx = NULL;
		b->state = ATOMIC_INIT(REQW_STATE_FREE);
		b->refcount = ATOMIC_INIT(0);
		k_event_init(&b->evt);

		sys_slist_append(&l->list, &b->node);
	}

exit:
	k_sem_give(l->sem);
}

struct reqw *reqw_aquire(struct reqw_list *l)
{
	int err;
	sys_snode_t *n;
	atomic_val_t s;
	struct reqw *b = NULL;

	__ASSERT(l != NULL, "NULL pointer");

	/* Ensure thread safe access to reqw list. */
	err = k_sem_take(l->sem, K_FOREVER);
	if (err != 0) {
		__ASSERT(false, "Semaphore take failed");
		k_oops();
	}

	n = sys_slist_get(&l->list);
	if (n == NULL) {
		/* Either (1) all nodes are in use -OR- (2) reqw list has not been initialized. */
		goto exit;
	}
	b = SYS_SLIST_CONTAINER(n, b, node);

	/* Set state to AQUIRED. If asserts are enabled, check that previous state was FREE. */
	__ASSERT_EVAL((void)atomic_set(&b->state, RRB_STATE_AQUIRED),
		atomic_val_t s = atomic_set(&b->state, RRB_STATE_AQUIRED),
		s == RRB_STATE_FREE, "Aquired element was not FREE");

	/* Reset context, refcount and event structure to ensure they have correct values. */
	b->ctx = NULL;
	b->refcount = ATOMIC_INIT(0);
	err = refctr_init(&b->refcount);
	if (err != 0) {
		__ASSERT(false, "Refcounter init failed");
		k_oops();
	}
	(void)k_event_clear(&b->evt, EVT_PROCESSING_STARTED | EVT_PROCESSING_DONE);

exit:
	k_sem_give(l->sem);
	return b;
}

void reqw_free(void)
{
	// Clear context.
	// Set state to free.
	// Add to list.
}

int reqw_wait(struct reqw *b, k_timeout_t timeout)
{
	int err;
	int retval;
	uint32_t events;
	atomic_val_t state;

	/* Increase reference to ensure that reqw is not recycled before function returns. */
	err = refctr_inc(&b->refcount);
	if (err != 0) {
		if (err == REFCTR_NOT_ACTIVE) {
			//reqw is freed
			return -1;
		} else if (err == REFCTR_IN_CLEANUP) {
			//reqw is in aborted or in done
			return -2;
		} else {
			__ASSERT(false, "Refcounter unknown error 0x%08x", err);
			k_oops();
		}
	}

	retval = -99;

	/* Pre-check state before wait. First thread sets state to waiting. */
retry1:
	state = atomic_get(&b->state);
	switch (state) {
	case REQW_STATE_FREE:
		retval = -3;
		goto exit;
	case REQW_STATE_AQUIRED:
		if (!atomic_cas(&b->state, state, REQW_STATE_WAITING)) {
			goto retry1;
		}
		break;
	case REQW_STATE_WAITING:
		break;
	case REQW_STATE_PROCESSING:
		timeout = K_FOREVER;
		break;
	case REQW_STATE_DONE:
		retval = 0;
		goto exit;
	case REQW_STATE_ABORTED:
		retval = -4;
		goto exit;
	default:
		__ASSERT(false, "State unknown, 0x%08x", state);
		k_sleep(1);
		goto retry1;
	}

	/* Wait for done or abort event, or handle wait timeout. */
wait:
	events = k_event_wait(&b->evt, EVT_PROCESSING_DONE, false, timeout);
	if (events == 0) {
		/* Timed out while waiting for event.
		 * If state is still WAITING, then abort.
		 * If state is PROCESSING, then wait forever on proessing done event.
		 */
retry2:
		state = atomic_get(&b->state);
		switch (state) {
		case REQW_STATE_FREE:
			__ASSERT(false, "State should never be FREE here");
			k_oops();
		case REQW_STATE_AQUIRED:
			__ASSERT(false, "State should never be AQUIRED here");
			k_oops();
		case REQW_STATE_WAITING:
			if (!atomic_cas(&b->state, state, REQW_STATE_ABORTED)) {
				goto retry2;
			}
			err = refctr_cleanup(&b->refcount);
			if (err != 0) {
				__ASSERT(err != REFCTR_NOT_ACTIVE, "Refcounter should never be inactive here");
				__ASSERT(err != REFCTR_IN_CLEANUP, "Refcounter should never have a second call to cleanup here");
				__ASSERT(err != REFCTR_DO_RECYCLE, "Refcounter should never recycle here. At least one ref should exist");
				__ASSERT(err == REFCTR_NOT_ACTIVE || err == REFCTR_IN_CLEANUP ||
					 err == REFCTR_DO_RECYCLE || err == 0, "Refcounter unknown error 0x%08x", err);
				k_oops();
			}
			(void)k_event_post(&b->evt, EVT_PROESSING_DONE);
			retval = -5;
			goto exit;
		case REQW_STATE_PROCESSING:
			timeout = K_FORVER;
			goto wait;
		case REQW_STATE_DONE:
			retval = 0;
			goto exit;
		case REQW_STATE_ABORTED:
			retval = -6;
			goto exit;
		default:
			__ASSERT(false, "State unknown, 0x%08x", state);
			k_sleep(1);
			goto retry2;
		}
	} else {
		/* Received done or abort event before timing out. */
		__ASSERT(events == EVT_PROCESSING_DONE, "Unexpected event 0x%08x", events);
retry3:
		state = atomic_get(&b->state);
		switch (state) {
		case REQW_STATE_FREE:
			__ASSERT(false, "State should never be FREE here");
			k_oops();
		case REQW_STATE_AQUIRED:
			__ASSERT(false, "State should never be AQUIRED here");
			k_oops();
		case REQW_STATE_WAITING:
			__ASSERT(false, "State should never be WAITING here");
			k_oops();
		case REQW_STATE_PROCESSING:
			__ASSERT(false, "State should never be PROCESSING here");
			k_oops();
		case REQW_STATE_DONE:
			retval = 0;
			goto exit;
		case REQW_STATE_ABORTED:
			retval = -7;
			goto exit;
		default:
			__ASSERT(false, "State unknown, 0x%08x", state);
			k_sleep(1);
			goto retry3;
		}
	}

exit:
	/* Decrease reference count and recycle reqw if last reference. */
	err = refctr_dec(&b->refcount);
	if (err != 0) {
		if (err == REFCTR_NOT_ACTIVE) {
			__ASSERT(false, "Refcounter should never be inactive here");
			k_oops();
		} else if (err == REFCTR_CANNOT_DEC) {
			__ASSERT(false, "Refcounter should always be able to decrement here");
			k_oops();
		} else if (err == REFCTR_DO_RECYCLE) {
retry4:
			state = atomic_get(&b->state);
			switch (state) {
			case REQW_STATE_FREE:
				__ASSERT(false, "State should never be FREE here");
				k_oops();
			case REQW_STATE_AQUIRED:
				__ASSERT(false, "State should never be AQUIRED here");
				k_oops();
			case REQW_STATE_WAITING:
				__ASSERT(false, "State should never be WAITING here");
				k_oops();
			case REQW_STATE_PROCESSING:
				__ASSERT(false, "State should never be PROCESSING here");
				k_oops();
			case REQW_STATE_DONE:
			case REQW_STATE_ABORTED:
				// Clear context.
				// Set state to free.
				// Add to list.
				reqw_free();
				break;
			default:
				__ASSERT(false, "State unknown, 0x%08x", state);
				k_sleep(1);
				goto retry4;
			}
		} else {
			__ASSERT(false, "Refcounter unknown error 0x%08x", err);
			k_oops();
		}
	}

	return 0;
}

void reqw_cancel(struct reqw *b)
{
	int err;
	atomic_val_t state;

retry1:
	state = atomic_get(&b->state);
	switch (state) {
	case REQW_STATE_FREE:
		return 0;
	case REQW_STATE_AQUIRED:
		if (!atomic_cas(&b->state, state, REQW_STATE_ABORTED)) {
			goto retry1;
		}
		break;
	case REQW_STATE_WAITING:
		if (!atomic_cas(&b->state, state, REQW_STATE_ABORTED)) {
			goto retry1;
		}
		break;
	case REQW_STATE_PROCESSING:
		return 0;
	case REQW_STATE_DONE:
		return 0;
	case REQW_STATE_ABORTED:
		return 0;
	default:
		__ASSERT(false, "State unknown, 0x%08x", state);
		k_sleep(1);
		goto retry1;
	}

	err = refctr_cleanup(&b->refcount);
	if (err != 0) {
		if (err == REFCTR_NOT_ACTIVE) {
			__ASSERT(false, "Refcounter should never be inactive here");
			k_oops();
		} else if (err == REFCTR_IN_CLEANUP) {
			__ASSERT(false, "Refcounter should never have a second call to cleanup here");
			k_oops();
		} else if (err == REFCTR_DO_RECYCLE) {
retry2:
			state = atomic_get(&b->state);
			switch (state) {
			case REQW_STATE_FREE:
				__ASSERT(false, "State should never be FREE here");
				k_oops();
			case REQW_STATE_AQUIRED:
				__ASSERT(false, "State should never be AQUIRED here");
				k_oops();
			case REQW_STATE_WAITING:
				__ASSERT(false, "State should never be WAITING here");
				k_oops();
			case REQW_STATE_PROCESSING:
				__ASSERT(false, "State should never be PROCESSING here");
				k_oops();
			case REQW_STATE_DONE:
				__ASSERT(false, "State should never be DONE here");
				k_oops();
			case REQW_STATE_ABORTED:
				// Clear context.
				// Set state to free.
				// Add to list.
				reqw_free();
				return 0;
			default:
				__ASSERT(false, "State unknown, 0x%08x", state);
				k_sleep(1);
				goto retry2;
			}
		} else {
			__ASSERT(false, "Refcounter unknown error 0x%08x", err);
			k_oops();
		}
	}

	(void)k_event_post(&b->evt, EVT_PROESSING_DONE);
	return 0;
}

int reqw_begin(struct reqw *b)
{
	int err;
	int retval;

	err = refctr_inc(&b->refcount);
	if (err != 0) {
		if (err == REFCTR_NOT_ACTIVE) {
			//reqw is freed
			return -1;
		} else if (err == REFCTR_IN_CLEANUP) {
			//reqw is in aborted or in done
			return -2;
		} else {
			__ASSERT(false, "Refcounter unknown error 0x%08x", err);
			k_oops();
		}
	}

retry1:
	state = atomic_get(&b->state);
	switch (state) {
	case REQW_STATE_FREE:
		retval = -3;
		goto exit;
	case REQW_STATE_AQUIRED:
		if (!atomic_cas(&b->state, state, REQW_STATE_PROCESSING)) {
			goto retry1;
		}
		break;
	case REQW_STATE_WAITING:
		if (!atomic_cas(&b->state, state, REQW_STATE_PROCESSING)) {
			goto retry1;
		}
		break;
	case REQW_STATE_PROCESSING:
		retval = -4;
		goto exit;
	case REQW_STATE_DONE:
		retval = -5;
		goto exit;
	case REQW_STATE_ABORTED:
		retval = -6;
		goto exit;
	default:
		__ASSERT(false, "State unknown, 0x%08x", state);
		k_sleep(1);
		goto retry1;
	}

	return 0;

exit:
	/* Decrease reference count and recycle reqw if last reference. */
	err = refctr_dec(&b->refcount);
	if (err != 0) {
		if (err == REFCTR_NOT_ACTIVE) {
			__ASSERT(false, "Refcounter should never be inactive here");
			k_oops();
		} else if (err == REFCTR_CANNOT_DEC) {
			__ASSERT(false, "Refcounter should always be able to decrement here");
			k_oops();
		} else if (err == REFCTR_DO_RECYCLE) {
retry2:
			state = atomic_get(&b->state);
			switch (state) {
			case REQW_STATE_FREE:
				__ASSERT(false, "State should never be FREE here");
				k_oops();
			case REQW_STATE_AQUIRED:
				__ASSERT(false, "State should never be AQUIRED here");
				k_oops();
			case REQW_STATE_WAITING:
				__ASSERT(false, "State should never be WAITING here");
				k_oops();
			case REQW_STATE_PROCESSING:
				__ASSERT(false, "State should never be PROCESSING here");
				k_oops();
			case REQW_STATE_DONE:
			case REQW_STATE_ABORTED:
				// Clear context.
				// Set state to free.
				// Add to list.
				reqw_free();
				break;
			default:
				__ASSERT(false, "State unknown, 0x%08x", state);
				k_sleep(1);
				goto retry2;
			}
		} else {
			__ASSERT(false, "Refcounter unknown error 0x%08x", err);
			k_oops();
		}
	}

	return 0;
}

int reqw_finish(struct reqw *b)
{
	int err;
	atomic_val_t state;

retry1:
	state = atomic_get(&b->state);
	switch (state) {
	case REQW_STATE_FREE:
		return -1;
	case REQW_STATE_AQUIRED:
		return -2;
	case REQW_STATE_WAITING:
		return -3;
	case REQW_STATE_PROCESSING:
		if (!atomic_cas(&b->state, state, REQW_STATE_DONE)) {
			goto retry1;
		}
		break;
	case REQW_STATE_DONE:
		return -4;
	case REQW_STATE_ABORTED:
		return -5;
	default:
		__ASSERT(false, "State unknown, 0x%08x", state);
		k_sleep(1);
		goto retry1;
	}

	err = refctr_cleanup(&b->refcount);
	if (err != 0) {
		__ASSERT(err != REFCTR_NOT_ACTIVE, "Refcounter should never be inactive here");
		__ASSERT(err != REFCTR_IN_CLEANUP, "Refcounter should never have a second call to cleanup here");
		__ASSERT(err != REFCTR_DO_RECYCLE, "Refcounter should never recycle here. At least one ref should exist");
		__ASSERT(err == REFCTR_NOT_ACTIVE || err == REFCTR_IN_CLEANUP ||
			 err == REFCTR_DO_RECYCLE || err == 0, "Refcounter unknown error 0x%08x", err);
		k_oops();
	}
	(void)k_event_post(&b->evt, EVT_PROESSING_DONE);

	/* Decrease reference count and recycle reqw if last reference. */
	err = refctr_dec(&b->refcount);
	if (err != 0) {
		if (err == REFCTR_NOT_ACTIVE) {
			__ASSERT(false, "Refcounter should never be inactive here");
			k_oops();
		} else if (err == REFCTR_CANNOT_DEC) {
			__ASSERT(false, "Refcounter should always be able to decrement here");
			k_oops();
		} else if (err == REFCTR_DO_RECYCLE) {
retry2:
			state = atomic_get(&b->state);
			switch (state) {
			case REQW_STATE_FREE:
				__ASSERT(false, "State should never be FREE here");
				k_oops();
			case REQW_STATE_AQUIRED:
				__ASSERT(false, "State should never be AQUIRED here");
				k_oops();
			case REQW_STATE_WAITING:
				__ASSERT(false, "State should never be WAITING here");
				k_oops();
			case REQW_STATE_PROCESSING:
				__ASSERT(false, "State should never be PROCESSING here");
				k_oops();
			case REQW_STATE_DONE:
			case REQW_STATE_ABORTED:
				// Clear context.
				// Set state to free.
				// Add to list.
				reqw_free();
				break;
			default:
				__ASSERT(false, "State unknown, 0x%08x", state);
				k_sleep(1);
				goto retry2;
			}
		} else {
			__ASSERT(false, "Refcounter unknown error 0x%08x", err);
			k_oops();
		}
	}

	return 0;
}
