#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include <rbuf.h>

#define PRINT_HEXDUMP(_ptr, _len, _msg)                                                            \
	do {                                                                                       \
		printk("Line %d: %s", __LINE__, _msg);                                             \
		for (uint32_t hd_i = 0; hd_i < (_len); hd_i++) {                                   \
			printk("%s0x%02hhx ", ((hd_i%10)?"":"\n\t"), ((uint8_t *)_ptr)[hd_i]);     \
		}                                                                                  \
		printk("\n");                                                                      \
	} while (0)

static const uint8_t data100[] = {
	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA,
	0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0xF0, 0xE0, 0xD0, 0xC0, 0xB0,
	0xA0, 0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
	0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x2F, 0x2E, 0x2D, 0x2C, 0x2B,
	0x2A, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21,
	0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A,
	0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x6F, 0x6E, 0x6D, 0x6C, 0x6B,
	0x6A, 0x69, 0x68, 0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61,
	0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A,
};

RBUF_DEF(buf_default, 13, 0);
RBUF_DEF(buf_100, 100, 0);
RBUF_DEF(buf_overwritable, 7, RBUF_FLAG_ALLOW_OVERWRITE);
RBUF_DEF(buf_no_partial_write, 10, RBUF_FLAG_NO_PARTIAL_WRITE);
RBUF_DEF(buf_suspended_add, 1, RBUF_FLAG_ADD_SUSPEND);
RBUF_DEF(buf_no_peeking, 14, RBUF_FLAG_NO_PEEKING);
RBUF_DEF(buf_zero_always, 13, RBUF_FLAG_ZERO_MEM_ALWAYS);
RBUF_DEF(buf_suspended_get, 2, RBUF_FLAG_GET_SUSPEND);

static struct rbuf *const rbufs[] = {
	&buf_default,
	&buf_100,
	&buf_overwritable,
	&buf_no_partial_write,
	&buf_suspended_add,
	&buf_no_peeking,
	&buf_zero_always,
	&buf_suspended_get,
};

static void check_sizes(void)
{
	int err;
	struct rbuf_sizes stats;

	ARRAY_FOR_EACH(rbufs, idx) {
		err = rbuf_sizes_get(rbufs[idx], &stats);
		zassert_equal(err, RBUF_SUCCESS);
		zassert_equal(stats.used + stats.free, stats.max);
	}
}

static void reset_rbufs(void)
{
	int err;

	ARRAY_FOR_EACH(rbufs, idx) {
		err = rbuf_reset(rbufs[idx], true);
		zassert_equal(err, RBUF_SUCCESS);
	}
}

static void rbuf_suite_before(void *fixture)
{
	ARG_UNUSED(fixture);

	reset_rbufs();
}

static void rbuf_suite_after(void *fixture)
{
	ARG_UNUSED(fixture);

	check_sizes();
}

ZTEST_SUITE(rbuf_suite, NULL, NULL, rbuf_suite_before, rbuf_suite_after, NULL);

ZTEST(rbuf_suite, test_rbuf_mixed_add_get)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 10;
	err = rbuf_add(&buf_default, data100, &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 10);

	out_len = 20;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 10);

	zassert_mem_equal(out, data100, 10);
}

ZTEST(rbuf_suite, test_rbuf_mixed_add_get_get)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 10;
	err = rbuf_add(&buf_default, data100, &len, 0);
	zassert_equal(err, RBUF_SUCCESS);

	out_len = 6;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 6);

	out_len = 4;
	err = rbuf_get(&buf_default, &out[6], &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 4);

	zassert_mem_equal(out, data100, 10);
}

ZTEST(rbuf_suite, test_rbuf_mixed_add_add_get)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 5;
	err = rbuf_add(&buf_default, data100, &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 5);

	len = 5;
	err = rbuf_add(&buf_default, &data100[5], &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 5);

	out_len = 10;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 10);

	zassert_mem_equal(out, data100, 10);
}

