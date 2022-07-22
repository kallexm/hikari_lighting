#ifndef RGB_HSV_H__
#define RGB_HSV_H__

#include <stdint.h>

struct rgb_value {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct hsv_value {
	float h; // [0,360]
	float s; // [0,1]
	float v; // [0,1]
};

struct rgb_value hsv2rgb(const struct hsv_value val);

struct hsv_value rgb2hsv(const struct rgb_value val);

#endif /* RGB_HSV_H__ */
