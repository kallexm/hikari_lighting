#include <stdbool.h>
#include <stdint.h>

#include "hsm.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/toolchain.h>

#define MAX_NEST_DEPTH ((int32_t)CONFIG_HSM_MAX_NEST_DEPTH)
#define NEST_DEPTH_MSG "Max nesting depth exceeded"
BUILD_ASSERT(MAX_NEST_DEPTH > 0, "Nesting depth must be greater than zero");

#define HSM_EMPTY_SIGNAL (0)

static const struct hsm_event reserved_events[] = {
	{ .signal = (hsm_signal)HSM_EMPTY_SIGNAL, ._dynamic = (uint32_t)0 },
	{ .signal = (hsm_signal)HSM_ENTRY_SIGNAL, ._dynamic = (uint32_t)0 },
	{ .signal = (hsm_signal)HSM_EXIT_SIGNAL, ._dynamic = (uint32_t)0 },
	{ .signal = (hsm_signal)HSM_INIT_SIGNAL, ._dynamic = (uint32_t)0 }
};

#define HSM_TRIG(_state, _signal)                                                                  \
	((*(_state))(me, &reserved_events[_signal]))

#define HSM_EXIT(_state)                                                                           \
	if (HSM_TRIG(_state, HSM_EXIT_SIGNAL) == HSM_RETURN_HANDLED) {                             \
		/* For debugging */                                                                \
	}

#define HSM_ENTER(_state)                                                                          \
	if (HSM_TRIG(_state, HSM_ENTRY_SIGNAL) == HSM_RETURN_HANDLED) {                            \
		/* For debugging */                                                                \
	}

hsm_state hsm_top(struct hsm *me, const struct hsm_event *e)
{
	ARG_UNUSED(me);
	ARG_UNUSED(e);
	return HSM_IGNORED();
}


void hsm_init(struct hsm *me, const struct hsm_event *e)
{
	int32_t ip;
	hsm_state_handler tmp, path[MAX_NEST_DEPTH];

	/* Top most transition must be taken. */
	if ((*me->state)(me, e) != HSM_RETURN_TRANSITION) k_oops();

	tmp = (hsm_state_handler)&hsm_top;
	do {
		ip = 0L;
		path[0] = me->state;
		(void)HSM_TRIG(me->state, HSM_EMPTY_SIGNAL);
		while (me->state != tmp) {
			path[++ip] = me->state;
			(void)HSM_TRIG(me->state, HSM_EMPTY_SIGNAL);
		}
		me->state = path[0];

		__ASSERT(ip < MAX_NEST_DEPTH, NEST_DEPTH_MSG);

		do {
			HSM_ENTER(path[ip]);
		} while ((--ip) >= 0L);

		tmp = path[0];
	} while (HSM_TRIG(tmp, HSM_INIT_SIGNAL) == HSM_RETURN_TRANSITION);
	me->state = tmp;
}

void hsm_dispatch(struct hsm *me, const struct hsm_event *e)
{
	hsm_state_handler src, tmp, path[MAX_NEST_DEPTH];
	hsm_state ret;
	int32_t ip, iq;

	tmp = me->state;

	do {
		src = me->state;
		ret = (*src)(me, e);
	} while (ret == HSM_RETURN_PARENT);

	if (ret == HSM_RETURN_TRANSITION) {
		ip = -1L;
		path[0] = me->state;
		path[1] = tmp;

		while (tmp != src) {
			if (HSM_TRIG(tmp, HSM_EXIT_SIGNAL) == HSM_RETURN_HANDLED) {
				(void)HSM_TRIG(tmp, HSM_EMPTY_SIGNAL);
			}
			tmp = me->state;
		}

		tmp = path[0];
		if (src == tmp) {
			HSM_EXIT(src);
			ip = 0L;
		} else {
			(void)HSM_TRIG(tmp, HSM_EMPTY_SIGNAL);
			tmp = me->state;
			if (src == tmp) {
				ip = 0L;
			} else {
				(void)HSM_TRIG(src, HSM_EMPTY_SIGNAL);

				if (me->state == tmp) {
					HSM_EXIT(src);
					ip = 0L;
				} else {
					if (me->state == path[0]) {
						HSM_EXIT(src);
					} else {
						iq = 0L;
						ip = 1L;
						path[1] = tmp;
						tmp = me->state;

						ret = HSM_TRIG(path[1], HSM_EMPTY_SIGNAL);
						while (ret == HSM_RETURN_PARENT) {
							path[++ip] = me->state;
							if (me->state == src) {
								iq = 1L;
								__ASSERT(ip < MAX_NEST_DEPTH, NEST_DEPTH_MSG);
								--ip;
								ret = HSM_RETURN_HANDLED;
							} else {
								ret = HSM_TRIG(me->state, HSM_EMPTY_SIGNAL);
							}
						}
						if (iq == 0L) {
							__ASSERT(ip < MAX_NEST_DEPTH, NEST_DEPTH_MSG);
							HSM_EXIT(src);
							iq = ip;
							ret = HSM_RETURN_IGNORED;
							do {
								if (tmp == path[iq]) {
									ret = HSM_RETURN_HANDLED;
									ip = iq - 1L;
									iq = -1L;
								} else {
									--iq;
								}
							} while (iq >= 0L);

							if (ret != HSM_RETURN_HANDLED) {
								ret = HSM_RETURN_IGNORED;
								do {
									if (HSM_TRIG(tmp, HSM_EXIT_SIGNAL) == HSM_RETURN_HANDLED) {
										(void)HSM_TRIG(tmp, HSM_EMPTY_SIGNAL);
									}
									tmp = me->state;
									iq = ip;
									do {
										if (tmp == path[iq]) {
											ip = iq - 1L;
											iq = -1L;
											ret = HSM_RETURN_HANDLED;
										} else {
											--iq;
										}
									} while (iq >= 0L);
								} while (ret != HSM_RETURN_HANDLED);
							}
						}
					}
				}
			}
		}

		for (; ip >= 0L; --ip) HSM_ENTER(path[ip]);

		tmp = path[0];
		me->state = tmp;

		while (HSM_TRIG(tmp, HSM_INIT_SIGNAL) == HSM_RETURN_TRANSITION) {
			ip = 0L;
			path[0] = me->state;
			(void)HSM_TRIG(me->state, HSM_EMPTY_SIGNAL);
			while (me->state != tmp) {
				path[++ip] = me->state;
				(void)HSM_TRIG(me->state, HSM_EMPTY_SIGNAL);
			}
			me->state = path[0];

			__ASSERT(ip < MAX_NEST_DEPTH, NEST_DEPTH_MSG);

			do {
				HSM_ENTER(path[ip]);
			} while ((--ip) >= 0L);

			tmp = path[0];
		}
	}
	me->state = tmp;
}

bool hsm_is_in(struct hsm *me, hsm_state_handler state)
{
	uint32_t lvl = 0UL;
	const hsm_state_handler tmp = me->state;
	const hsm_state_handler top = (hsm_state_handler)&hsm_top;

	if (state == top || state == tmp) return true;

	do {
		(void)HSM_TRIG(me->state, HSM_EMPTY_SIGNAL);
		if (me->state == state) {
			return (me->state = tmp, true);
		}
	} while (me->state != top && ++lvl < MAX_NEST_DEPTH);

	return (me->state = tmp, false);
}
