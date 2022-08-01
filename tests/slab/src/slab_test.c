#include <unity.h>

#include "slab.h"
#include "mock_slab_event.h"
#include "mock_slab_led.h"
#include "mock_slab_delay.h"
#include "mock_slab_glower.h"
#include "mock_slab_hsv2rgb.h"
#include "mock_slab_rgb2hsv.h"
#include "mock_dlist.h"

extern int unity_main(void);

void setUp(void)
{
	mock_slab_event_Init();
	mock_slab_led_Init();
	mock_slab_delay_Init();
	mock_slab_glower_Init();
	mock_slab_hsv2rgb_Init();
	mock_slab_rgb2hsv_Init();
	mock_dlist_Init();
}

void tearDown(void)
{
	mock_slab_event_Verify();
	mock_slab_led_Verify();
	mock_slab_delay_Verify();
	mock_slab_glower_Verify();
	mock_slab_hsv2rgb_Verify();
	mock_slab_rgb2hsv_Verify();
	mock_dlist_Verify();
}

/* Suite teardown shall finalize with mandatory call to generic_suiteTearDown. */
extern int generic_suiteTearDown(int num_failures);

int test_suiteTearDown(int num_failures)
{
	return generic_suiteTearDown(num_failures);
}

/*==============================[Helpers]=====================================*/
/* Defined in slab.c. Need definition also here for pointer comparison. */
struct slab_child {
	sys_dnode_t root;
	struct slab *child;
};

static bool is_child_of(struct slab* slab, struct slab* parent, int num_in_list)
{
	int count = 0;
	sys_dlist_t *list;
	struct slab_child *child_node;

	list = &parent->childs;
	TEST_ASSERT_NOT_NULL(list);
	child_node = (struct slab_child *)(list->head);

	/* Last element points back to the list so we use this as stop condition. */
	while ((sys_dnode_t *)child_node != list) {
		/* Negative num_in_list means we don't care for
		 * the slab's position, only that it exists in the list.
		 */
		if (num_in_list < 0 || count == num_in_list) {
			if (child_node->child == slab) {
				return true;
			}
		}
		count++;
		child_node = (struct slab_child *)(child_node->root.next);
	}
	return false;
}

static bool no_childs(struct slab *s)
{
	if (&s->childs == s->childs.head &&
		&s->childs == s->childs.tail) {
		return true;
	}

	return false;
}

/*==============================[Tests]=======================================*/
void test_slab_create_and_destroy_led(void)
{
	uint8_t v[3];
	struct slab *s;
	struct slab_led *sl;
	struct slab *dummy_slab;
	struct slab_led dummy_led_slab;

	/* Populate led specific values of dummy slab */
	dummy_led_slab.led = v;
	dummy_led_slab.led_type = LED_TYPE_RGB;
	dummy_slab = (struct slab *)&dummy_led_slab;

	/* Test slab_create with type led */
	__wrap_slab_led_create_ExpectAndReturn(v, LED_TYPE_RGB, dummy_slab);
	s = slab_create(SLAB_TYPE_LED, v, LED_TYPE_RGB);
	TEST_ASSERT_TRUE(no_childs(s));
	TEST_ASSERT_EQUAL(SLAB_TYPE_LED, s->type);
	sl = (struct slab_led *)s;
	TEST_ASSERT_EQUAL(LED_TYPE_RGB, sl->led_type);
	TEST_ASSERT_EQUAL(v, sl->led);

	/* Test slab_destroy on the dummy slab */
	__wrap_slab_led_destroy_Expect(s);
	slab_destroy(s);
	TEST_ASSERT_TRUE(no_childs(dummy_slab));
}