ZTEST(rbuf_suite, test_rbuf_mixed_add_get_100)
{
	int err;
	size_t len, out_len;
	uint8_t out[100] = {0};

	/* num: Number of bytes added and removed with one add or get operation
	 * max: Max index to iterate up to in multiples of num.
	 * idx: Index into data100 array. Always lower than max. Iterated with num bytes at a time.
	 */
	for (int num = 1; num <= 100; num++) {
		for (int max = num; max <= 100; max += num) {
			for (int idx = 0; idx < max; idx += num) {
				/* Add num bytes of data starting at position idx in data100. */
				err = rbuf_add(&buf_100, &data100[idx], (len = num, &len), 0);
				zassert_equal(err, RBUF_SUCCESS);
				zassert_equal(num, len);
			}

			
			for (int idx = 0; idx < max; idx += num) {
				/* Get num bytes of data and. */
				err = rbuf_get(&buf_100, out, (out_len = num, &out_len),
					       RBUF_OPT_ZERO_ON_READ);
				zassert_equal(err, RBUF_SUCCESS);
				zassert_equal(num, out_len);

				/* Compare data against data at position idx in data100. */
				zassert_mem_equal(&data100[idx], out, num);
			}
		}
	}

	/* Check that buf_100 is empty. */
	err = rbuf_get(&buf_100, out, (out_len = 100, &out_len), 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);
}

ZTEST(rbuf_suite, test_rbuf_mixed_add_get_100_2)
{
	int err, idx;
	size_t len, out_len;
	uint8_t out[20] = {0};

	for (int num = 1; num <= 10; num++) {
		for (int cnt = 0; cnt < 1000000; cnt += num) {
			idx = (cnt % 100);

			/* Do not read past the data100 array. */
			if (idx + num > 100) {
				continue;
			}

			/* Add num bytes. */
			err = rbuf_add(&buf_default, &data100[idx], (len = num, &len), 0);
			zassert_equal(err, RBUF_SUCCESS);
			zassert_equal(num, len);

			/* Get num bytes back. */
			err = rbuf_get(&buf_default, out, (out_len = num, &out_len),
				       RBUF_OPT_ZERO_ON_READ);
			zassert_equal(err, RBUF_SUCCESS);
			zassert_equal(num, out_len);

			/* Compare data against data at position idx in data100. */
			zassert_mem_equal(&data100[idx], out, num);
		}
	}

	/* Check that buf_default is empty. */
	err = rbuf_get(&buf_default, out, (out_len = 20, &out_len), 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);
}

ZTEST(rbuf_suite, test_rbuf_mixed_add_get_nothing)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 0;
	err = rbuf_add(&buf_default, data100, &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 0);

	out_len = 5;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);
}

ZTEST(rbuf_suite, test_rbuf_add_arg_invalid)
{
	int err;
	size_t len;

	err = rbuf_add(&buf_default, data100, NULL, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	len = 10;
	err = rbuf_add(&buf_default, NULL, &len, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(len, 10);

	err = rbuf_add(&buf_default, NULL, NULL, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	len = 10;
	err = rbuf_add(NULL, data100, &len, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(len, 10);

	err = rbuf_add(NULL, data100, NULL, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	len = 10;
	err = rbuf_add(NULL, NULL, &len, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(len, 10);

	err = rbuf_add(NULL, NULL, NULL, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
}

ZTEST(rbuf_suite, test_rbuf_add_not_enough_space)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 20;
	err = rbuf_add(&buf_default, data100, &len, 0);
	zassert_equal(err, RBUF_ERR_NOT_ENOUGH_SPACE);
	zassert_equal(len, 20);

	out_len = 20;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);
}

ZTEST(rbuf_suite, test_rbuf_add_overwrite)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 5;
	err = rbuf_add(&buf_overwritable, data100, &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 5);

	len = 5;
	err = rbuf_add(&buf_overwritable, &data100[len], &len, RBUF_OPT_OVERWRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 5);

	out_len = 7;
	err = rbuf_get(&buf_overwritable, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 7);

	zassert_mem_equal(out, &data100[3], 7);
}

ZTEST(rbuf_suite, test_rbuf_add_overwrite_larger_than_buffer)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 20;
	err = rbuf_add(&buf_overwritable, data100, &len, RBUF_OPT_OVERWRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 20);

	out_len = 10;
	err = rbuf_get(&buf_overwritable, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 7);

	zassert_mem_equal(out, &data100[13], 7);
}

ZTEST(rbuf_suite, test_rbuf_add_overwrite_not_allowed)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 20;
	err = rbuf_add(&buf_default, data100, &len, RBUF_OPT_OVERWRITE);
	zassert_equal(err, RBUF_ERR_NOT_ALLOWED);
	zassert_equal(len, 20);

	out_len = 10;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);
}

ZTEST(rbuf_suite, test_rbuf_add_partial)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 2;
	err = rbuf_add(&buf_overwritable, data100, &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 2);

	out_len = 20;
	err = rbuf_get(&buf_overwritable, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 2);

	len = 10;
	err = rbuf_add(&buf_overwritable, &data100[20], &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 7);

	out_len = 20;
	err = rbuf_get(&buf_overwritable, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 7);

	zassert_mem_equal(&data100[20], out, 7);
}

