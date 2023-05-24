#include "light_resource.h"
#include "slab.h"
#include "slabs/slab_led.h"

/* LED resources */
static struct light_resource *lp[2];
static struct light_resource *lc[6];
static struct light_resource *lsq[2];

static struct light_resource *lms[6];
static struct light_resource *lts[2];

static struct light_resource *llb[6];
static struct light_resource *llt[8];
static struct light_resource *lrb[6];
static struct light_resource *lrt[8];

static struct light_resource *llg[4];
static struct light_resource *lrg[4];

static struct light_resource *lls[3];
static struct light_resource *lrs[3];

/* Slabs */
static struct slab *slp[2];
static struct slab *slc[6];
static struct slab *slsq[2];

static struct slab *slms[6];
static struct slab *slts[2];

static struct slab *sllb[6];
static struct slab *sllt[8];
static struct slab *slrb[6];
static struct slab *slrt[8];

static struct slab *sllg[4];
static struct slab *slrg[4];

static struct slab *slls[3];
static struct slab *slrs[3];

#define CREATE_LED_ARRAY(_slab_array, _led_array)                                       \
	if (sizeof(_slab_array)/sizeof(struct slab *) !=                                    \
		sizeof(_led_array)/sizeof(struct light_resource *)) {                           \
		printk("slab and led resource array sizes mismatch");                           \
		k_oops();                                                                       \
	}                                                                                   \
	for (int i = 0; i < sizeof(_slab_array)/sizeof(struct slab *); i++) {               \
		_slab_array[i] = slab_create(SLAB_TYPE_LED, _led_array[i]->data, LED_TYPE_RGB); \
	}

#define DESTROY_LED_ARRAY(_slab_array)                                                  \
	for (int i = sizeof(_slab_array)/sizeof(struct slab *) - 1; i >= 0; i--) {          \
		slab_destroy(_slab_array[i]);                                                   \
	}

#define SET_LED_TYPE(_led_slab, _type)                                                  \
	((struct slab_led *)_led_slab)->led_type = _type

#define USE_ALL_HIKARI_LIGHT_RESOURCES            \
	  light_resource_use("pommel_f", &lp[0])      \
	| light_resource_use("pommel_b", &lp[1])      \
	                                              \
	| light_resource_use("core_f1", &lc[0])       \
	| light_resource_use("core_f2", &lc[1])       \
	| light_resource_use("core_f3", &lc[2])       \
	| light_resource_use("core_b1", &lc[3])       \
	| light_resource_use("core_b2", &lc[4])       \
	| light_resource_use("core_b3", &lc[5])       \
	                                              \
	| light_resource_use("square_f", &lsq[0])     \
	| light_resource_use("square_b", &lsq[1])     \
	                                              \
	| light_resource_use("midstar_f1", &lms[0])   \
	| light_resource_use("midstar_f2", &lms[1])   \
	| light_resource_use("midstar_f3", &lms[2])   \
	| light_resource_use("midstar_b1", &lms[3])   \
	| light_resource_use("midstar_b2", &lms[4])   \
	| light_resource_use("midstar_b3", &lms[5])   \
	                                              \
	| light_resource_use("tipstar_l", &lts[0])    \
	| light_resource_use("tipstar_r", &lts[1])    \
	                                              \
	| light_resource_use("Lblade_b1", &llb[0])    \
	| light_resource_use("Lblade_b2", &llb[1])    \
	| light_resource_use("Lblade_m1", &llb[2])    \
	| light_resource_use("Lblade_m2", &llb[3])    \
	| light_resource_use("Lblade_t1", &llb[4])    \
	| light_resource_use("Lblade_t2", &llb[5])    \
	                                              \
	| light_resource_use("Ltriangle_f1", &llt[0]) \
	| light_resource_use("Ltriangle_f2", &llt[1]) \
	| light_resource_use("Ltriangle_f3", &llt[2]) \
	| light_resource_use("Ltriangle_f4", &llt[3]) \
	| light_resource_use("Ltriangle_b1", &llt[4]) \
	| light_resource_use("Ltriangle_b2", &llt[5]) \
	| light_resource_use("Ltriangle_b3", &llt[6]) \
	| light_resource_use("Ltriangle_b4", &llt[7]) \
	                                              \
	| light_resource_use("Rblade_b1", &lrb[0])    \
	| light_resource_use("Rblade_b2", &lrb[1])    \
	| light_resource_use("Rblade_m1", &lrb[2])    \
	| light_resource_use("Rblade_m2", &lrb[3])    \
	| light_resource_use("Rblade_t1", &lrb[4])    \
	| light_resource_use("Rblade_t2", &lrb[5])    \
	                                              \
	| light_resource_use("Rtriangle_f1", &lrt[0]) \
	| light_resource_use("Rtriangle_f2", &lrt[1]) \
	| light_resource_use("Rtriangle_f3", &lrt[2]) \
	| light_resource_use("Rtriangle_f4", &lrt[3]) \
	| light_resource_use("Rtriangle_b1", &lrt[4]) \
	| light_resource_use("Rtriangle_b2", &lrt[5]) \
	| light_resource_use("Rtriangle_b3", &lrt[6]) \
	| light_resource_use("Rtriangle_b4", &lrt[7]) \
	                                              \
	| light_resource_use("Lguard_1", &llg[0])     \
	| light_resource_use("Lguard_2", &llg[1])     \
	| light_resource_use("Lguard_3", &llg[2])     \
	| light_resource_use("Lguard_4", &llg[3])     \
	                                              \
	| light_resource_use("Rguard_1", &lrg[0])     \
	| light_resource_use("Rguard_2", &lrg[1])     \
	| light_resource_use("Rguard_3", &lrg[2])     \
	| light_resource_use("Rguard_4", &lrg[3])     \
	                                              \
	| light_resource_use("Lspike_t", &lls[0])     \
	| light_resource_use("Lspike_b", &lls[1])     \
	| light_resource_use("Lspike", &lls[2])       \
	                                              \
	| light_resource_use("Rspike_t", &lrs[0])     \
	| light_resource_use("Rspike_b", &lrs[1])     \
	| light_resource_use("Rspike", &lrs[2])

