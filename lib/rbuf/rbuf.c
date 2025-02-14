#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/sys/util.h>

#include <rbuf.h>

#define BUFFER_FULL_BIT   (13)
#define ADD_SUSPENDED_BIT (14)
#define GET_SUSPENDED_BIT (15)

#if !defined(LOG2)
#error LOG2 not defined
#endif

#ifndef IS_BIT_SET
#define IS_BIT_SET(value, bit) ((((value) >> (bit)) & (0x1)) != 0)
#endif

#define MAGIC_CHECK(_buf) COND_CODE_1(CONFIG_RBUF_MAGIC, ((_buf)->magic != RBUF_MAGIC), (0))

int rbuf_add(struct rbuf *buf, const uint8_t *data, size_t *len, uint16_t opts)
{
	uint16_t free1, free2;
	const bool overwrite = IS_BIT_SET(opts, LOG2(RBUF_OPT_OVERWRITE));
	const bool partial = IS_BIT_SET(opts, LOG2(RBUF_OPT_PARTIAL_WRITE));

	if (buf == NULL || data == NULL || len == NULL || MAGIC_CHECK(buf)) {
		return RBUF_ERR_ARG_INVALID;
	}

	if (IS_BIT_SET(buf->flags, ADD_SUSPENDED_BIT)) {
		return RBUF_ERR_SUSPENDED;
	}

	if ((partial && IS_BIT_SET(buf->flags, LOG2(RBUF_FLAG_NO_PARTIAL_WRITE))) ||
	    (overwrite && !IS_BIT_SET(buf->flags, LOG2(RBUF_FLAG_ALLOW_OVERWRITE)))) {
		return RBUF_ERR_NOT_ALLOWED;
	}

	uint16_t data_len = MIN(*len, UINT16_MAX);

	/* Find sizes of the two (possibly) empty segments of the ring buffer. */
	if (buf->wi < buf->ri || IS_BIT_SET(buf->flags, BUFFER_FULL_BIT)) {
		free1 = buf->ri - buf->wi;
		free2 = 0;
	} else {
		free1 = buf->size - buf->wi;
		free2 = buf->ri;
	}

	const uint16_t free = free1 + free2;

	if (!partial && !overwrite && data_len > free) {
		return RBUF_ERR_NOT_ENOUGH_SPACE;
	}

	/* Find number of bytes to overwrite previous buffer content if overwriting is allowed. */
	const uint16_t overwrite_len = (overwrite && data_len > free) ?
		(MIN(data_len, buf->size) - free) : 0;

	if (!partial && overwrite) {
		/* Copying the first set of bytes is unnecessary.They will be overwritten anyway. */
		data += MAX(data_len, buf->size) - buf->size;
	}

	if (partial) {
		data_len = MIN(data_len, free);
	}

	if (data_len >= free) {
		/* Buffer will be full after copying data, so we can set it now. */
		WRITE_BIT(buf->flags, BUFFER_FULL_BIT, 1);
	}

	const uint16_t copy1_len = MIN(data_len, free1);
	const uint16_t copy2_len = MIN(data_len - copy1_len, free2);

	memcpy(&buf->buf[buf->wi], &data[0], copy1_len);
	memcpy(&buf->buf[0], &data[copy1_len], copy2_len);

	/* Update write index. */
	if (copy2_len > 0) {
		buf->wi = copy2_len;
	} else if (buf->wi < buf->size - copy1_len) {
		buf->wi += copy1_len;
	} else if (buf->wi == buf->size - copy1_len) {
		buf->wi = 0;
	} else {
		/* Should not happen. Should assert. */
	}

	*len = copy1_len + copy2_len;

	if (overwrite_len == 0) {
		if (!partial && overwrite) {
			*len += MAX(data_len - buf->size, 0);
		}
		return RBUF_SUCCESS;
	}

	const uint16_t overwrite1_len = MIN(buf->size - buf->wi, overwrite_len);
	const uint16_t overwrite2_len = overwrite_len - overwrite1_len;

	memcpy(&buf->buf[buf->wi], &data[*len], overwrite1_len);
	memcpy(&buf->buf[0], &data[*len + overwrite1_len], overwrite2_len);

	/* Update write index. */
	if (overwrite2_len > 0) {
		buf->wi = overwrite2_len;
	} else if (buf->wi < buf->size - overwrite1_len) {
		buf->wi += overwrite1_len;
	} else if (buf->wi == buf->size - overwrite1_len) {
		buf->wi = 0;
	} else {
		/* Should not happen. Should assert. */
	}

	/* Update read index to the same as write index. Data have been overwritten. */
	buf->ri = buf->wi;

	*len += overwrite_len;

	/* Add length of the data that was jumped over when offsetting data. */
	if (!partial && overwrite) {
		*len += MAX(data_len - buf->size, 0);
	}

	return RBUF_SUCCESS;
}