ZTEST(rbuf_suite, test_rbuf_add_partial_overwrite)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 2;
	err = rbuf_add(&buf_overwritable, data100, &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 2);

	len = 20;
	err = rbuf_add(&buf_overwritable, &data100[2], &len,
		       RBUF_OPT_PARTIAL_WRITE | RBUF_OPT_OVERWRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 7);

	out_len = 20;
	err = rbuf_get(&buf_overwritable, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 7);

	zassert_mem_equal(&data100[2], out, 7);
}

ZTEST(rbuf_suite, test_rbuf_add_partial_not_allowed)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 15;
	err = rbuf_add(&buf_no_partial_write, data100, &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_ERR_NOT_ALLOWED);
	zassert_equal(len, 15);

	out_len = 10;
	err = rbuf_get(&buf_no_partial_write, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);
}

ZTEST(rbuf_suite, test_rbuf_add_suspended_pre)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 5;
	err = rbuf_add(&buf_suspended_add, data100, &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_ERR_SUSPENDED);
	zassert_equal(len, 5);

	err = rbuf_suspended_set(&buf_suspended_add, RBUF_OPT_ADD_RESUME);
	zassert_equal(err, RBUF_SUCCESS);

	len = 5;
	err = rbuf_add(&buf_suspended_add, data100, &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 1);

	out_len = 10;
	err = rbuf_get(&buf_suspended_add, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 1);

	zassert_mem_equal(data100, out, 1);
}

ZTEST(rbuf_suite, test_rbuf_add_suspended)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 5;
	err = rbuf_add(&buf_default, data100, &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 5);

	err = rbuf_suspended_set(&buf_default, RBUF_OPT_ADD_SUSPEND);

	len = 5;
	err = rbuf_add(&buf_default, &data100[5], &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_ERR_SUSPENDED);
	zassert_equal(len, 5);

	out_len = 10;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 5);

	zassert_mem_equal(data100, out, 1);
}

