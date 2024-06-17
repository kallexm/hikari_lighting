#ifndef REQW_H__
#define REQW_H__

#include <zephyr/kernel.h>

#define REQW_STATE_FREE       (0x02UL)
#define REQW_STATE_AQUIRED    (0x04UL)
#define REQW_STATE_WAITING    (0x08UL)
#define REQW_STATE_PROCESSING (0x20UL)
#define REQW_STATE_DONE       (0x40UL)
#define REQW_STATE_ABORTED    (0x80UL)

struct reqw {
	sys_snode_t node;
	void *ctx;
	atomic_t state;
	atomic_t refcount;
	struct k_event evt;
};

struct reqw_list {
	sys_slist_t list;
	uint32_t flags;
	k_sem *const sem;
	struct reqw *const _nodes;
	const uint32_t _num_nodes;
};

#define _REQW_LIST_DEFINE(_name, _size, _static)                                                   \
	static K_SEM_DEFINE(_name##_reqw_sem, 1, 1);                                               \
	static struct reqw _name##_reqw_nodes[_size];                                              \
	_static struct reqw_list _name = {                                                         \
		.flags = 0UL, .sem = &(_name##_reqw_sem),                                          \
		._nodes = &(_name##_reqw_nodes), ._num_nodes = (_size),                            \
	}

#define REQW_LIST_DEFINE(_name, _size) _REQW_LIST_DEFINE(_name, _size, (static))
#define REQW_LIST_STATIC_DEFINE(_name, _size) _REQW_LIST_DEFINE(_name, _size, ())

void reqw_init(struct reqw_list *l);

struct reqw *reqw_aquire(struct reqw_list *l);

int reqw_wait(struct reqw *b, k_timeout_t timeout);
void reqw_cancel(struct reqw *b);

int reqw_begin(struct reqw *b);
int reqw_finish(struct reqw *b);

#endif /* REQW_H__ */
