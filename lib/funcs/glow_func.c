#include "glow_func.h"

#include <zephyr.h>

static void reset(struct glow_func *gf, const struct glow_func_conf *conf)
{
	gf->conf.a = conf->a;
	gf->conf.b = conf->b;
	gf->conf.ym = conf->ym;
	gf->conf.yd = conf->yd;

	gf->y1 = conf->ym;
	gf->t1 = 0;
}

struct glow_func *glow_func_create(const struct glow_func_conf *config)
{
	struct glow_func *gf = k_malloc(sizeof(struct glow_func));
	__ASSERT(gf != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	reset(gf, config);

	return gf;
}

void glow_func_destroy(struct glow_func *gf)
{
	k_free(gf);
}

void glow_func_reset(struct glow_func *gf, const struct glow_func_conf *config)
{
	if (config != NULL) {
		reset(gf, config);
	} else {
		reset(gf, &(gf->conf));
	}
}

float glow_func_process(struct glow_func *gf, uint32_t t, float w)
{
	float dt;
	float p1;
	float p2;
	float y;
	struct glow_func_conf *params;
	
	if (gf == NULL) {
		return 0.0;
	}

	params = &(gf->conf);

	if (gf->t1 == 0) {
		gf->t1 = t;
		gf->y1 = params->ym;
		return params->ym;
	}

	/* Calculate timestep */
	dt = (t - gf->t1)/1000.0;
	if (t < gf->t1) {
		dt = -dt;
	}

	if (w < 0 || 1 < w) {
		/* Collapse into low-pass filter with middle value ym */
		w = 0.5;
	}

	/* Calculate output value */
	p1 = dt * (params->a + params->b);
	p2 = dt * params->b * params->yd;
	y = gf->y1 + p1 * (params->ym - gf->y1) + p2 * (w - 0.5);

	/* Update states */
	gf->t1 = t;
	gf->y1 = y;
	return y;
}