ZTEST(rbuf_suite, test_rbuf_get_arg_invalid)
{
	int err;
	size_t out_len;
	uint8_t out[20] = {0};

	err = rbuf_get(&buf_default, out, NULL, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	out_len = 10;
	err = rbuf_get(&buf_default, NULL, &out_len, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(out_len, 10);

	err = rbuf_get(&buf_default, NULL, NULL, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	out_len = 10;
	err = rbuf_get(NULL, out, &out_len, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(out_len, 10);

	err = rbuf_get(NULL, out, NULL, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	out_len = 10;
	err = rbuf_get(NULL, NULL, &out_len, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(out_len, 10);

	err = rbuf_get(NULL, NULL, NULL, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
}

ZTEST(rbuf_suite, test_rbuf_peek_arg_invalid)
{
	int err;
	size_t out_len;
	uint8_t out[20] = {0};

	err = rbuf_peek(&buf_default, out, NULL, 0, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	out_len = 10;
	err = rbuf_peek(&buf_default, NULL, &out_len, 0, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(out_len, 10);

	err = rbuf_peek(&buf_default, NULL, NULL, 0, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	out_len = 10;
	err = rbuf_peek(NULL, out, &out_len, 0, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(out_len, 10);

	err = rbuf_peek(NULL, out, NULL, 0, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	out_len = 10;
	err = rbuf_peek(NULL, NULL, &out_len, 0, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(out_len, 10);

	err = rbuf_peek(NULL, NULL, NULL, 0, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
}

ZTEST(rbuf_suite, test_rbuf_peek_not_allowed)
{
	int err;
	size_t out_len;
	uint8_t out[20] = {0};

	out_len = 20;
	err = rbuf_peek(&buf_no_peeking, out, &out_len, 0, 0);
	zassert_equal(err, RBUF_ERR_NOT_ALLOWED);
	zassert_equal(out_len, 20);
}

ZTEST(rbuf_suite, test_rbuf_peek)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	out_len = 20;
	err = rbuf_peek(&buf_default, out, &out_len, 0, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);

	len = 9;
	err = rbuf_add(&buf_default, &data100[54], &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 9);

	out_len = 8;
	err = rbuf_peek(&buf_default, out, &out_len, 0, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 8);

	zassert_mem_equal(&data100[54], out, 8);

	out_len = 6;
	err = rbuf_peek(&buf_default, out, &out_len, 0, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 6);

	zassert_mem_equal(&data100[54], out, 6);

	out_len = 20;
	err = rbuf_peek(&buf_default, out, &out_len, 0, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 9);

	zassert_mem_equal(&data100[54], out, 9);

	out_len = 20;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 9);

	zassert_mem_equal(&data100[54], out, 9);

	out_len = 20;
	err = rbuf_peek(&buf_default, out, &out_len, 0, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);
}

ZTEST(rbuf_suite, test_rbuf_mixed_peek_get_zero_on_read)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 6;
	err = rbuf_add(&buf_default, &data100[37], &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 6);

	zassert_mem_equal(&data100[37], &buf_default.buf[buf_default.ri], 6);

	out_len = 20;
	err = rbuf_peek(&buf_default, out, &out_len, 0, RBUF_OPT_ZERO_ON_READ);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 6);

	zassert_mem_equal(&data100[37], out, 6);
	zassert_mem_equal(&data100[37], &buf_default.buf[buf_default.ri], 6);

	PRINT_HEXDUMP(&buf_default.buf[buf_default.ri], 6, "before");

	uint8_t zeros[] = {0, 0, 0, 0, 0, 0};
	uint16_t ri_before_zero_read = buf_default.ri;

	out_len = 20;
	err = rbuf_get(&buf_default, out, &out_len, RBUF_OPT_ZERO_ON_READ);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 6);

	zassert_mem_equal(&data100[37], out, 6);

	zassert_mem_equal(zeros, &buf_default.buf[ri_before_zero_read], 6);
	zassert_equal(buf_default.ri, ri_before_zero_read + 6);

	PRINT_HEXDUMP(&buf_default.buf[ri_before_zero_read], 6, "after");

	out_len = 20;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);
}

ZTEST(rbuf_suite, test_rbuf_get_zero_on_read_always)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 6;
	err = rbuf_add(&buf_zero_always, &data100[37], &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 6);

	zassert_mem_equal(&data100[37], &buf_zero_always.buf[buf_zero_always.ri], 6);

	uint8_t zeros[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint16_t ri_before_zero_read = buf_zero_always.ri;

	out_len = 20;
	err = rbuf_get(&buf_zero_always, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 6);

	zassert_mem_equal(&data100[37], out, 6);

	zassert_mem_equal(zeros, &buf_zero_always.buf[ri_before_zero_read], 6);
	zassert_equal(buf_zero_always.ri, ri_before_zero_read + 6);

	len = 10;
	err = rbuf_add(&buf_zero_always, &data100[87], &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 10);

	uint16_t len1 = buf_zero_always.size - buf_zero_always.ri;
	zassert_mem_equal(&data100[87], &buf_zero_always.buf[buf_zero_always.ri], len1);
	zassert_mem_equal(&data100[87+len1], &buf_zero_always.buf[0], 10 - len1);

	PRINT_HEXDUMP(&buf_zero_always.buf[buf_zero_always.ri], len1, "before 1");
	PRINT_HEXDUMP(&buf_zero_always.buf[0], 10 - len1, "before 2");

	ri_before_zero_read = buf_zero_always.ri;

	out_len = 20;
	err = rbuf_get(&buf_zero_always, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 10);

	zassert_mem_equal(&data100[87], out, 10);

	zassert_mem_equal(zeros, &buf_zero_always.buf[ri_before_zero_read], len1);
	zassert_mem_equal(&zeros[len1], &buf_zero_always.buf[0], 10 - len1);

	PRINT_HEXDUMP(&buf_zero_always.buf[ri_before_zero_read], len1, "after 1");
	PRINT_HEXDUMP(&buf_zero_always.buf[0], 10 - len1, "after 2");

	zassert_equal(buf_zero_always.ri, 10 - len1);
}

ZTEST(rbuf_suite, test_rbuf_get_partial)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	memset(buf_default.buf, 0, buf_default.size);

	len = 6;
	err = rbuf_add(&buf_default, &data100[31], &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 6);

	out_len = 15;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 6);

	uint8_t zeros[] = {0, 0, 0, 0, 0};

	zassert_mem_equal(&data100[31], out, 6);
	zassert_mem_equal(zeros, &out[6], 5);

	zassert_mem_equal(&data100[31], &buf_default.buf[0], 6);
	zassert_mem_equal(zeros, &buf_default.buf[6], 5);
}

ZTEST(rbuf_suite, test_rbuf_get_no_partial)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	memset(buf_default.buf, 0, buf_default.size);

	len = 6;
	err = rbuf_add(&buf_default, &data100[56], &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 6);

	out_len = 15;
	err = rbuf_get(&buf_default, out, &out_len, RBUF_OPT_NO_PARTIAL_READ);
	zassert_equal(err, RBUF_ERR_NOT_ENOUGH_DATA);
	zassert_equal(out_len, 15);

	out_len = 2;
	err = rbuf_get(&buf_default, out, &out_len, RBUF_OPT_NO_PARTIAL_READ);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 2);

	out_len = 5;
	err = rbuf_get(&buf_default, out, &out_len, RBUF_OPT_NO_PARTIAL_READ);
	zassert_equal(err, RBUF_ERR_NOT_ENOUGH_DATA);
	zassert_equal(out_len, 5);

	out_len = 4;
	err = rbuf_get(&buf_default, out, &out_len, RBUF_OPT_NO_PARTIAL_READ);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 4);
}

ZTEST(rbuf_suite, test_rbuf_peek_no_partial)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	memset(buf_default.buf, 0, buf_default.size);

	len = 11;
	err = rbuf_add(&buf_default, &data100[56], &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 11);

	out_len = 15;
	err = rbuf_peek(&buf_default, out, &out_len, 0, RBUF_OPT_NO_PARTIAL_READ);
	zassert_equal(err, RBUF_ERR_NOT_ENOUGH_DATA);
	zassert_equal(out_len, 15);

	out_len = 11;
	err = rbuf_peek(&buf_default, out, &out_len, 0, RBUF_OPT_NO_PARTIAL_READ);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 11);

	zassert_mem_equal(&data100[56], out, 11);
	memset(out, 0, ARRAY_SIZE(out));

	out_len = 7;
	err = rbuf_peek(&buf_default, out, &out_len, 0, RBUF_OPT_NO_PARTIAL_READ);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 7);

	zassert_mem_equal(&data100[56], out, 7);
	memset(out, 0, ARRAY_SIZE(out));

	out_len = 11;
	err = rbuf_get(&buf_default, out, &out_len, RBUF_OPT_NO_PARTIAL_READ);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 11);

	zassert_mem_equal(&data100[56], out, 11);
	memset(out, 0, ARRAY_SIZE(out));

	out_len = 11;
	err = rbuf_get(&buf_default, out, &out_len, RBUF_OPT_NO_PARTIAL_READ);
	zassert_equal(err, RBUF_ERR_NOT_ENOUGH_DATA);
	zassert_equal(out_len, 11);

	out_len = 11;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);
}

