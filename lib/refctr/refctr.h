#ifndef REFCTR_H__
#define REFCTR_H__

#include <zephyr/sys/atomic.h>

/** @brief Values returned by refctr functions. */
#define REFCTR_IN_USE      (0x0000195EUL) /* Refcounter is already initialized. */
#define REFCTR_NOT_ACTIVE  (0x009AC11EUL) /* Refcounter is not initialized. */
#define REFCTR_CANNOT_DEC  (0x0CA90DECUL) /* Refcounter is at its lowest value. */
#define REFCTR_IN_CLEANUP  (0x00C1EE9AUL) /* Cleanup has been invoked on Refcounter. */
#define REFCTR_DO_RECYCLE  (0x0D0C1EA9UL) /* The last reference have been removed. */

/**
 * @brief Initialize a refcounter prior to incrementing and decrementing the refcounter.
 *
 * @retval 0                  Success.
 * #retval REFCTR_IN_USE      Refcount already in use.
 */
int refctr_init(atomic_t *refctr);

/**
 * @brief Start cleanup. No more increments to the refcounter will be accepted after this.
 *        The last decrement will return REFCTR_DO_RECYCLE to indicate that resources
 *        tracked with the refcounter can be released.
 *
 * @retval 0                  Success.
 * @retval REFCTR_DO_RECYCLE  Success, The last reference have been removed.
 * #retval REFCTR_NOT_ACTIVE  Refcount not initialized.
 * #retval REFCTR_IN_CLEANUP  Cleanup has already been invoked on this refcount.
 */
int refctr_cleanup(atomic_t *refctr);

/**
 * @brief Increase the reference count of the refcounter.
 * 
 * @retval 0                  Success.
 * #retval REFCTR_NOT_ACTIVE  Refcount not initialized.
 * #retval REFCTR_IN_CLEANUP  Cleanup has already been invoked on this refcount.
 */
int refctr_inc(atomic_t *refctr);

/**
 * @brief Decrease the reference counter of the refcounter. If it is the last reference,
 *        it returns REFCTR_DO_RECYCLE to indicate that resources
 *        tracked with the refcounter can be released.
 *
 * @retval 0                  Success.
 * @retval REFCTR_DO_RECYCLE  Success, the last reference have been removed.
 * #retval REFCTR_NOT_ACTIVE  Refcount not initialized.
 * #retval REFCTR_CANNOT_DEC  Refcount cannot be decreased more.
 */
int refctr_dec(atomic_t *refctr);

#endif /* REFCTR_H__ */
