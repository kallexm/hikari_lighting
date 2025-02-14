#ifndef HIKARI_LIGHT_H__
#define HIKARI_LIGHT_H__

#include <zephyr/kernel.h>

#include <stdint.h>

enum hikari_light_mode {
	HIKARI_LIGHT_MODE_GLOW = 1,
	HIKARI_LIGHT_MODE_SOLE,
	HIKARI_LIGHT_MODE_OFF,
	HIKARI_LIGHT_MODE_WAVE,
};

struct hikari_light_mode_api {
	void (*constructor)(void);
	void (*destructor)(void);
	void (*tweak_color)(float hue);
	void (*tweak_intensity)(float saturation);
	void (*tweak_gain)(float value);
	void (*tweak_speed)(float speed);
};

struct hikari_light_mode_entry {
	enum hikari_light_mode mode;
	struct hikari_light_mode_api *api;
};

#define DEFINE_HIKARI_LIGHT_MODE(name, _mode, _api) \
	static STRUCT_SECTION_ITERABLE(hikari_light_mode_entry, name) = { \
		.mode = _mode, .api = &_api \
	}

void hikari_light_mode_set(enum hikari_light_mode mode);
enum hikari_light_mode hikari_light_mode_get(void);

void hikari_light_tweak_color(float hue);
void hikari_light_tweak_intensity(float saturation);
void hikari_light_tweak_gain(float value);
void hikari_light_tweak_speed(float speed);

#endif /* HIKARI_LIGHT_H__ */