ZTEST(rbuf_suite, test_rbuf_get_suspended_pre)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 4;
	err = rbuf_add(&buf_suspended_get, &data100[61], &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 2);

	out_len = 1;
	err = rbuf_get(&buf_suspended_get, out, &out_len, 0);
	zassert_equal(err, RBUF_ERR_SUSPENDED);
	zassert_equal(out_len, 1);

	err = rbuf_suspended_set(&buf_suspended_get, RBUF_OPT_GET_RESUME);
	zassert_equal(err, RBUF_SUCCESS);

	out_len = 1;
	err = rbuf_get(&buf_suspended_get, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 1);

	zassert_mem_equal(&data100[61], out, 1);
}

ZTEST(rbuf_suite, test_rbuf_get_suspended)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 5;
	err = rbuf_add(&buf_default, data100, &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 5);

	out_len = 10;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 5);

	zassert_mem_equal(data100, out, 5);
	zassert_equal(out[5], 0);

	memset(out, 0, ARRAY_SIZE(out));

	err = rbuf_suspended_set(&buf_default, RBUF_OPT_GET_SUSPEND);
	zassert_equal(err, RBUF_SUCCESS);

	out_len = 10;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_ERR_SUSPENDED);
	zassert_equal(out_len, 10);

	len = 5;
	err = rbuf_add(&buf_default, &data100[91], &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 5);

	out_len = 10;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_ERR_SUSPENDED);
	zassert_equal(out_len, 10);

	err = rbuf_suspended_set(&buf_default, RBUF_OPT_GET_RESUME);
	zassert_equal(err, RBUF_SUCCESS);

	out_len = 10;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 5);

	zassert_mem_equal(&data100[91], out, 5);
	zassert_equal(out[5], 0);
}

