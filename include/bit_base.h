#include <stdint.h>
#include <zephyr.h>

struct bit_out_elem {
	sys_dnode_t root;
	struct bit *bit;
};

enum bit_type {
	BIT_TYPE_BASE,
	BIT_TYPE_RGB,
};

struct bit {
	sys_dlist_t out_list;
	enum bit_type type;
	
};

enum bit_evt_id {
	BIT_EVENT_RESET,
};

struct bit_event {
	enum bit_evt_id id;
};

struct bit *bit_create(void);
void bit_destroy(struct bit *bit);
void bit_connect(struct bit *bit, struct bit *connect_to);
void bit_disconnect(struct bit *bit, struct bit *disconnect_from);
void bit_stimulate(struct bit *bit, struct bit_event *evt);
