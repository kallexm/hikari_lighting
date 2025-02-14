#include <string.h>
#include <unity.h>

#include "hsm.h"

#include <zephyr/sys/printk.h>

extern int unity_main(void);

#define FUNC_UNUSED __attribute__((unused))

#define STATIC_EVT_DEF(_sig) { .signal = (_sig), ._dynamic = 0UL }

enum test_signals {
	SIGNAL_A = HSM_FIRST_USER_SIGNAL,
	SIGNAL_B,
	SIGNAL_C,
	SIGNAL_D,
	SIGNAL_E
};

struct event_E {
	struct hsm_event super;
	char sym;
};

struct hsm_event event_a = STATIC_EVT_DEF(SIGNAL_A);
struct hsm_event event_b = STATIC_EVT_DEF(SIGNAL_B);
struct hsm_event event_c = STATIC_EVT_DEF(SIGNAL_C);
struct hsm_event event_d = STATIC_EVT_DEF(SIGNAL_D);
struct event_E event_e = { .super = STATIC_EVT_DEF(SIGNAL_E), .sym = 'Z' };

#define EXPECT_BUF_SIZE 128
static char expect_buffer[EXPECT_BUF_SIZE];
static uint32_t expect_buffer_idx;

static void exp_reset(void)
{
	memset(expect_buffer, 0, EXPECT_BUF_SIZE);
	expect_buffer_idx = 0;
}

static void exp_add(const char *msg)
{
	const uint32_t len = strlen(msg);

	if (len >= EXPECT_BUF_SIZE - expect_buffer_idx) {
		TEST_FAIL();
	}

	memcpy(&expect_buffer[expect_buffer_idx], msg, len+1);
	expect_buffer_idx += len;
}

static void exp_assert_equal(const char *msg)
{
	TEST_ASSERT_EQUAL_STRING(msg, &expect_buffer[0]);
}

static FUNC_UNUSED void exp_print(void)
{
	printk("expect_buffer: %s\n", expect_buffer);
}

void setUp(void)
{
	exp_reset();
}

void tearDown(void)
{
}

extern int generic_suiteTearDown(int num_failures);

int test_suiteTearDown(int num_failures)
{
	return generic_suiteTearDown(num_failures);
}

/*==============================[Tests]=======================================*/
static hsm_state state_deep7(void *me, const struct hsm_event *e);
static hsm_state state_deep6(void *me, const struct hsm_event *e);
static hsm_state state_deep5(void *me, const struct hsm_event *e);
static hsm_state state_deep4(void *me, const struct hsm_event *e);
static hsm_state state_deep3(void *me, const struct hsm_event *e);
static hsm_state state_deep2(void *me, const struct hsm_event *e);
static hsm_state state_deep1(void *me, const struct hsm_event *e);

static hsm_state state_deep8(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_ENTRY_SIGNAL:
		exp_add("e8");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("x8");
		return HSM_HANDLED();
	case SIGNAL_A:
		exp_add("a");
		return HSM_TRANSITION(&state_deep1);
	default:
		return HSM_PARENT(&state_deep7);
	}
}

static hsm_state state_deep7(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_INIT_SIGNAL:
		exp_add("i7");
		return HSM_TRANSITION(&state_deep8);
	case HSM_ENTRY_SIGNAL:
		exp_add("e7");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("x7");
		return HSM_HANDLED();
	default:
		return HSM_PARENT(&state_deep6);
	}
}

static hsm_state state_deep6(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_INIT_SIGNAL:
		exp_add("i6");
		return HSM_TRANSITION(&state_deep7);
	case HSM_ENTRY_SIGNAL:
		exp_add("e6");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("x6");
		return HSM_HANDLED();
	default:
		return HSM_PARENT(&state_deep5);
	}
}

static hsm_state state_deep5(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_INIT_SIGNAL:
		exp_add("i5");
		return HSM_TRANSITION(&state_deep6);
	case HSM_ENTRY_SIGNAL:
		exp_add("e5");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("x5");
		return HSM_HANDLED();
	case SIGNAL_C:
		exp_add("c");
		return HSM_TRANSITION(&state_deep4);
	default:
		return HSM_PARENT(&state_deep4);
	}
}

static hsm_state state_deep4(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_INIT_SIGNAL:
		exp_add("i4");
		return HSM_TRANSITION(&state_deep5);
	case HSM_ENTRY_SIGNAL:
		exp_add("e4");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("x4");
		return HSM_HANDLED();
	case SIGNAL_B:
		exp_add("b");
		return HSM_TRANSITION(&state_deep4);
	case SIGNAL_D:
		exp_add("d");
		return HSM_HANDLED();
	default:
		return HSM_PARENT(&state_deep3);
	}
}

static hsm_state state_deep3(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_INIT_SIGNAL:
		exp_add("i3");
		return HSM_TRANSITION(&state_deep4);
	case HSM_ENTRY_SIGNAL:
		exp_add("e3");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("x3");
		return HSM_HANDLED();
	default:
		return HSM_PARENT(&state_deep2);
	}
}

static hsm_state state_deep2(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_INIT_SIGNAL:
		exp_add("i2");
		return HSM_TRANSITION(&state_deep3);
	case HSM_ENTRY_SIGNAL:
		exp_add("e2");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("x2");
		return HSM_HANDLED();
	default:
		return HSM_PARENT(&state_deep1);
	}
}