ZTEST(rbuf_suite, test_rbuf_reset_arg_invalid)
{
	int err;

	err = rbuf_reset(NULL, false);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	err = rbuf_reset(NULL, true);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
}

ZTEST(rbuf_suite, test_rbuf_reset_no_zeroing)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};

	len = 5;
	err = rbuf_add(&buf_default, data100, &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 5);

	err = rbuf_reset(&buf_default, false);
	zassert_equal(err, RBUF_SUCCESS);

	out_len = 10;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);

	zassert_mem_equal(data100, buf_default.buf, 5);
	zassert_equal(buf_default.buf[5], 0);
}

ZTEST(rbuf_suite, test_rbuf_reset_zeroing)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};
	uint8_t zeros[] = {0, 0, 0, 0, 0};

	len = 5;
	err = rbuf_add(&buf_default, data100, &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 5);

	zassert_mem_equal(data100, buf_default.buf, 5);

	err = rbuf_reset(&buf_default, true);
	zassert_equal(err, RBUF_SUCCESS);

	out_len = 10;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);

	zassert_mem_equal(zeros, buf_default.buf, 5);
	zassert_equal(buf_default.buf[5], 0);
}

ZTEST(rbuf_suite, test_rbuf_sizes_get)
{
	int err;
	size_t len, out_len;
	uint8_t out[20] = {0};
	struct rbuf_sizes stats;

	len = 20;
	err = rbuf_add(&buf_default, data100, &len, RBUF_OPT_PARTIAL_WRITE);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 13);

	zassert_mem_equal(data100, buf_default.buf, 13);

	err = rbuf_sizes_get(&buf_default, &stats);
	zassert_equal(err, RBUF_SUCCESS);

	zassert_equal(stats.max, 13);
	zassert_equal(stats.used, 13);
	zassert_equal(stats.free, 0);

	out_len = 7;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 7);

	err = rbuf_sizes_get(&buf_default, &stats);
	zassert_equal(err, RBUF_SUCCESS);

	zassert_equal(stats.max, 13);
	zassert_equal(stats.used, 6);
	zassert_equal(stats.free, 7);

	out_len = 5;
	err = rbuf_peek(&buf_default, out, &out_len, 0, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 5);

	zassert_mem_equal(&data100[7], out, 5);

	err = rbuf_sizes_get(&buf_default, &stats);
	zassert_equal(err, RBUF_SUCCESS);

	zassert_equal(stats.max, 13);
	zassert_equal(stats.used, 6);
	zassert_equal(stats.free, 7);

	memset(out, 0, ARRAY_SIZE(out));

	out_len = 5;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 5);

	zassert_mem_equal(&data100[7], out, 5);

	err = rbuf_sizes_get(&buf_default, &stats);
	zassert_equal(err, RBUF_SUCCESS);

	zassert_equal(stats.max, 13);
	zassert_equal(stats.used, 1);
	zassert_equal(stats.free, 12);

	err = rbuf_reset(&buf_default, false);
	zassert_equal(err, RBUF_SUCCESS);

	err = rbuf_sizes_get(&buf_default, &stats);
	zassert_equal(err, RBUF_SUCCESS);

	zassert_equal(stats.max, 13);
	zassert_equal(stats.used, 0);
	zassert_equal(stats.free, 13);
}

