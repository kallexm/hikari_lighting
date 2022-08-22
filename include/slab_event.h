#ifndef SLAB_EVENT_H__
#define SLAB_EVENT_H__

#include <stdint.h>
#include <zephyr.h>

enum slab_event_id {
	SLAB_EVENT_RESET = 1,
	SLAB_EVENT_TICK,
	SLAB_EVENT_RGB,
	SLAB_EVENT_HSV,
};

struct slab_event {
	enum slab_event_id id;
	int num_refs;
};

/* Call to create events of different types. Based on the types, the
 * events takes extra arguments at creation.
 *
 * List of extra arguments:
 * SLAB_EVENT_RESET:  no extra arguments
 * SLAB_EVENT_TICK:   uint32_t time
 * SLAB_EVENT_RGB:    struct rgb_value val
 * SLAB_EVENT_HSV:    struct hsv_value val
 */
struct slab_event *slab_event_create(enum slab_event_id event_id, ...);

/* Call on an event to destroy it.
 *
 * NOTE: Events passed to a slab will automatically be destroyed
 * when the number of references to the event reaches zero.
 *
 * To keep an event even after giving it to a slab, call
 * slab_event_acquire() on the event after creation. In this case,
 * the event must be destroyed manually by slab_event_release() or
 * slab_event_destroy(). slab_event_destroy() should only be used
 * when it is certain that no slab is still keeping a reference
 * to the event.
 */
void slab_event_destroy(struct slab_event *evt);

/* Call on an event that should be kept for later processing
 * to avoid that the event will be destroyed.
 *
 * IMPORTANT: Also call on newly created events inside slabs
 * before calling slab_stim_childs() on the events.
 */
void slab_event_acquire(struct slab_event *evt);

/* Call on an event when slab_event_acquire() have previously
 * been called on the event and processing of the event is done.
 *
 * IMPORTANT: Inside slabs, call this on events that should not
 * be forwarded to child slabs with slab_stim_childs().
 */
void slab_event_release(struct slab_event *evt);

#endif /* SLAB_EVENT_H__ */