int rbuf_get(struct rbuf *buf, uint8_t *data, size_t *len, uint16_t opts)
{
	if (buf == NULL || data == NULL || len == NULL || MAGIC_CHECK(buf)) {
		return RBUF_ERR_ARG_INVALID;
	}

	uint16_t used1, used2;
	const uint16_t data_len = MIN(*len, UINT16_MAX);
	const bool must_fill_output = IS_BIT_SET(opts, LOG2(RBUF_OPT_NO_PARTIAL_READ));

	if (IS_BIT_SET(buf->flags, GET_SUSPENDED_BIT)) {
		return RBUF_ERR_SUSPENDED;
	}

	/* Find sizes of the two filled segments of the ring buffer. */
	if ((buf->wi < buf->ri) || IS_BIT_SET(buf->flags, BUFFER_FULL_BIT)) {
		used1 = buf->size - buf->ri;
		used2 = buf->wi;
	} else {
		used1 = buf->wi - buf->ri;
		used2 = 0;
	}

	/* Return error if partially filling the output buffer is not allowed. */
	if (must_fill_output && (data_len > used1 + used2)) {
		return RBUF_ERR_NOT_ENOUGH_DATA;
	}

	/* Find how much to copy from the middle, or the end and start of the buffer. */
	const uint16_t copy1_len = MIN(data_len, used1);
	const uint16_t copy2_len = MIN(data_len - copy1_len, used2);

	/* Copy data out of the ring buffer. */
	memcpy(&data[0], &buf->buf[buf->ri], copy1_len);
	memcpy(&data[copy1_len], &buf->buf[0], copy2_len);

	if ((buf->flags & RBUF_FLAG_ZERO_MEM_ALWAYS) || (opts & RBUF_OPT_ZERO_ON_READ)) {
		/* Zero memory where the data was stored in the ring buffer. */
		memset(&buf->buf[buf->ri], 0x00, copy1_len);
		memset(&buf->buf[0], 0x00, copy2_len);
	}

	/* Update read index. */
	if (copy2_len > 0) {
		buf->ri = copy2_len;
	} else if (buf->ri + copy1_len < buf->size) {
		buf->ri += copy1_len;
	} else if (buf->ri + copy1_len == buf->size) {
		buf->ri = 0;
	} else {
		/* Should not happen. */
		__ASSERT_NO_MSG(false);
	}

	/* Set output length. */
	*len = (size_t)(copy1_len + copy2_len);

	/* Unset buffer full flag if anything was copied out. */
	if (*len > 0) {
		WRITE_BIT(buf->flags, BUFFER_FULL_BIT, 0);
	}

	return RBUF_SUCCESS;
}

int rbuf_peek(struct rbuf *buf, uint8_t *data, size_t *len, size_t offset, uint16_t opts)
{
	if (buf == NULL || data == NULL || len == NULL || MAGIC_CHECK(buf)) {
		return RBUF_ERR_ARG_INVALID;
	}

	uint16_t used1, used2, temp;
	const uint16_t data_len = MIN(*len, UINT16_MAX);
	const bool must_fill_output = IS_BIT_SET(opts, LOG2(RBUF_OPT_NO_PARTIAL_READ));

	if (IS_BIT_SET(buf->flags, GET_SUSPENDED_BIT)) {
		return RBUF_ERR_SUSPENDED;
	}

	if (IS_BIT_SET(buf->flags, LOG2(RBUF_FLAG_NO_PEEKING))) {
		return RBUF_ERR_NOT_ALLOWED;
	}

	/* Find sizes of the two filled segments of the ring buffer. */
	if ((buf->wi < buf->ri) || IS_BIT_SET(buf->flags, BUFFER_FULL_BIT)) {
		used1 = buf->size - buf->ri;
		used2 = buf->wi;
	} else {
		used1 = buf->wi - buf->ri;
		used2 = 0;
	}

	/* Clamp offset to uint16_t. */
	offset = MIN(offset, UINT16_MAX);

	/* Adjust size of segments based on offset. */
	temp = MAX(used1, offset) - offset;
	used2 = MAX(used1 + used2 - temp, offset) - offset;
	used1 = temp;

	/* Return error if partially filling the output buffer is not allowed. */
	if (must_fill_output && (data_len > used1 + used2)) {
		return RBUF_ERR_NOT_ENOUGH_DATA;
	}

	/* Find how much to copy from the middle, or the end and start of the buffer. */
	const uint16_t copy1_len = MIN(data_len, used1);
	const uint16_t copy2_len = MIN(data_len - copy1_len, used2);

	/* Find offsets to use when copying. Offsets outside of the buffer is not an issue
	 * because the lengths will be zero in those cases.
	 */
	const uint16_t copy1_off = buf->ri + offset;
	const uint16_t copy2_off = MAX(copy1_off, buf->size) - buf->size;

	/* Copy data out of the ring buffer. */
	memcpy(&data[0], &buf->buf[copy1_off], copy1_len);
	memcpy(&data[copy1_len], &buf->buf[copy2_off], copy2_len);

	/* Set output length. */
	*len = (size_t)(copy1_len + copy2_len);

	return RBUF_SUCCESS;
}

