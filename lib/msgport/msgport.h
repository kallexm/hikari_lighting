#ifndef MSGPORT_H__
#define MSGPORT_H__

#include <hsm.h>

struct mp {
	struct hsm super;
	struct k_queue evtq;
	struct k_thread thrd;
	sys_dlist_t epts;
	/* msgport specific data and extended state */
};

struct mp_ept {
	struct hsm super;
	sys_dnode_t node;
	struct mp *mp;
	bool auto_connect;
	/* endpoint specific data and extended state */
};

/*
mp_ept events will only be serviced by a thread if registered to an mp.
if not registered to mp, thread calling function must service event?
*/

struct mpe_msg {
	void *data;
	size_t len;
	void (*cb)(void *ctx);
	void *ctx;
};

#endif /* HSGPORT_H__ */

/*
k_queue
K_QUEUE_DEFINE
k_queue_init
k_queue_cancel_wait
k_queue_append
k_queue_alloc_append
k_queue_prepend
k_queue_alloc_prepend
k_queue_insert
k_queue_append_list
k_queue_merge_slist
k_queue_get
k_queue_remove
k_queue_unique_append
k_queue_is_empty
k_queue_peek_head
k_queue_peek_tail

k_fifo
K_FIFO_DEFINE
k_fifo_init
k_fifo_cancel_wait
k_fifo_put
k_fifo_alloc_put
k_fifo_put_list
k_fifo_put_slist
k_fifo_get
k_fifo_is_empty
k_fifo_peek_head
k_fifo_peek_tail

k_lifo
K_LIFO_DEFINE
k_lifo_init
k_lifo_put
k_lifo_alloc_put
k_lifo_get

*/