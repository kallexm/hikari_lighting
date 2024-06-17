#include <refctr.h>

#include <zephyr/sys/__assert.h>
#include <zephyr/sys/atomic.h>

int refctr_init(atomic_t *refctr)
{
	__ASSERT(refctr != NULL, "Ptr is NULL");

	/* Changing value from 0 to 1 is only allowed here. */
	if (atomic_cas(refctr, 0, 1)) {
		/* Ready to be used. */
		return 0;
	}
	return REFCTR_IN_USE;
}

int refctr_cleanup(atomic_t *refctr)
{
	atomic_val_t val;

	__ASSERT(refctr != NULL, "Ptr is NULL");
retry:
	val = atomic_get(refctr);
	if (val == 0) {
		/* Refcount not in use. */
		return REFCTR_NOT_ACTIVE;
	} else if (val < 0) {
		/* cleanup already started. */
		return REFCTR_IN_CLEANUP;
	}

	/* Attempt to flip value to negative to stop new increments and start cleanup. */
	if (atomic_cas(refctr, val, -val+1)) {
		/* Cleanup started. Recycle immediately if there are no refs. */
		return (val == 1 ? REFCTR_DO_RECYCLE : 0);
	}

	/* Retry if value has changed between get and cas. */
	goto retry;
}

int refctr_inc(atomic_t *refctr)
{
	atomic_val_t val;

	__ASSERT(refctr != NULL, "Ptr is NULL");
retry:
	val = atomic_get(refctr);
	if (val == 0) {
		/* Cannot increase refcount. Not in use. */
		return REFCTR_NOT_ACTIVE;
	} else if (val < 0) {
		/* Cannot increase refcount. Cleanup ongoing. */
		return REFCTR_IN_CLEANUP;
	}

	/* Attempt to increase refcount. */
	if (atomic_cas(refctr, val, val+1)) {
		/* Refcount increased by one. */
		return 0;
	}

	/* Retry if value has changed between get and cas. */
	goto retry;
}


int refctr_dec(atomic_t *refctr)
{
	atomic_val_t val;

	__ASSERT(refctr != NULL, "Ptr is NULL");
retry:
	val = atomic_get(refctr);
	if (val == 0) {
		/* Cannot decrease refcount. Not in use. */
		return REFCTR_NOT_ACTIVE;
	} else if (val == 1) {
		/* Decreasing refcount not allowed if value is 1. */
		return REFCTR_CANNOT_DEC;
	}

	/* Attempt to "decrease" refcount one step towards zero. */
	if (atomic_cas(refctr, val, (val > 0) ? val-1 : val+1)) {
		/* Refcount decreased by one. Recycle if all refs are gone. */
		return (val == -1 ? REFCTR_DO_RECYCLE : 0);
	}

	/* Retry if value has changed between get and cas. */
	goto retry;
}