int rbuf_reset(struct rbuf *buf, bool zero_memory)
{
	if (buf == NULL || MAGIC_CHECK(buf)) {
		return RBUF_ERR_ARG_INVALID;
	}

	uint16_t flags = buf->flags;

	WRITE_BIT(flags, BUFFER_FULL_BIT, 0);
	WRITE_BIT(flags, ADD_SUSPENDED_BIT, IS_BIT_SET(flags, LOG2(RBUF_FLAG_ADD_SUSPEND)));
	WRITE_BIT(flags, GET_SUSPENDED_BIT, IS_BIT_SET(flags, LOG2(RBUF_FLAG_GET_SUSPEND)));

	if (zero_memory || IS_BIT_SET(flags, LOG2(RBUF_FLAG_ZERO_MEM_ALWAYS))) {
		memset(&buf->buf[0], 0x00, buf->size);
	}

	buf->wi = 0;
	buf->ri = 0;
	buf->flags = flags;

	return RBUF_SUCCESS;
}

int rbuf_suspended_set(struct rbuf *buf, uint16_t opts)
{
	if (buf == NULL || MAGIC_CHECK(buf)) {
		return RBUF_ERR_ARG_INVALID;
	}

	const bool suspend_add = IS_BIT_SET(opts, LOG2(RBUF_OPT_ADD_SUSPEND));
	const bool suspend_get = IS_BIT_SET(opts, LOG2(RBUF_OPT_GET_SUSPEND));
	uint16_t flags = buf->flags;

	if (suspend_add ^ IS_BIT_SET(opts, LOG2(RBUF_OPT_ADD_RESUME))) {
		WRITE_BIT(flags, ADD_SUSPENDED_BIT, suspend_add);
	}
	if (suspend_get ^ IS_BIT_SET(opts, LOG2(RBUF_OPT_GET_RESUME))) {
		WRITE_BIT(flags, GET_SUSPENDED_BIT, suspend_get);
	}

	buf->flags = flags;

	return RBUF_SUCCESS;
}

int rbuf_suspended_get(struct rbuf *buf, uint16_t *flags_out)
{
	if (buf == NULL || flags_out == NULL || MAGIC_CHECK(buf)) {
		return RBUF_ERR_ARG_INVALID;
	}

	uint16_t flags = 0;

	WRITE_BIT(flags, LOG2(RBUF_FLAG_ADD_SUSPEND), IS_BIT_SET(buf->flags, ADD_SUSPENDED_BIT));
	WRITE_BIT(flags, LOG2(RBUF_FLAG_GET_SUSPEND), IS_BIT_SET(buf->flags, GET_SUSPENDED_BIT));

	*flags_out = flags;

	return RBUF_SUCCESS;
}

int rbuf_sizes_get(struct rbuf *buf, struct rbuf_sizes *sizes)
{
	if (buf == NULL || sizes == NULL || MAGIC_CHECK(buf)) {
		return RBUF_ERR_ARG_INVALID;
	}

	if (buf->wi < buf->ri || IS_BIT_SET(buf->flags, BUFFER_FULL_BIT)) {
		sizes->used = (buf->size - buf->ri + buf->wi);
	} else {
		sizes->used = (buf->wi - buf->ri);
	}

	sizes->max = buf->size;
	sizes->free = buf->size - sizes->used;

	return RBUF_SUCCESS;
}

const char *rbuf_strerr(int err)
{
	switch (err) {
#if CONFIG_RBUF_ERROR_STRING
	case RBUF_SUCCESS:              return "SUCCESS";
	case RBUF_ERR_ARG_INVALID:      return "ARG_INVALID";
	case RBUF_ERR_NOT_ENOUGH_SPACE: return "NOT_ENOUGH_SPACE";
	case RBUF_ERR_NOT_ENOUGH_DATA:  return "NOT_ENOUGH_DATA";
	case RBUF_ERR_NOT_ALLOWED:      return "NOT_ALLOWED";
	case RBUF_ERR_SUSPENDED:        return "SUSPENDED";
	default:                        return "UNKNOWN";
#else
	default:                        return "";
#endif
	}
}