#define CREATE_ALL_HIKARI_LIGHT_SLABS    \
	CREATE_LED_ARRAY(slp, lp);           \
	SET_LED_TYPE(slp[0], LED_TYPE_GRB);  \
	SET_LED_TYPE(slp[1], LED_TYPE_GRB);  \
	CREATE_LED_ARRAY(slc, lc);           \
	CREATE_LED_ARRAY(slsq, lsq);         \
	CREATE_LED_ARRAY(slms, lms);         \
	CREATE_LED_ARRAY(slts, lts);         \
	CREATE_LED_ARRAY(sllb, llb);         \
	CREATE_LED_ARRAY(sllt, llt);         \
	CREATE_LED_ARRAY(slrb, lrb);         \
	CREATE_LED_ARRAY(slrt, lrt);         \
	CREATE_LED_ARRAY(sllg, llg);         \
	CREATE_LED_ARRAY(slrg, lrg);         \
	CREATE_LED_ARRAY(slls, lls);         \
	CREATE_LED_ARRAY(slrs, lrs)

#define DESTROY_ALL_HIKARI_LIGHT_SLABS \
	DESTROY_LED_ARRAY(slrs);           \
	DESTROY_LED_ARRAY(slls);           \
	DESTROY_LED_ARRAY(slrg);           \
	DESTROY_LED_ARRAY(sllg);           \
	DESTROY_LED_ARRAY(slrt);           \
	DESTROY_LED_ARRAY(slrb);           \
	DESTROY_LED_ARRAY(sllt);           \
	DESTROY_LED_ARRAY(sllb);           \
	DESTROY_LED_ARRAY(slts);           \
	DESTROY_LED_ARRAY(slms);           \
	DESTROY_LED_ARRAY(slsq);           \
	DESTROY_LED_ARRAY(slc);            \
	DESTROY_LED_ARRAY(slp)

#define RETURN_ALL_HIKARI_LIGHT_RESOURCES \
	  light_resource_return(lp[0])        \
	| light_resource_return(lp[1])        \
	                                      \
	| light_resource_return(lc[0])        \
	| light_resource_return(lc[1])        \
	| light_resource_return(lc[2])        \
	| light_resource_return(lc[3])        \
	| light_resource_return(lc[4])        \
	| light_resource_return(lc[5])        \
	                                      \
	| light_resource_return(lsq[0])       \
	| light_resource_return(lsq[1])       \
	                                      \
	| light_resource_return(lms[0])       \
	| light_resource_return(lms[1])       \
	| light_resource_return(lms[2])       \
	| light_resource_return(lms[3])       \
	| light_resource_return(lms[4])       \
	| light_resource_return(lms[5])       \
	                                      \
	| light_resource_return(lts[0])       \
	| light_resource_return(lts[1])       \
	                                      \
	| light_resource_return(llb[0])       \
	| light_resource_return(llb[1])       \
	| light_resource_return(llb[2])       \
	| light_resource_return(llb[3])       \
	| light_resource_return(llb[4])       \
	| light_resource_return(llb[5])       \
	                                      \
	| light_resource_return(llt[0])       \
	| light_resource_return(llt[1])       \
	| light_resource_return(llt[2])       \
	| light_resource_return(llt[3])       \
	| light_resource_return(llt[4])       \
	| light_resource_return(llt[5])       \
	| light_resource_return(llt[6])       \
	| light_resource_return(llt[7])       \
	                                      \
	| light_resource_return(lrb[0])       \
	| light_resource_return(lrb[1])       \
	| light_resource_return(lrb[2])       \
	| light_resource_return(lrb[3])       \
	| light_resource_return(lrb[4])       \
	| light_resource_return(lrb[5])       \
	                                      \
	| light_resource_return(lrt[0])       \
	| light_resource_return(lrt[1])       \
	| light_resource_return(lrt[2])       \
	| light_resource_return(lrt[3])       \
	| light_resource_return(lrt[4])       \
	| light_resource_return(lrt[5])       \
	| light_resource_return(lrt[6])       \
	| light_resource_return(lrt[7])       \
	                                      \
	| light_resource_return(llg[0])       \
	| light_resource_return(llg[1])       \
	| light_resource_return(llg[2])       \
	| light_resource_return(llg[3])       \
	                                      \
	| light_resource_return(lrg[0])       \
	| light_resource_return(lrg[1])       \
	| light_resource_return(lrg[2])       \
	| light_resource_return(lrg[3])       \
	                                      \
	| light_resource_return(lls[0])       \
	| light_resource_return(lls[1])       \
	| light_resource_return(lls[2])       \
	                                      \
	| light_resource_return(lrs[0])       \
	| light_resource_return(lrs[1])       \
	| light_resource_return(lrs[2])
