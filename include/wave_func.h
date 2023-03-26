#ifndef WAVE_FUNC_H__
#define WAVE_FUNC_H__

#include <stdint.h>

/*
	ym = 0.5 * (yh + yl);
	yd = yh - yl;
*/

struct wave_func_conf {
	uint32_t T; /* Period in miliseconds */
	float ym;   /* Middle value of window */
	float yd;   /* Width of window */
};

struct wave_func {
	struct wave_func_conf conf;

	/* States */
	uint32_t t0;
};

struct wave_func *wave_func_create(const struct wave_func_conf *config);
void wave_func_destroy(struct wave_func *wf);

void wave_func_reset(struct wave_func *wf, const struct wave_func_conf *config);

/* 
 * param[in] t Time in milliseconds
 *
 * Return Wave function output value y
 */
float wave_func_process(struct wave_func *wf, uint32_t t);

#endif /* WAVE_FUNCTION_H__ */
