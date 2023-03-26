#include <string.h>

#include <zephyr/zephyr.h>

#include "adrledrgb.h"
#include "light_resource.h"

static sys_dlist_t resources;
static bool is_initialized = false;

#define LIGHT_RESOURCE_UPDATE_PERIOD_MS 25

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

		id_already_exist = !strcmp(res_elem->id, id);
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
#define CHAIN_1_NUM   4
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

	rgb_t *c10 = &(chain_1.rgb_values[0]);
	rgb_t *c11 = &(chain_1.rgb_values[1]);
	rgb_t *c12 = &(chain_1.rgb_values[2]);
	rgb_t *c13 = &(chain_1.rgb_values[3]);

	c10->red = 255;
	c10->green = 0;
	c10->blue = 0;

	c11->red = 0;
	c11->green = 255;
	c11->blue = 0;

	c12->red = 0;
	c12->green = 0;
	c12->blue = 255;

	c13->red = 85;
	c13->green = 85;
	c13->blue = 85;

	do {
		ret = adrledrgb_update_leds(&chain_1);
		if (ret) {
			k_msleep(1);
		}
	} while (ret != 0);

	ret = 0;
	ret = register_resource("L1", (uint8_t *)c10, sizeof(rgb_t));
	if (!ret) {
		k_oops();
	}
	ret = register_resource("L2", (uint8_t *)c11, sizeof(rgb_t));
	if (!ret) {
		k_oops();
	}
	ret = register_resource("L3", (uint8_t *)c12, sizeof(rgb_t));
	if (!ret) {
		k_oops();
	}
	ret = register_resource("L4", (uint8_t *)c13, sizeof(rgb_t));
	if (!ret) {
		k_oops();
	}
}

/*==============================[Light Update Thread]========================*/
K_SEM_DEFINE(light_init_sem, 0, 1);

#define LIGHT_UPDATE_THREAD_STACK_SIZE 512
#define LIGHT_UPDATE_THREAD_PRIORITY 5
#define LIGHT_UPDATE_THREAD_START_DELAY_MS 500

static void light_update_loop(void *p1, void *p2, void *p3)
{
	int ret;

	k_sem_take(&light_init_sem, K_FOREVER);

	while (1) {
		do {
			ret = adrledrgb_update_leds(&chain_1);
			if (ret) {
				k_msleep(1);
			}
		} while (ret != 0);

		k_sleep(K_MSEC(LIGHT_RESOURCE_UPDATE_PERIOD_MS));
	}
}

K_THREAD_DEFINE(light_update_thread, LIGHT_UPDATE_THREAD_STACK_SIZE,
				light_update_loop, NULL, NULL, NULL,
				LIGHT_UPDATE_THREAD_PRIORITY, 0,
				LIGHT_UPDATE_THREAD_START_DELAY_MS);

/*==============================[Public methods]==============================*/
void light_resource_init(void)
{
	if (is_initialized) {
		return;
	}
	is_initialized = true;

	sys_dlist_init(&resources);

	setup_light_resources();

	k_sem_give(&light_init_sem);
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
