#include <string.h>

#include <zephyr/zephyr.h>

#include "adrledrgb.h"
#include "light_resource.h"

static sys_dlist_t resources;
static bool is_initialized = false;

/*==============================[Private methods]=============================*/
static bool is_overlapping(uint8_t *p1, size_t l1, uint8_t *p2, size_t l2)
{
	bool is_separate = (p2 >= p1 + l1) || (p1 >= p2 + l2);

	return !is_separate;
}

static bool register_resource(char *id, uint8_t *data, size_t data_size)
{
	sys_dnode_t *elem;
	struct light_resource *res_elem;
	int id_already_exist;
	bool data_overlapping;
	
	SYS_DLIST_FOR_EACH_NODE(&resources, elem) {
		res_elem = CONTAINER_OF(elem, struct light_resource, root);

		id_already_exist = strcmp(res_elem->id, id);
		data_overlapping = is_overlapping(data, data_size, res_elem->data, res_elem->data_size);
		if (id_already_exist || data_overlapping) {
			return false;
		}
	}

	struct light_resource *new_res = k_malloc(sizeof(struct light_resource));
	if (new_res == NULL) {
		return false;
	}

	new_res->id = id;
	new_res->data = data;
	new_res->data_size = data_size;
	new_res->used = false;
	sys_dlist_append(&resources, &new_res->root);
	return true;
}

/*==============================[Setup resources]=============================*/
#define CHAIN_1_NUM   3
#define CHAIN_1_PIN  11
#define CHAIN_1_PORT  1
RGB_CHAIN_DEF(chain_1, CHAIN_1_NUM, CHAIN_1_PIN, CHAIN_1_PORT, true);

//#define CHAIN_2_NUM  14
//#define CHAIN_2_PIN  12
//#define CHAIN_2_PORT  1
//RGB_CHAIN_DEF(chain_2, CHAIN_2_NUM, CHAIN_2_PIN, CHAIN_2_PORT, true);

static void setup_light_resources(void)
{
	int ret;

	ret = adrledrgb_init(&chain_1);
	if (ret < 0) {
		k_oops();
	}

	//ret = adrledrgb_init(&chain_2);
	//if (ret < 0) {
	//	k_oops();
	//}

	ret = 0;
	rgb_t *c10 = &(chain_1.rgb_values[0]);
	rgb_t *c11 = &(chain_1.rgb_values[1]);
	rgb_t *c12 = &(chain_1.rgb_values[2]);
	ret |= register_resource("L1", (uint8_t *)c10, sizeof(rgb_t));
	ret |= register_resource("L2", (uint8_t *)c11, sizeof(rgb_t));
	ret |= register_resource("L3", (uint8_t *)c12, sizeof(rgb_t));
	if (ret) {
		k_oops();
	}
}

/*==============================[Public methods]==============================*/
void light_resource_init(void)
{
	if (is_initialized) {
		return;
	}
	is_initialized = true;

	sys_dlist_init(&resources);

	setup_light_resources();
}

light_res_err_t light_resource_use(char *id, struct light_resource **res)
{
	sys_dnode_t *elem;
	struct light_resource *res_elem;
	bool id_wrong;

	SYS_DLIST_FOR_EACH_NODE(&resources, elem) {
		res_elem = CONTAINER_OF(elem, struct light_resource, root);

		id_wrong = strcmp(res_elem->id, id);
		if (id_wrong) {
			continue;
		}

		if (res_elem->used) {
			*res = NULL;
			return LIGHT_RESOURCE_ALLREADY_USED;
		} else {
			res_elem->used = true;
			*res = res_elem;
			return LIGHT_RESOURCE_SUCCESS;
		}
	}

	*res = NULL;
	return LIGHT_RESOURCE_NOT_FOUND;
}

light_res_err_t light_resource_return(struct light_resource *res)
{
	sys_dnode_t *elem;
	struct light_resource *res_elem;

	SYS_DLIST_FOR_EACH_NODE(&resources, elem) {
		res_elem = CONTAINER_OF(elem, struct light_resource, root);

		if (res_elem != res) {
			continue;
		}

		res->used = false;
		return LIGHT_RESOURCE_SUCCESS;
	}

	return LIGHT_RESOURCE_NOT_FOUND;
}

/*============================================================================*/
