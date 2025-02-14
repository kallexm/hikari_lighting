#include <unity.h>

#include "slabs/slab_led.h"
#include "cmock_slab.h"
#include "cmock_slab_event.h"
#include "cmock_slab_event_rgb.h"
#include "cmock_rgb_hsv.h"

extern int unity_main(void);

void setUp(void)
{
	cmock_slab_Init();
	cmock_slab_event_Init();
}

void tearDown(void)
{
	cmock_slab_Verify();
	cmock_slab_event_Verify();
}

/* Suite teardown shall finalize with mandatory call to generic_suiteTearDown. */
extern int generic_suiteTearDown(int num_failures);

int test_suiteTearDown(int num_failures)
{
	return generic_suiteTearDown(num_failures);
}

/*==============================[Helpers]=====================================*/


/*==============================[Tests]=======================================*/
void test_slab_led_create(void)
{
	uint8_t led_data[3];
	struct slab *s;
	struct slab_led *sl;

	s = slab_led_create(led_data, LED_TYPE_RGB);
	sl = (struct slab_led *)s;
	TEST_ASSERT_EQUAL_PTR(led_data, sl->led);
	TEST_ASSERT_EQUAL(LED_TYPE_RGB, sl->type);

	slab_led_destroy(s);
}

void test_slab_led_stim_rgb_led_type_rgb(void)
{
	uint8_t led_data[3] = {0x0, 0x0, 0x0};
	struct slab *s;
	struct slab_led *sl;
	struct slab_event *evt;
	struct slab_event_rgb dummy_evt;

	dummy_evt.id = SLAB_EVENT_RGB;
	dummy_evt.num_refs = 0;
	dummy_evt.r = 0xA5;
	dummy_evt.g = 0xC3;
	dummy_evt.b = 0x96;
	evt = (struct slab_event *)&dummy_evt;

	s = slab_led_create(led_data, LED_TYPE_RGB);
	sl = (struct slab_led *)s;

	__cmock_slab_stim_childs_Expect(s, evt);
	slab_led_stim(s, evt);
	TEST_ASSERT_EQUAL_UINT8(0xC3, led_data[0]);
	TEST_ASSERT_EQUAL_UINT8(0xA5, led_data[1]);
	TEST_ASSERT_EQUAL_UINT8(0x96, led_data[2]);

	slab_led_destroy(s);
}

void test_slab_led_stim_rgb_led_type_grb(void)
{
	uint8_t led_data[3] = {0x0, 0x0, 0x0};
	struct slab *s;
	struct slab_led *sl;
	struct slab_event *evt;
	struct slab_event_rgb dummy_evt;

	dummy_evt.id = SLAB_EVENT_RGB;
	dummy_evt.num_refs = 0;
	dummy_evt.r = 0xA5;
	dummy_evt.g = 0xC3;
	dummy_evt.b = 0x96;
	evt = (struct slab_event *)&dummy_evt;

	s = slab_led_create(led_data, LED_TYPE_GRB);
	sl = (struct slab_led *)s;

	__cmock_slab_stim_childs_Expect(s, evt);
	slab_led_stim(s, evt);
	TEST_ASSERT_EQUAL_UINT8(0xA5, led_data[0]);
	TEST_ASSERT_EQUAL_UINT8(0xC3, led_data[1]);
	TEST_ASSERT_EQUAL_UINT8(0x96, led_data[2]);

	slab_led_destroy(s);
}

void test_slab_led_stim_rgb_led_type_red(void)
{
	uint8_t led_data[3] = {0x0, 0x0, 0x0};
	struct slab *s;
	struct slab_led *sl;
	struct slab_event *evt;
	struct slab_event_rgb dummy_evt;

	dummy_evt.id = SLAB_EVENT_RGB;
	dummy_evt.num_refs = 0;
	dummy_evt.r = 0xA5;
	dummy_evt.g = 0xC3;
	dummy_evt.b = 0x96;
	evt = (struct slab_event *)&dummy_evt;

	s = slab_led_create(led_data, LED_TYPE_RED);
	sl = (struct slab_led *)s;

	__cmock_slab_stim_childs_Expect(s, evt);
	slab_led_stim(s, evt);
	TEST_ASSERT_EQUAL_UINT8(0xA5, led_data[0]);
	TEST_ASSERT_EQUAL_UINT8(0x0, led_data[1]);
	TEST_ASSERT_EQUAL_UINT8(0x0, led_data[2]);

	slab_led_destroy(s);
}

