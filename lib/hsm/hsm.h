
/**
 * Hierarchical state machine (HSM) design adopted from:
 *
 * Practical UML Statecharts in C/C++, Second Edition
 * Event-Driven Programming for Embedded Systems.
 *
 * Miro Samek
 * 
 * ISBN 9780750687065
 */

#include <stdbool.h>
#include <stdint.h>

#define HSM_RETURN_HANDLED    ((hsm_state)0)
#define HSM_RETURN_IGNORED    ((hsm_state)1)
#define HSM_RETURN_TRANSITION ((hsm_state)2)
#define HSM_RETURN_PARENT     ((hsm_state)3)

/** Value to return from state handler function when event has been handled. */
#define HSM_HANDLED()                                                                              \
	(HSM_RETURN_HANDLED)
/** Value to return from state handler function when event should be ignored. */
#define HSM_IGNORED()                                                                              \
	(HSM_RETURN_IGNORED)
/** Value to return from state handler function when a state transition should be taken. */
#define HSM_TRANSITION(_target)                                                                    \
	(((struct hsm *)me)->state = (hsm_state_handler)(_target), HSM_RETURN_TRANSITION)
/** Value to return from state handler function when event should be handled by the parent state. */
#define HSM_PARENT(_parent)                                                                        \
	(((struct hsm *)me)->state = (hsm_state_handler)(_parent), HSM_RETURN_PARENT) 

/** Signals reserved for internal use by the hsm module.
 *  The last value is the first signal value that the application can use.
 */
enum hsm_reserved_signals {
	HSM_ENTRY_SIGNAL = 1,
	HSM_EXIT_SIGNAL,
	HSM_INIT_SIGNAL,

	HSM_FIRST_USER_SIGNAL
};

typedef uint32_t hsm_signal;
typedef uint32_t hsm_state;

struct hsm_event {
	/** Event type. */
	hsm_signal signal;
	/** Dynamic event attributes (zero for static event). */
	uint32_t _dynamic; 
};

typedef hsm_state (*hsm_state_handler)(void *me, const struct hsm_event *e);

struct hsm {
	/** Active state. */
	hsm_state_handler state;
};

#define HSM_constructor(_me, _initial) ((_me)->state = (_initial))

void hsm_init(struct hsm *me, const struct hsm_event *e);
void hsm_dispatch(struct hsm *me, const struct hsm_event *e);
bool hsm_is_in(struct hsm *me, hsm_state_handler state);
hsm_state hsm_top(struct hsm *me, const struct hsm_event *e);
