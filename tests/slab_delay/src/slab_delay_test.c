#include <unity.h>

#include "slabs/slab_delay.h"
#include "mock_slab.h"
#include "mock_slab_event.h"

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
void test_slab_delay_create(void)
{
	const uint32_t delay_periods = 50;
	struct slab *s;
	struct slab_delay *sd;

	s = slab_delay_create(delay_periods);
	sd = (struct slab_delay *)s;
	TEST_ASSERT_NOT_NULL(sd->queue);
	TEST_ASSERT_EQUAL(delay_periods, sd->length);
	TEST_ASSERT_EQUAL(0, sd->idx);
	for (int i = 0; i < delay_periods; i++) {
		TEST_ASSERT_NULL(sd->queue[i]);
	}

	slab_delay_destroy(s);
}

void test_slab_delay_stim_rgb(void)
{
	const uint32_t delay_periods = 2;
	struct slab *s;
	struct slab_delay *sd;
	const uint32_t num_evts = 3;
	struct slab_event *evt[num_evts];
	struct slab_event dummy_evt[num_evts];

	/* Create dummy RGB event to test with */
	for (int i = 0; i < num_evts; i++) {
		dummy_evt[i].id = SLAB_EVENT_RGB;
		dummy_evt[i].num_refs = 0;
		evt[i] = &dummy_evt[i];
	}
	
	s = slab_delay_create(delay_periods);
	sd = (struct slab_delay *)s;

	/* Queue first event as number 0 in queue. */
	slab_delay_stim(s, evt[0]);
	TEST_ASSERT_EQUAL(1, sd->idx);
	TEST_ASSERT_EQUAL(evt[0], sd->queue[0]);

	/* Queue second event as number 1 in queue. */
	slab_delay_stim(s, evt[1]);
	TEST_ASSERT_EQUAL(0, sd->idx);
	TEST_ASSERT_EQUAL(evt[1], sd->queue[1]);

	/* Queue third event as number 0 in queue and expect
	 * the previous number 0 event to be forwarded to childs.
	 */
	__wrap_slab_stim_childs_Expect(s, evt[0]);
	slab_delay_stim(s, evt[2]);
	TEST_ASSERT_EQUAL(1, sd->idx);
	TEST_ASSERT_EQUAL(evt[2], sd->queue[0]);

	slab_delay_destroy(s);
}

void test_slab_delay_stim_hsv(void)
{
	const uint32_t delay_periods = 2;
	struct slab *s;
	struct slab_delay *sd;
	const uint32_t num_evts = 4;
	struct slab_event *evt[num_evts];
	struct slab_event dummy_evt[num_evts];

	/* Create dummy HSV events to test with */
	for (int i = 0; i < num_evts; i++) {
		dummy_evt[i].id = SLAB_EVENT_HSV;
		dummy_evt[i].num_refs = 0;
		evt[i] = &dummy_evt[i];
	}
	
	s = slab_delay_create(delay_periods);
	sd = (struct slab_delay *)s;

	/* Queue first event as number 0 in queue. */
	slab_delay_stim(s, evt[0]);
	TEST_ASSERT_EQUAL(1, sd->idx);
	TEST_ASSERT_EQUAL(evt[0], sd->queue[0]);

	/* Queue second event as number 1 in queue. */
	slab_delay_stim(s, evt[1]);
	TEST_ASSERT_EQUAL(0, sd->idx);
	TEST_ASSERT_EQUAL(evt[1], sd->queue[1]);

	/* Queue third event as number 0 in queue and expect
	 * the previous number 0 event to be forwarded to childs.
	 */
	__wrap_slab_stim_childs_Expect(s, evt[0]);
	slab_delay_stim(s, evt[2]);
	TEST_ASSERT_EQUAL(1, sd->idx);
	TEST_ASSERT_EQUAL(evt[2], sd->queue[0]);

	/* Queue fourth event as number 1 in queue and expect
	 * the previous number 1 event to be forwarded to childs.
	 */
	__wrap_slab_stim_childs_Expect(s, evt[1]);
	slab_delay_stim(s, evt[3]);
	TEST_ASSERT_EQUAL(0, sd->idx);
	TEST_ASSERT_EQUAL(evt[3], sd->queue[1]);

	slab_delay_destroy(s);
}


