#include <unity.h>

#include "slabs/slab_glower.h"
#include "mock_slab.h"
#include "mock_slab_event.h"
#include "mock_slab_event_hsv.h"
#include "mock_slab_event_tick.h"
#include "mock_glow_func.h"
#include "zephyr/random/rand32.h"

extern int unity_main(void);

void setUp(void)
{
	mock_slab_Init();
	mock_slab_event_Init();
}

void tearDown(void)
{
	mock_slab_Verify();
	mock_slab_event_Verify();
}

/* Suite teardown shall finalize with mandatory call to generic_suiteTearDown. */
extern int generic_suiteTearDown(int num_failures);

int test_suiteTearDown(int num_failures)
{
	return generic_suiteTearDown(num_failures);
}

/*==============================[Helpers]=====================================*/


/*==============================[Tests]=======================================*/
void test_slab_glower_create(void)
{
	struct slab *s;
	struct slab_glower *sg;
	struct slab_glower_config glower_config = {
		.hue = 152, .sat = 0.5,
		.val = { .a = 0.1, .b = 1, .ym = 0.5, .yd = 0.5 }
	};
	struct glow_func gf = { .conf = glower_config.val, .y1 = 1, .t1 = 0 };

	__wrap_glow_func_create_ExpectAndReturn(&glower_config.val, &gf);
	s = slab_glower_create(&glower_config);
	sg = (struct slab_glower *)s;
	TEST_ASSERT_EQUAL(152, sg->data.h);
	TEST_ASSERT_EQUAL(0.5, sg->data.s);
	TEST_ASSERT_EQUAL(0, sg->data.v);

	__wrap_glow_func_destroy_Expect(&gf);
	slab_glower_destroy(s);
}

void test_slab_glower_stim_tick(void)
{
	struct slab *s;
	struct slab_glower *sg;
	struct slab_glower_config glower_config = {
		.hue = 152, .sat = 0.5,
		.val = { .a = 0.1, .b = 1, .ym = 0.5, .yd = 0.5 }
	};
	struct glow_func gf = { .conf = glower_config.val, .y1 = 1, .t1 = 0 };

	struct slab_event *evt;
	struct slab_event_tick dummy_evt;
	
	dummy_evt.id = SLAB_EVENT_TICK;
	dummy_evt.num_refs = 0;
	dummy_evt.time = 2765;
	evt = (struct slab_event *)&dummy_evt;

	struct slab_event *hsv_evt;
	struct slab_event_hsv dummy_hsv_evt;
	dummy_hsv_evt.id = SLAB_EVENT_HSV;
	hsv_evt = (struct slab_event *)&dummy_hsv_evt;

	__wrap_glow_func_create_ExpectAndReturn(&glower_config.val, &gf);
	s = slab_glower_create(&glower_config);
	sg = (struct slab_glower *)s;

	/* With a native_posix test CONFIG_FAKE_ENTROPY_NATIVE_POSIX is enabled
	 * leading to psudo-random variables with a given seed. Result is that
	 * the sys_rand32_get() function will return the same sequence of values.
	 * Therefore, we hardcode the expected random value (0.3554688) that
	 * the glow_func_process() function will be called with, since it will be
	 * the same every time.
	 */
	__wrap_glow_func_process_ExpectAndReturn(&gf, 2765, 0.3554688, 0.2);
	__wrap_slab_event_create_ExpectAndReturn(SLAB_EVENT_HSV, hsv_evt);
	__wrap_slab_stim_childs_Expect(s, hsv_evt);
	__wrap_slab_stim_childs_Expect(s, evt);
	slab_glower_stim(s, evt);
	TEST_ASSERT_EQUAL_UINT8(152, sg->data.h);
	TEST_ASSERT_EQUAL_UINT8(0.5, sg->data.s);
	TEST_ASSERT_EQUAL_UINT8(0.2, sg->data.v);

	__wrap_glow_func_destroy_Expect(&gf);
	slab_glower_destroy(s);
}

void test_slab_glower_stim_reset(void)
{
	return;
}

void test_slab_glower_stim_default(void)
{
	return;
}

/*============================================================================*/

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
