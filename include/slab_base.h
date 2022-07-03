#include <stdint.h>
#include <zephyr.h>

/** SLAB: Smart LED Animation Block
 */

struct slab_out_elem {
	sys_dnode_t root;
	struct slab *slab;
};

enum slab_type {
	SLAB_TYPE_BASE,
	SLAB_TYPE_RGB,
};

struct slab {
	sys_dlist_t out_list;
	enum slab_type type;
	
};

enum slab_evt_id {
	SLAB_EVENT_RESET,
};

struct slab_event {
	enum slab_evt_id id;
};

struct slab *slab_create(void);
void slab_destroy(struct slab *slab);
void slab_connect(struct slab *slab, struct slab *connect_to);
void slab_disconnect(struct slab *slab, struct slab *disconnect_from);
void slab_stimulate(struct slab *slab, struct slab_event *evt);