void test_slab_create_and_destroy_delay(void)
{
	uint32_t delay_periods = 20;
	struct slab *s;
	struct slab_delay *sd;
	struct slab *dummy_slab;
	struct slab_delay dummy_delay_slab;

	/* Populate delay specific values of dummy slab */
	dummy_delay_slab.length = delay_periods;
	dummy_slab = (struct slab *)&dummy_delay_slab;

	/* Test slab_create with type delay */
	__wrap_slab_delay_create_ExpectAndReturn(delay_periods, dummy_slab);
	s = slab_create(SLAB_TYPE_DELAY, delay_periods);
	TEST_ASSERT_TRUE(no_childs(s));
	TEST_ASSERT_EQUAL(SLAB_TYPE_DELAY, s->type);
	sd = (struct slab_delay *)s;
	TEST_ASSERT_EQUAL(delay_periods, sd->length);

	/* Test slab_destroy on the dummy slab */
	__wrap_slab_delay_destroy_Expect(s);
	slab_destroy(s);
	TEST_ASSERT_TRUE(no_childs(dummy_slab));
}

void test_slab_create_and_destroy_glower(void)
{
	struct slab_glower_config glow_conf = {
		.hue = 123.4, .sat = 0.5, .val = {.a = 1, .b = 2, .ym = 3, .yd = 4}
	};
	struct slab *s;
	struct slab_glower *sg;
	struct slab *dummy_slab;
	struct slab_glower dummy_glower_slab;

	/* Populate glower specific values of dummy slab */
	dummy_glower_slab.gen = (void *)0xBEEFCAFE;
	dummy_slab = (struct slab *)&dummy_glower_slab;

	/* Test slab_create with type glower */
	__wrap_slab_glower_create_ExpectAndReturn(&glow_conf, dummy_slab);
	s = slab_create(SLAB_TYPE_GLOWER, &glow_conf);
	TEST_ASSERT_TRUE(no_childs(s));
	TEST_ASSERT_EQUAL(SLAB_TYPE_GLOWER, s->type);
	sg = (struct slab_glower *)s;
	TEST_ASSERT_EQUAL(0xBEEFCAFE, sg->gen);

	/* Test slab_destroy on the dummy slab */
	__wrap_slab_glower_destroy_Expect(s);
	slab_destroy(s);
	TEST_ASSERT_TRUE(no_childs(dummy_slab));
}

void test_slab_create_and_destroy_hsv2rgb(void)
{
	struct slab *s;
	struct slab dummy_slab;

	/* Test slab_create with type hsv2rgb */
	__wrap_slab_hsv2rgb_create_ExpectAndReturn(&dummy_slab);
	s = slab_create(SLAB_TYPE_HSV2RGB);
	TEST_ASSERT_TRUE(no_childs(s));
	TEST_ASSERT_EQUAL(SLAB_TYPE_HSV2RGB, s->type);

	/* Test slab_destroy on the dummy slab */
	__wrap_slab_hsv2rgb_destroy_Expect(s);
	slab_destroy(s);
	TEST_ASSERT_TRUE(no_childs(&dummy_slab));
}

void test_slab_create_and_destroy_rgb2hsv(void)
{
	struct slab *s;
	struct slab dummy_slab;

	/* Test slab_create with type rgb2hsv */
	__wrap_slab_rgb2hsv_create_ExpectAndReturn(&dummy_slab);
	s = slab_create(SLAB_TYPE_RGB2HSV);
	TEST_ASSERT_TRUE(no_childs(s));
	TEST_ASSERT_EQUAL(SLAB_TYPE_RGB2HSV, s->type);

	/* Test slab_destroy on the dummy slab */
	__wrap_slab_rgb2hsv_destroy_Expect(s);
	slab_destroy(s);
	TEST_ASSERT_TRUE(no_childs(&dummy_slab));
}

void test_slab_create_and_destroy_invalid_type(void)
{
	struct slab *s;
	uint8_t dummy_slab[sizeof(struct slab)];
	int slab_type_invalid = -1;

	s = (struct slab *)dummy_slab;
	s->type = -1;

	/* Test slab_create with invalid type should return NULL */
	s = slab_create(slab_type_invalid);
	TEST_ASSERT_NULL(s);

	/* Test slab_destroy on NULL */
	slab_destroy(NULL);

	/* Test slab destroy on random value */
	slab_destroy((struct slab *)&dummy_slab);
}

