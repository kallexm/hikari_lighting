#include "wave_func.h"

#include <zephyr/kernel.h>

static void reset(struct wave_func *wf, const struct wave_func_conf *conf)
{
	wf->conf.T = conf->T;
	wf->conf.ym = conf->ym;
	wf->conf.yd = conf->yd;

	wf->t0 = 0;
}

struct wave_func *wave_func_create(const struct wave_func_conf *config)
{
	struct wave_func *wf = k_malloc(sizeof(struct wave_func));
	__ASSERT(wf != NULL, "System heap too small. Increase CONFIG_HEAP_MEM_POOL_SIZE");

	reset(wf, config);

	return wf;
}

void wave_func_destroy(struct wave_func *wf)
{
	k_free(wf);
}

void wave_func_reset(struct wave_func *wf, const struct wave_func_conf *config)
{
	if (config != NULL) {
		reset(wf, config);
	} else {
		reset(wf, &(wf->conf));
	}
}

float wave_func_process(struct wave_func *wf, uint32_t t)
{
	float y;
	uint32_t t1;
	uint32_t t2;
	struct wave_func_conf *params;
	
	if (wf == NULL) {
		return 0.0;
	}

	params = &(wf->conf);

	t1 = params->T + wf->t0;
	t2 = params->T + t1;

	/* update state t0 */
	if (wf->t0 == 0 || t >= t2) {
		wf->t0 = t;
	}

	/* Calculate output value */
	if (t < wf->t0) {
		y = params->ym - params->yd;
	} else if (t < t1) {
		y = params->ym + (2.0f/(params->T)*(t - wf->t0) - 1) * params->yd;
	} else if (t < t2) {
		y = params->ym - (2.0f/(params->T)*(t - wf->t0 - params->T) - 1) * params->yd;
	} else {
		y = params->ym - params->yd;
	}

	return y;
}
