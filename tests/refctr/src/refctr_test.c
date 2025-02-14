#include <string.h>
#include <unity.h>

#include <refctr.h>

#include <zephyr/sys/printk.h>

void setUp(void)
{
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
void test_refctr_init_success(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);
}

void test_refctr_init_in_use(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(REFCTR_IN_USE, ret);
}

void test_refctr_cleanup_do_recycle(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);
}

void test_refctr_cleanup_success(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);
}

void test_refctr_cleanup_not_active(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(REFCTR_NOT_ACTIVE, ret);
}

void test_refctr_cleanup_already_in_cleanup(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(REFCTR_IN_CLEANUP, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);
}

void test_refctr_inc_success(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(0, ret);
}

void test_refctr_inc_not_active(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(REFCTR_NOT_ACTIVE, ret);
}

void test_refctr_inc_already_in_cleanup(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(REFCTR_IN_CLEANUP, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);
}

void test_refctr_dec_success(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);
}

void test_refctr_dec_do_recycle(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);
}

void test_refctr_dec_not_active(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_NOT_ACTIVE, ret);
}

void test_refctr_dec_cannot_dec(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_CANNOT_DEC, ret);
}

void test_refctr_100000_refs(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	for (int i = 0; i < 100000; ++i) {
		ret = refctr_inc(&r);
		TEST_ASSERT_EQUAL(0, ret);
	}

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(0, ret);

	for (int i = 0; i < 99999; ++i) {
		ret = refctr_dec(&r);
		TEST_ASSERT_EQUAL(0, ret);
	}

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);
}

void test_refctr_reuse(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	for (int n = 5; n > 0; --n) {
		ret = refctr_init(&r);
		TEST_ASSERT_EQUAL(0, ret);

		for (int i = 0; i < 10; ++i) {
			ret = refctr_inc(&r);
			TEST_ASSERT_EQUAL(0, ret);
		}

		ret = refctr_cleanup(&r);
		TEST_ASSERT_EQUAL(0, ret);

		for (int i = 0; i < 9; ++i) {
			ret = refctr_dec(&r);
			TEST_ASSERT_EQUAL(0, ret);
		}

		ret = refctr_dec(&r);
		TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);
	}
}

void test_refctr_all_retvals(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_NOT_ACTIVE, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(REFCTR_NOT_ACTIVE, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(REFCTR_NOT_ACTIVE, ret);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_CANNOT_DEC, ret);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(REFCTR_IN_USE, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_CANNOT_DEC, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(REFCTR_IN_CLEANUP, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(REFCTR_IN_CLEANUP, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(REFCTR_IN_CLEANUP, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(REFCTR_IN_CLEANUP, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_NOT_ACTIVE, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(REFCTR_NOT_ACTIVE, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(REFCTR_NOT_ACTIVE, ret);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);
}

void test_refctr_two_different(void)
{
	int ret;
	atomic_t r = ATOMIC_INIT(0);
	atomic_t s = ATOMIC_INIT(0);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_init(&s);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_inc(&s);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&s);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_dec(&s);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);

	ret = refctr_dec(&r);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);

	ret = refctr_init(&r);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&r);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);

	ret = refctr_init(&s);
	TEST_ASSERT_EQUAL(0, ret);

	ret = refctr_cleanup(&s);
	TEST_ASSERT_EQUAL(REFCTR_DO_RECYCLE, ret);
}
/*============================================================================*/

extern int unity_main(void);

int main(void)
{
	return unity_main();
}