void test_slab_connect_and_disconnect(void)
{
	const int num_slabs = 2;
	struct slab *s[num_slabs];
	struct slab dummy_slab[num_slabs];

	/* Create generic slabs */
	for (int i = 0; i < num_slabs; i++) {
		__wrap_slab_hsv2rgb_create_ExpectAndReturn(&dummy_slab[i]);
		s[i] = slab_create(SLAB_TYPE_HSV2RGB);
		TEST_ASSERT_TRUE(no_childs(s[i]));
	}
	
	/* Connect slab 1 to slab 0 */
	slab_connect(s[1], s[0]);
	TEST_ASSERT_TRUE(is_child_of(s[1], s[0], 0));
	TEST_ASSERT_TRUE(no_childs(s[1]));

	/* Disconnect slab 1 from slab 0 */
	slab_disconnect(s[1], s[0]);
	TEST_ASSERT_TRUE(no_childs(s[0]));
	TEST_ASSERT_TRUE(no_childs(s[1]));
}

void test_slab_connect_and_disconnect_many(void)
{
	const int num_slabs = 5;
	struct slab *s[num_slabs];
	struct slab dummy_slab[num_slabs];

	/* Create generic slabs */
	for (int i = 0; i < num_slabs; i++) {
		__wrap_slab_hsv2rgb_create_ExpectAndReturn(&dummy_slab[i]);
		s[i] = slab_create(SLAB_TYPE_HSV2RGB);
		TEST_ASSERT_TRUE(no_childs(s[i]));
	}

	/* Connect slabs
     * s0 -> s1
	 *    -> s2 -> s3
	 *	        -> s4
	 */
	slab_connect(s[1], s[0]);
	slab_connect(s[2], s[0]);
	slab_connect(s[3], s[2]);
	slab_connect(s[4], s[2]);
	TEST_ASSERT_TRUE(is_child_of(s[1], s[0], 0));
	TEST_ASSERT_TRUE(is_child_of(s[2], s[0], 1));
	TEST_ASSERT_TRUE(is_child_of(s[3], s[2], 0));
	TEST_ASSERT_TRUE(is_child_of(s[4], s[2], 1));
	TEST_ASSERT_TRUE(no_childs(s[1]));
	TEST_ASSERT_TRUE(no_childs(s[3]));
	TEST_ASSERT_TRUE(no_childs(s[4]));

	/* Disconnect slab 1 from slab 0 */
	slab_disconnect(s[1], s[0]);
	slab_disconnect(s[2], s[0]);
	slab_disconnect(s[3], s[2]);
	slab_disconnect(s[4], s[2]);
	TEST_ASSERT_TRUE(no_childs(s[0]));
	TEST_ASSERT_TRUE(no_childs(s[1]));
	TEST_ASSERT_TRUE(no_childs(s[2]));
	TEST_ASSERT_TRUE(no_childs(s[3]));
	TEST_ASSERT_TRUE(no_childs(s[4]));
}

void test_slab_connect_and_disconnect_invalid_slab(void)
{
	const int num_slabs = 3;
	struct slab *s[num_slabs];
	struct slab dummy_slab[num_slabs];

	/* Create generic slabs */
	for (int i = 0; i < num_slabs; i++) {
		__wrap_slab_hsv2rgb_create_ExpectAndReturn(&dummy_slab[i]);
		s[i] = slab_create(SLAB_TYPE_HSV2RGB);
		TEST_ASSERT_TRUE(no_childs(s[i]));
	}

	/* Silently ignore connecting a slab to NULL */
	slab_connect(s[1], NULL);
	TEST_ASSERT_TRUE(no_childs(s[1]));

	/* Silently ignore connecting NULL to slab */
	slab_connect(NULL, s[0]);
	TEST_ASSERT_TRUE(no_childs(s[0]));

	/* Try connecting a slab twice and expect it to ignore the second attempt */
	slab_connect(s[1], s[0]);
	TEST_ASSERT_TRUE(is_child_of(s[1], s[0], 0));
	slab_connect(s[1], s[0]);
	TEST_ASSERT_FALSE(is_child_of(s[1], s[0], 1));
	slab_disconnect(s[1], s[0]);
	TEST_ASSERT_TRUE(no_childs(s[0]));

	/* Silently ignore attempt at disconnecting slab from null */
	slab_disconnect(s[1], NULL);
	TEST_ASSERT_TRUE(no_childs(s[1]));

	/* Silently ignore attempt at disconnecting NULL from slab */
	slab_connect(s[1], s[0]);
	slab_disconnect(NULL, s[0]);
	TEST_ASSERT_TRUE(is_child_of(s[1], s[0], 0));
	slab_disconnect(s[1], s[0]);
	TEST_ASSERT_TRUE(no_childs(s[0]));


	/* Silently ignore attempt at disconnecting a slab that is not connected */
	slab_connect(s[2], s[0]);
	TEST_ASSERT_TRUE(is_child_of(s[2], s[0], 0));
	slab_disconnect(s[1], s[0]);
	TEST_ASSERT_TRUE(is_child_of(s[2], s[0], 0));
	TEST_ASSERT_FALSE(is_child_of(s[1], s[0], -1));

	
}

