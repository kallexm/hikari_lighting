#include <zephyr/kernel.h>

#include "hikari_light.h"

#define HIKARI_LIGHT_THREAD_STACK_SIZE 2048
#define HIKARI_LIGHT_THREAD_PRIORITY 5
#define HIKARI_LIGHT_THREAD_START_DELAY_MS 500

/*==============================[Action FIFO]=================================*/
static K_FIFO_DEFINE(action_fifo);

enum action_type {
	ACTION_TYPE_MODE_SET = 1,
	ACTION_TYPE_TWEAK_COLOR,
	ACTION_TYPE_TWEAK_INTENSITY,
	ACTION_TYPE_TWEAK_GAIN,
	ACTION_TYPE_TWEAK_SPEED
};

struct action_item {
	void *fifo_reserved;
	enum action_type type;
	union {
		enum hikari_light_mode mode;
		float hue;
		float saturation;
		float value;
		float speed;
	};
};

static void empty_action_fifo(void)
{
	struct action_item *item;

	do {
		item = k_fifo_get(&action_fifo, K_NO_WAIT);
	} while (item != NULL);
}

/*==============================[Singleton members]===========================*/
static enum hikari_light_mode mode;
static struct hikari_light_mode_api *api = NULL;

static K_SEM_DEFINE(num_threads_posting_actions, 0, K_SEM_MAX_LIMIT);
static bool accepting_new_actions = true;


/*==============================[Helper methods]==============================*/
static void post_action(enum action_type type, void *action_data)
{
	struct action_item *action = NULL;

	printk("Action posted: %d\n", type);

	/* Return if new actions are not to be accepted. */
	if (!accepting_new_actions) {
		return;
	}

	/* Count number of threads currently in the process of posting actions. */
	k_sem_give(&num_threads_posting_actions);

	/* Create and compose action. */
	action = k_malloc(sizeof(struct action_item));
	if (action == NULL) {
		k_sem_take(&num_threads_posting_actions, K_NO_WAIT);
		k_oops();
	}

	switch (type) {
	case ACTION_TYPE_MODE_SET:
		action->mode = *((enum hikari_light_mode *)action_data);
		break;

	case ACTION_TYPE_TWEAK_COLOR:
		action->hue = *((float *)action_data);
		break;

	case ACTION_TYPE_TWEAK_INTENSITY:
		action->saturation = *((float *)action_data);
		break;

	case ACTION_TYPE_TWEAK_GAIN:
		action->value = *((float *)action_data);
		break;

	case ACTION_TYPE_TWEAK_SPEED:
		action->speed = *((float *)action_data);
		break;

	default:
		/* Cleanup and silently return if action is unsupported. */
		k_free(action);
		k_sem_take(&num_threads_posting_actions, K_NO_WAIT);
		return;
	}
	action->type = type;

	printk("Add to action queue\n");

	/* Queue action. */
	k_fifo_put(&action_fifo, action);

	/* Signal that thread is done posting an action. */
	k_sem_take(&num_threads_posting_actions, K_NO_WAIT);
}

static struct hikari_light_mode_api *get_mode_api(enum hikari_light_mode mode) 
{
	STRUCT_SECTION_FOREACH(hikari_light_mode_entry, entry) {
		printk("get_mode_api entry mode: %d\n", entry->mode);

		if (entry->mode == mode) {
			return entry->api;
		}
	}

	printk("Did not find mode %d\n", mode);
	return NULL;
}

/*==============================[Process Thread]==============================*/
static void hikari_light_loop(void *p1, void *p2, void *p3);

K_THREAD_DEFINE(hikari_light_thread, HIKARI_LIGHT_THREAD_STACK_SIZE,
				hikari_light_loop, NULL, NULL, NULL,
				HIKARI_LIGHT_THREAD_PRIORITY, 0,
				HIKARI_LIGHT_THREAD_START_DELAY_MS);

static void try_switching_mode(enum hikari_light_mode new_mode)
{
	struct hikari_light_mode_api *new_api = NULL;

	if (new_mode == mode) {
		printk("Mode %d already set\n", new_mode);
		return;
	}

	/* Check if mode is implemented. */
	new_api = get_mode_api(new_mode);
	if (new_api == NULL || 
		new_api->constructor == NULL ||
		new_api->destructor == NULL) {
		/* Do not switch mode. */

		printk("Do not switch mode\n");
		return;
	}

	printk("Switching to mode %d from %d\n", new_mode, mode);

	accepting_new_actions = false;

	/* Switch mode. */
	if (api != NULL && api->destructor != NULL) {
		api->destructor();
	}

	new_api->constructor();
	mode = new_mode;
	api = new_api;

	/* Wait for all threads to finish posting actions. */
	while (k_sem_count_get(&num_threads_posting_actions)) {
		k_sleep(K_MSEC(1));
	}

	/* Drop outdated actions. */
	empty_action_fifo();

	accepting_new_actions = true;
}

static inline void hikari_light_process(struct action_item *action)
{
	if (action == NULL) {
		k_oops();
	}

	printk("Dequeueing action: %d\n", action->type);

	switch (action->type) {
	case ACTION_TYPE_MODE_SET:
		try_switching_mode(action->mode);
		break;

	case ACTION_TYPE_TWEAK_COLOR:
		if (api->tweak_color != NULL) {
			api->tweak_color(action->hue);
		}
		break;

	case ACTION_TYPE_TWEAK_INTENSITY:
		if (api->tweak_intensity != NULL) {
			api->tweak_intensity(action->saturation);
		}
		break;

	case ACTION_TYPE_TWEAK_GAIN:
		if (api->tweak_gain != NULL) {
			api->tweak_gain(action->value);
		}
		break;

	case ACTION_TYPE_TWEAK_SPEED:
		if (api->tweak_speed != NULL) {
			api->tweak_speed(action->speed);
		}
		break;

	default:
		k_oops();
		break;
	}
}

static void hikari_light_loop(void *p1, void *p2, void *p3)
{
	struct action_item *action;

	while (1) {
		action = k_fifo_get(&action_fifo, K_FOREVER);

		hikari_light_process(action);

		k_free(action);
	}
}

/*==============================[Public methods]==============================*/
void hikari_light_mode_set(enum hikari_light_mode mode)
{
	post_action(ACTION_TYPE_MODE_SET, (void *)&mode);
}

enum hikari_light_mode hikari_light_mode_get(void)
{
	return mode;
}

void hikari_light_tweak_color(float hue)
{
	post_action(ACTION_TYPE_TWEAK_COLOR, (void *)&hue);
}

void hikari_light_tweak_intensity(float saturation)
{
	post_action(ACTION_TYPE_TWEAK_INTENSITY, (void *)&saturation);
}

void hikari_light_tweak_gain(float value)
{
	post_action(ACTION_TYPE_TWEAK_GAIN, (void *)&value);
}

void hikari_light_tweak_speed(float speed)
{
	post_action(ACTION_TYPE_TWEAK_SPEED, (void *)&speed);
}

/*============================================================================*/
