#ifndef LIGHT_RESOURCE_H__
#define LIGHT_RESOURCE_H__

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/zephyr.h>

typedef enum {
	LIGHT_RESOURCE_SUCCESS = 0,
	LIGHT_RESOURCE_NOT_FOUND = 1,
	LIGHT_RESOURCE_ALLREADY_USED = 2,
	LIGHT_RESOURCE_INSUFFICIENT_MEMORY = 3,
	LIGHT_RESOURCE_NOT_INITIALIZED = 4,
} light_res_err_t;

struct light_resource {
	sys_dnode_t root;
	char *id;
	uint8_t *data;
	size_t data_size;
	bool used;
};

void light_resource_init(void);

light_res_err_t light_resource_use(char *id, struct light_resource **res);

light_res_err_t light_resource_return(struct light_resource *res);

#endif /* LIGHT_RESOURCE_H__ */