ZTEST(rbuf_suite, test_rbuf_sizes_get_arg_invalid)
{
	int err;
	struct rbuf_sizes stats;

	err = rbuf_sizes_get(NULL, &stats);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	err = rbuf_sizes_get(&buf_default, NULL);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	err = rbuf_sizes_get(NULL, NULL);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
}

ZTEST(rbuf_suite, test_rbuf_suspended_set_arg_invalid)
{
	int err;

	err = rbuf_suspended_set(NULL, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
}

ZTEST(rbuf_suite, test_rbuf_suspended_get_and_set)
{
	int err;
	uint16_t flags;

	err = rbuf_suspended_get(&buf_default, &flags);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(flags, 0);

	err = rbuf_suspended_set(&buf_default, RBUF_OPT_ADD_SUSPEND);
	zassert_equal(err, RBUF_SUCCESS);

	err = rbuf_suspended_get(&buf_default, &flags);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(flags & RBUF_FLAG_ADD_SUSPEND, RBUF_FLAG_ADD_SUSPEND);

	err = rbuf_suspended_set(&buf_default, RBUF_OPT_ADD_RESUME | RBUF_OPT_GET_SUSPEND);
	zassert_equal(err, RBUF_SUCCESS);

	err = rbuf_suspended_get(&buf_default, &flags);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(flags & RBUF_FLAG_GET_SUSPEND, RBUF_FLAG_GET_SUSPEND);

	err = rbuf_suspended_set(&buf_default, RBUF_OPT_ADD_SUSPEND);
	zassert_equal(err, RBUF_SUCCESS);

	err = rbuf_suspended_get(&buf_default, &flags);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(flags & (RBUF_FLAG_ADD_SUSPEND | RBUF_FLAG_GET_SUSPEND),
		      RBUF_FLAG_ADD_SUSPEND | RBUF_FLAG_GET_SUSPEND);

	err = rbuf_suspended_set(&buf_default, RBUF_OPT_ADD_RESUME | RBUF_OPT_GET_RESUME);
	zassert_equal(err, RBUF_SUCCESS);

	err = rbuf_suspended_get(&buf_default, &flags);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(flags, 0);
}

ZTEST(rbuf_suite, test_rbuf_suspended_get_pre)
{
	int err;
	uint16_t flags;

	err = rbuf_suspended_get(&buf_suspended_add, &flags);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(flags & RBUF_FLAG_ADD_SUSPEND, RBUF_FLAG_ADD_SUSPEND);

	err = rbuf_suspended_set(&buf_suspended_add, RBUF_OPT_GET_SUSPEND);
	zassert_equal(err, RBUF_SUCCESS);

	err = rbuf_suspended_get(&buf_suspended_add, &flags);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(flags & (RBUF_FLAG_ADD_SUSPEND | RBUF_FLAG_GET_SUSPEND),
		      RBUF_FLAG_ADD_SUSPEND | RBUF_FLAG_GET_SUSPEND);

	err = rbuf_suspended_get(&buf_suspended_get, &flags);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(flags & RBUF_FLAG_GET_SUSPEND, RBUF_FLAG_GET_SUSPEND);

	err = rbuf_suspended_set(&buf_suspended_get, RBUF_OPT_ADD_SUSPEND);
	zassert_equal(err, RBUF_SUCCESS);

	err = rbuf_suspended_get(&buf_suspended_get, &flags);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(flags & (RBUF_FLAG_ADD_SUSPEND | RBUF_FLAG_GET_SUSPEND),
		      RBUF_FLAG_ADD_SUSPEND | RBUF_FLAG_GET_SUSPEND);
}

ZTEST(rbuf_suite, test_rbuf_suspended_get_arg_invalid)
{
	int err;
	uint16_t flags;

	err = rbuf_suspended_get(NULL, &flags);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	err = rbuf_suspended_get(&buf_default, NULL);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	err = rbuf_suspended_get(NULL, NULL);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
}

ZTEST(rbuf_suite, test_rbuf_add_arg_invalid_magic)
{
#if defined(CONFIG_RBUF_MAGIC)
	int err;
	size_t len;
	uint32_t *const magic_ptr = (uint32_t *const)(&buf_default.magic);

	*magic_ptr = ~RBUF_MAGIC;

	len = 10;
	err = rbuf_add(&buf_default, data100, &len, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(len, 10);

	*magic_ptr = RBUF_MAGIC;

	len = 10;
	err = rbuf_add(&buf_default, data100, &len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(len, 10);
#else
	ztest_test_skip();
#endif
}

ZTEST(rbuf_suite, test_rbuf_get_arg_invalid_magic)
{
#if defined(CONFIG_RBUF_MAGIC)
	int err;
	size_t out_len;
	uint8_t out[20] = {0};
	uint32_t *const magic_ptr = (uint32_t *const)(&buf_default.magic);

	*magic_ptr = ~RBUF_MAGIC;

	out_len = 10;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(out_len, 10);

	*magic_ptr = RBUF_MAGIC;

	out_len = 10;
	err = rbuf_get(&buf_default, out, &out_len, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);
#else
	ztest_test_skip();
#endif
}

ZTEST(rbuf_suite, test_rbuf_peek_arg_invalid_magic)
{
#if defined(CONFIG_RBUF_MAGIC)
	int err;
	size_t out_len;
	uint8_t out[20] = {0};
	uint32_t *const magic_ptr = (uint32_t *const)(&buf_default.magic);

	*magic_ptr = ~RBUF_MAGIC;

	out_len = 10;
	err = rbuf_peek(&buf_default, out, &out_len, 0, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);
	zassert_equal(out_len, 10);

	*magic_ptr = RBUF_MAGIC;

	out_len = 10;
	err = rbuf_peek(&buf_default, out, &out_len, 0, 0);
	zassert_equal(err, RBUF_SUCCESS);
	zassert_equal(out_len, 0);
#else
	ztest_test_skip();
#endif
}

ZTEST(rbuf_suite, test_rbuf_reset_arg_invalid_magic)
{
#if defined(CONFIG_RBUF_MAGIC)
	int err;
	uint32_t *const magic_ptr = (uint32_t *const)(&buf_default.magic);

	*magic_ptr = ~RBUF_MAGIC;

	err = rbuf_reset(&buf_default, false);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	*magic_ptr = RBUF_MAGIC;

	err = rbuf_reset(&buf_default, false);
	zassert_equal(err, RBUF_SUCCESS);
#else
	ztest_test_skip();
#endif
}

ZTEST(rbuf_suite, test_rbuf_sizes_get_arg_invalid_magic)
{
#if defined(CONFIG_RBUF_MAGIC)
	int err;
	struct rbuf_sizes stats;
	uint32_t *const magic_ptr = (uint32_t *const)(&buf_default.magic);

	*magic_ptr = ~RBUF_MAGIC;

	err = rbuf_sizes_get(&buf_default, &stats);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	*magic_ptr = RBUF_MAGIC;

	err = rbuf_sizes_get(&buf_default, &stats);
	zassert_equal(err, RBUF_SUCCESS);
#else
	ztest_test_skip();
#endif
}

ZTEST(rbuf_suite, test_rbuf_suspended_set_arg_invalid_magic)
{
#if defined(CONFIG_RBUF_MAGIC)
	int err;
	uint32_t *const magic_ptr = (uint32_t *const)(&buf_default.magic);

	*magic_ptr = ~RBUF_MAGIC;

	err = rbuf_suspended_set(&buf_default, 0);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	*magic_ptr = RBUF_MAGIC;

	err = rbuf_suspended_set(&buf_default, 0);
	zassert_equal(err, RBUF_SUCCESS);
#else
	ztest_test_skip();
#endif
}

ZTEST(rbuf_suite, test_rbuf_suspended_get_arg_invalid_magic)
{
#if defined(CONFIG_RBUF_MAGIC)
	int err;
	uint16_t flags;
	uint32_t *const magic_ptr = (uint32_t *const)(&buf_default.magic);

	*magic_ptr = ~RBUF_MAGIC;

	err = rbuf_suspended_get(&buf_default, &flags);
	zassert_equal(err, RBUF_ERR_ARG_INVALID);

	*magic_ptr = RBUF_MAGIC;

	err = rbuf_suspended_get(&buf_default, &flags);
	zassert_equal(err, RBUF_SUCCESS);
#else
	ztest_test_skip();
#endif
}
