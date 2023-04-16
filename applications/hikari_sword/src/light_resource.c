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
#define CHAIN_POMMEL_NUM 2
#define CHAIN_POMMEL_PIN 11 /* D2 */
#define CHAIN_POMMEL_PORT 1
RGB_CHAIN_DEF(chain_P, CHAIN_POMMEL_NUM, CHAIN_POMMEL_PIN, CHAIN_POMMEL_PORT, false);

#define CHAIN_CORE_NUM 8
#define CHAIN_CORE_PIN 12 /* D3 */
#define CHAIN_CORE_PORT 1
RGB_CHAIN_DEF(chain_C, CHAIN_CORE_NUM, CHAIN_CORE_PIN, CHAIN_CORE_PORT, true);

#define CHAIN_INNER_BLADE_NUM 8
#define CHAIN_INNER_BLADE_PIN 15 /* D4 */
#define CHAIN_INNER_BLADE_PORT 1
RGB_CHAIN_DEF(chain_IB, CHAIN_INNER_BLADE_NUM, CHAIN_INNER_BLADE_PIN, CHAIN_INNER_BLADE_PORT, true);

#define CHAIN_LEFT_BLADE_NUM 14
#define CHAIN_LEFT_BLADE_PIN 13 /* D5 */
#define CHAIN_LEFT_BLADE_PORT 1
RGB_CHAIN_DEF(chain_LB, CHAIN_LEFT_BLADE_NUM, CHAIN_LEFT_BLADE_PIN, CHAIN_LEFT_BLADE_PORT, true);

#define CHAIN_RIGHT_BLADE_NUM 14
#define CHAIN_RIGHT_BLADE_PIN 14 /* D6 */
#define CHAIN_RIGHT_BLADE_PORT 1
RGB_CHAIN_DEF(chain_RB, CHAIN_RIGHT_BLADE_NUM, CHAIN_RIGHT_BLADE_PIN, CHAIN_RIGHT_BLADE_PORT, true);

#define CHAIN_LEFT_GUARD_NUM 4
#define CHAIN_LEFT_GUARD_PIN 23 /* D7 */
#define CHAIN_LEFT_GUARD_PORT 0
RGB_CHAIN_DEF(chain_LG, CHAIN_LEFT_GUARD_NUM, CHAIN_LEFT_GUARD_PIN, CHAIN_LEFT_GUARD_PORT, true);

#define CHAIN_RIGHT_GUARD_NUM 4
#define CHAIN_RIGHT_GUARD_PIN 21 /* D8 */
#define CHAIN_RIGHT_GUARD_PORT 0
RGB_CHAIN_DEF(chain_RG, CHAIN_RIGHT_GUARD_NUM, CHAIN_RIGHT_GUARD_PIN, CHAIN_RIGHT_GUARD_PORT, true);

#define CHAIN_LEFT_SPIKE_NUM 3
#define CHAIN_LEFT_SPIKE_PIN 27 /* D9 */
#define CHAIN_LEFT_SPIKE_PORT 1
RGB_CHAIN_DEF(chain_LS, CHAIN_LEFT_SPIKE_NUM, CHAIN_LEFT_SPIKE_PIN, CHAIN_LEFT_SPIKE_PORT, false);

#define CHAIN_RIGHT_SPIKE_NUM 3
#define CHAIN_RIGHT_SPIKE_PIN 2 /* D10 */
#define CHAIN_RIGHT_SPIKE_PORT 1
RGB_CHAIN_DEF(chain_RS, CHAIN_RIGHT_SPIKE_NUM, CHAIN_RIGHT_SPIKE_PIN, CHAIN_RIGHT_SPIKE_PORT, false);

rgb_chain_t *chains[9] = {&chain_P, &chain_C, &chain_IB, &chain_LB, &chain_RB,
						  &chain_LG, &chain_RG, &chain_LS, &chain_RS};

#define INIT_COLOR(_chain, _num, _red, _green, _blue) \
    for (uint32_t i = 0; i < _num; i++) {             \
		_chain.rgb_values[i].red = _red;              \
		_chain.rgb_values[i].green = _green;          \
		_chain.rgb_values[i].blue = _blue;            \
	}

static void setup_light_resources(void)
{
	int ret;

	for (uint32_t i = 0; i < sizeof(chains)/sizeof(rgb_chain_t *); i++) {
		ret = adrledrgb_init(chains[i]);
		if (ret < 0) {
			k_oops();
		}
	}

	INIT_COLOR(chain_P, CHAIN_POMMEL_NUM, 40, 40, 40);
	INIT_COLOR(chain_C, CHAIN_CORE_NUM, 40, 40, 40);
	INIT_COLOR(chain_IB, CHAIN_INNER_BLADE_NUM, 40, 40, 40);
	INIT_COLOR(chain_LB, CHAIN_LEFT_BLADE_NUM, 40, 40, 40);
	INIT_COLOR(chain_RB, CHAIN_RIGHT_BLADE_NUM, 40, 40, 40);
	INIT_COLOR(chain_LG, CHAIN_LEFT_GUARD_NUM, 40, 40, 40);
	INIT_COLOR(chain_RG, CHAIN_RIGHT_GUARD_NUM, 40, 40, 40);
	INIT_COLOR(chain_LS, CHAIN_LEFT_SPIKE_NUM, 40, 40, 40);
	INIT_COLOR(chain_RS, CHAIN_RIGHT_SPIKE_NUM, 40, 40, 40);

	for (uint32_t i = 0; i < sizeof(chains)/sizeof(rgb_chain_t *); i++) {
		do {
			ret = adrledrgb_update_leds(chains[i]);
			if (ret) {
				k_msleep(1);
			}
		} while (ret != 0);
	}

	ret = register_resource("L1", (uint8_t *)&(chain_LG.rgb_values[0]), sizeof(rgb_t));
	if (!ret) {
		k_oops();
	}
	ret = register_resource("L2", (uint8_t *)&(chain_LG.rgb_values[1]), sizeof(rgb_t));
	if (!ret) {
		k_oops();
	}
	ret = register_resource("L3", (uint8_t *)&(chain_LG.rgb_values[2]), sizeof(rgb_t));
	if (!ret) {
		k_oops();
	}
	ret = register_resource("L4", (uint8_t *)&(chain_LG.rgb_values[3]), sizeof(rgb_t));
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

		for (uint32_t i = 0; i < sizeof(chains)/sizeof(rgb_chain_t *); i++) {
			do {
				ret = adrledrgb_update_leds(chains[i]);
				if (ret) {
					k_msleep(1);
				}
			} while (ret != 0);
		}

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