void test_slab_delay_stim_reset(void)
{
	const uint32_t delay_periods = 2;
	struct slab *s;
	struct slab_delay *sd;
	const uint32_t num_evts = 5;
	struct slab_event *evt[num_evts];
	struct slab_event dummy_evt[num_evts];

	/* Create dummy events to test with. */
	enum slab_event_id event_types[] = {
		SLAB_EVENT_RGB,
		SLAB_EVENT_HSV,
		SLAB_EVENT_HSV,
		SLAB_EVENT_RGB,
		SLAB_EVENT_RESET
	};
	for (int i = 0; i < num_evts; i++) {
		dummy_evt[i].id =  event_types[i];
		dummy_evt[i].num_refs = 0;
		evt[i] = &dummy_evt[i];
	}
	
	s = slab_delay_create(delay_periods);
	sd = (struct slab_delay *)s;

	/* Queue first event as number 0 in queue. */
	slab_delay_stim(s, evt[0]);
	TEST_ASSERT_EQUAL(1, sd->idx);
	TEST_ASSERT_EQUAL(evt[0], sd->queue[0]);

	/* Queue second event as number 1 in queue. */
	slab_delay_stim(s, evt[1]);
	TEST_ASSERT_EQUAL(0, sd->idx);
	TEST_ASSERT_EQUAL(evt[1], sd->queue[1]);

	/* Queue third event as number 0 in queue and expect
	 * the previous number 0 event to be forwarded to childs.
	 */
	__wrap_slab_stim_childs_Expect(s, evt[0]);
	slab_delay_stim(s, evt[2]);
	TEST_ASSERT_EQUAL(1, sd->idx);
	TEST_ASSERT_EQUAL(evt[2], sd->queue[0]);

	/* Queue fourth event as number 1 in queue and expect
	 * the previous number 1 event to be forwarded to childs.
	 */
	__wrap_slab_stim_childs_Expect(s, evt[1]);
	slab_delay_stim(s, evt[3]);
	TEST_ASSERT_EQUAL(0, sd->idx);
	TEST_ASSERT_EQUAL(evt[3], sd->queue[1]);

	/* Fifth event is a reset. Expect queue to
	 * be cleared and index set to 0.
	 */
	__wrap_slab_stim_childs_Expect(s, evt[4]);
	slab_delay_stim(s, evt[4]);
	TEST_ASSERT_EQUAL(0, sd->idx);
	for (int i = 0; i < delay_periods; i++) {
		TEST_ASSERT_NULL(sd->queue[i]);
	}

	/* Test that queuing still works after reset. */
	slab_delay_stim(s, evt[0]);
	TEST_ASSERT_EQUAL(1, sd->idx);
	TEST_ASSERT_EQUAL(evt[0], sd->queue[0]);

	slab_delay_stim(s, evt[1]);
	TEST_ASSERT_EQUAL(0, sd->idx);
	TEST_ASSERT_EQUAL(evt[1], sd->queue[1]);

	__wrap_slab_stim_childs_Expect(s, evt[0]);
	slab_delay_stim(s, evt[2]);
	TEST_ASSERT_EQUAL(1, sd->idx);
	TEST_ASSERT_EQUAL(evt[2], sd->queue[0]);

	/* Deallocate */
	slab_delay_destroy(s);
}

void test_slab_delay_stim_default(void)
{
	const uint32_t delay_periods = 2;
	struct slab *s;
	struct slab_delay *sd;
	struct slab_event *evt;
	struct slab_event dummy_evt;

	/* Create dummy events to test with. */
	dummy_evt.id = SLAB_EVENT_TICK;
	dummy_evt.num_refs = 0;
	evt = &dummy_evt;

	s = slab_delay_create(delay_periods);
	sd = (struct slab_delay *)s;

	__wrap_slab_stim_childs_Expect(s, evt);
	slab_delay_stim(s, evt);
	TEST_ASSERT_EQUAL(0, sd->idx);

	slab_delay_destroy(s);
}

/*============================================================================*/

extern int unity_main(void);

void main(void)
{
	(void)unity_main();
}
