#ifndef SLAB_H__
#define SLAB_H__

#include <stdint.h>
#include <zephyr.h>

#include "slab_event.h"

/** SLAB: Smart LED Animation Block
 */

enum slab_type {
	SLAB_TYPE_LED = 1,
	SLAB_TYPE_DELAY,
	SLAB_TYPE_TICKER,
	SLAB_TYPE_GLOWER,
	SLAB_TYPE_HSV2RGB,
	SLAB_TYPE_RGB2HSV,
	SLAB_TYPE_NOTIFIER,
};

struct slab {
	sys_dlist_t childs;
	enum slab_type type;
};

/* Call to dynamically allocate and initialize slabs of different types.
 * Based on the types, the slabs take extra arguments at creation.
 *
 * List of extra arguments:
 * SLAB_TYPE_LED:      void *led_buf, enum led_type type
 *
 * SLAB_TYPE_DELAY:    uint32_t delay_periods
 *
 * SLAB_TYPE_TICKER:   k_timeout_t tick_period
 *
 * SLAB_TYPE_GLOWER:   struct slab_glower_config *config
 *     struct slab_glower_config {
 *	       float hue [0,360]
 *	       float sat [0,1]
 *         struct glow_func_conf val;
 *     };
 *     struct glow_func_conf {
 *         float a;  [Time constant]
 *         float b;  [Gain constant]
 *         float ym; [Middle value of window]
 *         float yd; [Width of window]
 *     };
 *
 * SLAB_TYPE_HSV2RGB: No extra arguments
 *
 * SLAB_TYPE_RGB2HSV: No extra arguments
 *
 * SLAB_TYPE_NOTIFIER: slab_notifier_cb subscriber, void *context
 *     typedef void (*slab_notifier_cb)(struct slab_event *evt, void *ctx)
 */
struct slab *slab_create(enum slab_type type, ...);
void slab_destroy(struct slab *slab);

/* Connect slab to output of another slab (parent) so it will receive and
 * process events sent or forwarded by the parent slab.
 *
 * Multiple slabs can be connected to the output of another slab.
 * A slab can be connected to multiple slabs.
 */
void slab_connect(struct slab *slab, struct slab *connect_to);

/* Disconnect a slab from the output of another slab.
 *
 * This will do nothing if the slab is not connected the the parent slab.
 */
void slab_disconnect(struct slab *slab, struct slab *disconnect_from);

/* Sends an event to a slab.
 *
 * The slab will process the event, possibly sending one or more events
 * to the connected child slabs.
 */
void slab_stim(struct slab *slab, struct slab_event *evt);

 /* Send an event to the child slabs of a slab.
  *
  * This is also used internally to forward events to child slabs.
  *
  * Make sure to call slab_event_acquire() on the event before
  * calling this function. If slab_event_acquire() is not called
  * prior to this, the event will be destroyed at the end of
  * this function.
  */
void slab_stim_child(struct slab *slab, struct slab_event *evt);

#endif /* SLAB_H__ */
