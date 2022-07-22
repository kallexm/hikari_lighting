#ifndef GLOW_FUNC_H__
#define GLOW_FUNC_H__

#include <stdint.h>

/*
	ym = 0.5 * (yh + yl);
	yd = yh - yl;
*/

struct glow_func_conf {
	float a;  /* Time constant */
	float b;  /* Gain constant */
	float ym; /* Middle value of window */
	float yd; /* Width of window */
};

struct glow_func {
	struct glow_func_conf conf;

	/* States */
	float y1; /* First state, usually holds the previous value */
	uint32_t t1; /* Time of previous calculation (milliseconds) */
};

struct glow_func *glow_func_create(const struct glow_func_conf *config);
void glow_func_destroy(struct glow_func *gf);

void glow_func_reset(struct glow_func *gf, const struct glow_func_conf *config);

/* 
 * param[in] t Time in milliseconds
 * param[in] w Random value between 0 and 1
 *
 * Return Glow function output value y
 */
float glow_func_process(struct glow_func *gf, uint32_t t, float w);

#endif /* GLOW_FUNCTION_H__ */