void test_slab_led_stim_rgb_led_type_green(void)
{
	uint8_t led_data[3] = {0x0, 0x0, 0x0};
	struct slab *s;
	struct slab_led *sl;
	struct slab_event *evt;
	struct slab_event_rgb dummy_evt;

	dummy_evt.id = SLAB_EVENT_RGB;
	dummy_evt.num_refs = 0;
	dummy_evt.r = 0xA5;
	dummy_evt.g = 0xC3;
	dummy_evt.b = 0x96;
	evt = (struct slab_event *)&dummy_evt;

	s = slab_led_create(led_data, LED_TYPE_GREEN);
	sl = (struct slab_led *)s;

	__cmock_slab_stim_childs_Expect(s, evt);
	slab_led_stim(s, evt);
	TEST_ASSERT_EQUAL_UINT8(0xC3, led_data[0]);
	TEST_ASSERT_EQUAL_UINT8(0x0, led_data[1]);
	TEST_ASSERT_EQUAL_UINT8(0x0, led_data[2]);

	slab_led_destroy(s);
}

void test_slab_led_stim_rgb_led_type_blue(void)
{
	uint8_t led_data[3] = {0x0, 0x0, 0x0};
	struct slab *s;
	struct slab_led *sl;
	struct slab_event *evt;
	struct slab_event_rgb dummy_evt;

	dummy_evt.id = SLAB_EVENT_RGB;
	dummy_evt.num_refs = 0;
	dummy_evt.r = 0xA5;
	dummy_evt.g = 0xC3;
	dummy_evt.b = 0x96;
	evt = (struct slab_event *)&dummy_evt;

	s = slab_led_create(led_data, LED_TYPE_BLUE);
	sl = (struct slab_led *)s;

	__cmock_slab_stim_childs_Expect(s, evt);
	slab_led_stim(s, evt);
	TEST_ASSERT_EQUAL_UINT8(0x96, led_data[0]);
	TEST_ASSERT_EQUAL_UINT8(0x0, led_data[1]);
	TEST_ASSERT_EQUAL_UINT8(0x0, led_data[2]);

	slab_led_destroy(s);
}

void test_slab_led_stim_rgb_null(void)
{
	struct slab *s;
	struct slab_led *sl;
	struct slab_event *evt;
	struct slab_event_rgb dummy_evt;

	dummy_evt.id = SLAB_EVENT_RGB;
	dummy_evt.num_refs = 0;
	dummy_evt.r = 0xA5;
	dummy_evt.g = 0xC3;
	dummy_evt.b = 0x96;
	evt = (struct slab_event *)&dummy_evt;

	s = slab_led_create(NULL, LED_TYPE_RGB);
	sl = (struct slab_led *)s;

	__cmock_slab_stim_childs_Expect(s, evt);
	slab_led_stim(s, evt);

	slab_led_destroy(s);
}

void test_slab_led_stim_hsv(void)
{
	/* Test that a hsv event sets the correct rgb values of the led */
	TEST_IGNORE();
}

void test_slab_led_stim_reset(void)
{
	/* Test that a reset event sets all led values to default */
	TEST_IGNORE();
}

void test_slab_led_stim_default(void)
{
	uint8_t led_data[3] = {0xA5, 0xC3, 0x96};
	struct slab *s;
	struct slab_led *sl;
	struct slab_event *evt;
	struct slab_event dummy_evt;

	dummy_evt.id = SLAB_EVENT_TICK;
	dummy_evt.num_refs = 0;
	evt = &dummy_evt;

	s = slab_led_create(led_data, LED_TYPE_RGB);
	sl = (struct slab_led *)s;

	__cmock_slab_stim_childs_Expect(s, evt);
	slab_led_stim(s, evt);
	TEST_ASSERT_EQUAL_UINT8(0xA5, led_data[0]);
	TEST_ASSERT_EQUAL_UINT8(0xC3, led_data[1]);
	TEST_ASSERT_EQUAL_UINT8(0x96, led_data[2]);

	slab_led_destroy(s);
}

/*============================================================================*/

extern int unity_main(void);

int main(void)
{
	return unity_main();
}