static hsm_state state_deep1(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_INIT_SIGNAL:
		exp_add("i1");
		return HSM_TRANSITION(&state_deep2);
	case HSM_ENTRY_SIGNAL:
		exp_add("e1");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("x1");
		return HSM_HANDLED();
	default:
		return HSM_PARENT(&hsm_top);
	}
}

static hsm_state state_deep_initial(void *me, const struct hsm_event *e)
{
	return HSM_TRANSITION(&state_deep1);
}

void test_hsm_deep(void)
{
	struct hsm deep;

	HSM_constructor(&deep, &state_deep_initial);

	hsm_init(&deep, &event_a);
	exp_assert_equal("e1i1e2i2e3i3e4i4e5i5e6i6e7i7e8");

	exp_reset();
	hsm_dispatch(&deep, &event_a);
	exp_assert_equal("ax8x7x6x5x4x3x2i1e2i2e3i3e4i4e5i5e6i6e7i7e8");

	exp_reset();
	hsm_dispatch(&deep, &event_b);
	exp_assert_equal("bx8x7x6x5x4e4i4e5i5e6i6e7i7e8");

	exp_reset();
	hsm_dispatch(&deep, &event_c);
	exp_assert_equal("cx8x7x6x5i4e5i5e6i6e7i7e8");

	exp_reset();
	hsm_dispatch(&deep, &event_d);
	exp_assert_equal("d");


}

static hsm_state state_hs_l2(void *me, const struct hsm_event *e);
static hsm_state state_hs_l3(void *me, const struct hsm_event *e);
static hsm_state state_hs_r2(void *me, const struct hsm_event *e);
static hsm_state state_hs_r3(void *me, const struct hsm_event *e);
static hsm_state state_hs_r4(void *me, const struct hsm_event *e);

static hsm_state state_hs_1(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_INIT_SIGNAL:
		exp_add("i1");
		return HSM_TRANSITION(&state_hs_l2);
	case HSM_ENTRY_SIGNAL:
		exp_add("e1");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("x1");
		return HSM_HANDLED();
	default:
		return HSM_PARENT(&hsm_top);
	}
}

static hsm_state state_hs_l2(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_INIT_SIGNAL:
		exp_add("il2");
		return HSM_TRANSITION(&state_hs_l3);
	case HSM_ENTRY_SIGNAL:
		exp_add("el2");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("xl2");
		return HSM_HANDLED();
	default:
		return HSM_PARENT(&state_hs_1);
	}
}

static hsm_state state_hs_l3(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_ENTRY_SIGNAL:
		exp_add("el3");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("xl3");
		return HSM_HANDLED();
	case SIGNAL_A:
		exp_add("a");
		return HSM_TRANSITION(&state_hs_r3);
	case SIGNAL_B:
		exp_add("b");
		return HSM_TRANSITION(&state_hs_r4);
	default:
		return HSM_PARENT(&state_hs_l2);
	}
}

static hsm_state state_hs_r2(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_INIT_SIGNAL:
		exp_add("ir2");
		return HSM_TRANSITION(&state_hs_r3);
	case HSM_ENTRY_SIGNAL:
		exp_add("er2");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("xr2");
		return HSM_HANDLED();
	default:
		return HSM_PARENT(&state_hs_1);
	}
}

static hsm_state state_hs_r3(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_INIT_SIGNAL:
		exp_add("ir3");
		return HSM_TRANSITION(&state_hs_r4);
	case HSM_ENTRY_SIGNAL:
		exp_add("er3");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("xr3");
		return HSM_HANDLED();
	case SIGNAL_A:
		exp_add("a");
		return HSM_TRANSITION(&state_hs_l2);
	case SIGNAL_B:
		exp_add("b");
		return HSM_TRANSITION(&state_hs_l3);
	default:
		return HSM_PARENT(&state_hs_r2);
	}
}

static hsm_state state_hs_r4(void *me, const struct hsm_event *e)
{
	switch (e->signal) {
	case HSM_ENTRY_SIGNAL:
		exp_add("er4");
		return HSM_HANDLED();
	case HSM_EXIT_SIGNAL:
		exp_add("xr4");
		return HSM_HANDLED();
	default:
		return HSM_PARENT(&state_hs_r3);
	}
}

static hsm_state state_hs_initial(void *me, const struct hsm_event *e)
{
	return HSM_TRANSITION(&state_hs_1);
}

void test_hsm_horseshoe(void)
{
	struct hsm horseshoe;

	HSM_constructor(&horseshoe, &state_hs_initial);

	hsm_init(&horseshoe, NULL);
	exp_assert_equal("e1i1el2il2el3");

	exp_reset();
	hsm_dispatch(&horseshoe, &event_a);
	exp_assert_equal("axl3xl2er2er3ir3er4");

	exp_reset();
	hsm_dispatch(&horseshoe, &event_a);
	exp_assert_equal("axr4xr3xr2el2il2el3");

	exp_reset();
	hsm_dispatch(&horseshoe, &event_b);
	exp_assert_equal("bxl3xl2er2er3er4");

	exp_reset();
	hsm_dispatch(&horseshoe, &event_b);
	exp_assert_equal("bxr4xr3xr2el2el3");
}

/*============================================================================*/

extern int unity_main(void);

int main(void)
{
	return unity_main();
}