void test_slab_stim(void)
{
	struct slab *s;
	struct slab dummy_slab;
	struct slab_event *evt;
	struct slab_event dummy_evt;

	/* Create event to test with */
	dummy_evt.id = SLAB_EVENT_RESET;
	dummy_evt.num_refs = 0;
	evt = &dummy_evt;

	/* Create generic slab */
	__wrap_slab_hsv2rgb_create_ExpectAndReturn(&dummy_slab);
	s = slab_create(SLAB_TYPE_HSV2RGB);
	TEST_ASSERT_TRUE(no_childs(s));

	/* Give an event to the slab */
	__wrap_slab_event_acquire_Expect(evt);
	__wrap_slab_hsv2rgb_stim_Expect(s, evt);
	slab_stim(s, evt);
}

void test_slab_stim_childs(void)
{
	const int num_slabs = 2;
	struct slab *s[num_slabs];
	struct slab dummy_slab[num_slabs];

	struct slab_event *evt;
	struct slab_event dummy_evt;

	/* Create event to test with */
	dummy_evt.id = SLAB_EVENT_RESET;
	dummy_evt.num_refs = 0;
	evt = &dummy_evt;

	/* Create generic slabs */
	for (int i = 0; i < num_slabs; i++) {
		__wrap_slab_hsv2rgb_create_ExpectAndReturn(&dummy_slab[i]);
		s[i] = slab_create(SLAB_TYPE_HSV2RGB);
	}

	slab_connect(s[1], s[0]);

	__wrap_slab_event_acquire_Expect(evt);
	__wrap_slab_hsv2rgb_stim_Expect(s[1], evt);
	__wrap_slab_event_release_Expect(evt);
	slab_stim_childs(s[0], evt);
}

void test_slab_stim_childs_many(void)
{
	const int num_slabs = 5;
	struct slab *s[num_slabs];
	struct slab dummy_slab[num_slabs];

	struct slab_event *evt;
	struct slab_event dummy_evt;

	/* Create event to test with */
	dummy_evt.id = SLAB_EVENT_RESET;
	dummy_evt.num_refs = 0;
	evt = &dummy_evt;

	/* Create generic slabs */
	for (int i = 0; i < num_slabs; i++) {
		__wrap_slab_hsv2rgb_create_ExpectAndReturn(&dummy_slab[i]);
		s[i] = slab_create(SLAB_TYPE_HSV2RGB);
	}

	slab_connect(s[1], s[0]);
	slab_connect(s[2], s[0]);

	slab_connect(s[3], s[2]);
	slab_connect(s[4], s[2]);

	__wrap_slab_event_acquire_Expect(evt);
	__wrap_slab_hsv2rgb_stim_Expect(s[1], evt);
	__wrap_slab_event_acquire_Expect(evt);
	__wrap_slab_hsv2rgb_stim_Expect(s[2], evt);
	__wrap_slab_event_release_Expect(evt);
	slab_stim_childs(s[0], evt);

	__wrap_slab_event_acquire_Expect(evt);
	__wrap_slab_hsv2rgb_stim_Expect(s[3], evt);
	__wrap_slab_event_acquire_Expect(evt);
	__wrap_slab_hsv2rgb_stim_Expect(s[4], evt);
	__wrap_slab_event_release_Expect(evt);
	slab_stim_childs(s[2], evt);
}

/*============================================================================*/

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
