#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <zephyr/sys/util.h>

/** @brief Flags for altering behavior of ring buffer. */
#define RBUF_FLAG_ALLOW_OVERWRITE  BIT(0)
#define RBUF_FLAG_NO_PARTIAL_WRITE BIT(1)
#define RBUF_FLAG_NO_PEEKING       BIT(2)
#define RBUF_FLAG_ZERO_MEM_ALWAYS  BIT(3)
#define RBUF_FLAG_ADD_SUSPEND      BIT(4)
#define RBUF_FLAG_GET_SUSPEND      BIT(5)

/** @brief Options for altering behavior of ring buffer operations. */
#define RBUF_OPT_OVERWRITE         BIT(8)
#define RBUF_OPT_PARTIAL_WRITE     BIT(9)
#define RBUF_OPT_NO_PARTIAL_READ   BIT(10)
#define RBUF_OPT_ZERO_ON_READ      BIT(11)
#define RBUF_OPT_ADD_SUSPEND       BIT(12)
#define RBUF_OPT_ADD_RESUME        BIT(13)
#define RBUF_OPT_GET_SUSPEND       BIT(14)
#define RBUF_OPT_GET_RESUME        BIT(15)

/** @brief Values returned by ring buffer operations. */
#define RBUF_SUCCESS              0L
#define RBUF_ERR_ARG_INVALID      1L
#define RBUF_ERR_NOT_ENOUGH_SPACE 2L
#define RBUF_ERR_NOT_ENOUGH_DATA  3L
#define RBUF_ERR_NOT_ALLOWED      4L
#define RBUF_ERR_SUSPENDED        5L

/** @brief rbuf library struct magic. */
#define RBUF_MAGIC 0xC18CB0F8U

/** @brief Ring buffer structure. */
struct rbuf {
	IF_ENABLED(CONFIG_RBUF_MAGIC, (uint32_t const magic;))
	uint16_t wi;
	uint16_t ri;
	uint16_t flags;
	uint16_t const size;
	uint8_t *const buf;
};

/** @brief Sizes of a ring buffer returned by @ref rbuf_sizes_get */
struct rbuf_sizes {
	uint16_t max;
	uint16_t free;
	uint16_t used;
};

/** @brief Macro for defining a ring buffer, rbuf structure. */
#define RBUF_DEF(_name, _size, _flags)                                                             \
	static uint8_t CONCAT(_name, _buf)[_size];                                                 \
	struct rbuf (_name) = { IF_ENABLED(CONFIG_RBUF_MAGIC, (.magic = RBUF_MAGIC,))              \
				.wi = 0, .ri = 0, .flags = (_flags), .size = (_size),              \
				.buf = &(CONCAT(_name, _buf)[0]) }

/**
 * @brief Add data to ring buffer.
 *
 * @param         buf    Ring buffer.
 * @param[in]     data   Array of bytes to copy into the ring buffer.
 * @param[in,out] len    Variable holding number of bytes to copy from array into ring buffer.
 *                       On return, holds number of bytes that was copied into ring buffer.
 * @param[in]     opts   Options to alter function behaviour. Use @c 0 for no options.
 *
 * @retval RBUF_SUCCESS          On success.
 * @retval RBUF_ERR_ARG_INVALID  If @p buf @p data or @p len is @c NULL, or struct magic is wrong.
 * @retval RBUF_ERR_SUSPENDED    If adding data to ring buffer has been suspended with configuration
 *                               @c RBUF_FLAG_ADD_SUSPEND or function @ref rbuf_suspended_set.
 * @retval RBUF_ERR_NOT_ALLOWED  If using option @c RBUF_OPT_OVERWRITE and ring buffer was not
 *                               configured with @c RBUF_FLAG_ALLOW_OVERWRITE, or
 *                               using option @c RBUF_OPT_PARTIAL_WRITE and ring buffer was
 *                               configured with @c RBUF_FLAG_NO_PARTIAL_WRITE.
 * @retval RBUF_ERR_NOT_ENOUGH_SPACE  If There is not enough space left in the buffer and options
 *                                    @c RBUF_OPT_OVERWRITE or @c RBUF_OPT_PARTIAL_WRITE is not set.
 */
int rbuf_add(struct rbuf *buf, const uint8_t *data, size_t *len, uint16_t opts);

/**
 * @brief Get data from ring buffer.
 *
 * @param         buf    Ring buffer.
 * @param[out]    data   Array to copy bytes into from ring buffer.
 * @param[in,out] len    Variable holding the length of array to copy bytes into.
 *                       On return, holds number of bytes actually copied into the array.
 * @param[in]     opts   Options to alter function behaviour. Use @c 0 for no options.
 *
 * @retval RBUF_SUCCESS          On success.
 * @retval RBUF_ERR_ARG_INVALID  If @p buf @p data or @p len is @c NULL, or struct magic is wrong.
 * @retval RBUF_ERR_SUSPENDED    If getting data from buffer has been suspended with configuration
 *                               @c RBUF_FLAG_GET_SUSPEND or function @ref rbuf_suspended_set.
 * @retval RBUF_ERR_NOT_ENOUGH_DATA  If using option RBUF_OPT_NO_PARTIAL_READ and there is not
 *                                   enough data in the ring buffer to fill the output array.
 */
int rbuf_get(struct rbuf *buf, uint8_t *data, size_t *len, uint16_t opts);

/**
 * @brief Get data from ring buffer without removing it.
 *
 * @param         buf      Ring buffer.
 * @param[out]    data     Array to copy bytes into from ring buffer.
 * @param[in,out] len      Variable holding the length of array to copy bytes into.
 *                         On return, holds number of bytes actually copied into the array.
 * @param[in]     offset   Offset to start reading data from. Use @c 0 for no offset.
 * @param[in]     opts     Options to alter function behaviour. Use @c 0 for no options.
 *
 * @retval RBUF_SUCCESS          On success.
 * @retval RBUF_ERR_ARG_INVALID  If @p buf @p data or @p len is @c NULL, or struct magic is wrong.
 * @retval RBUF_ERR_SUSPENDED    If getting data from buffer has been suspended with configuration
 *                               @c RBUF_FLAG_GET_SUSPEND or function @ref rbuf_suspended_set.
 * @retval RBUF_ERR_NOT_ALLOWED  If ring buffer was configured with @c RBUF_FLAG_NO_PEEKING.
 * @retval RBUF_ERR_NOT_ENOUGH_DATA  If using option RBUF_OPT_NO_PARTIAL_READ and there is not
 *                                   enough data in the ring buffer to fill the output array.
 */
int rbuf_peek(struct rbuf *buf, uint8_t *data, size_t *len, size_t offset, uint16_t opts);

/**
 * @brief Reset ring buffer to its default state. Dropping all added data.
 *
 * @param buf           Ring buffer.
 * @param zero_memory   If ring buffer content should be zeroed during reset.
 *
 * @retval RBUF_SUCCESS          On success.
 * @retval RBUF_ERR_ARG_INVALID  If @p buf is @c NULL, or struct magic is wrong.
 */
int rbuf_reset(struct rbuf *buf, bool zero_memory);

/**
 * @brief Suspend add or get functionality.
 *
 * @param     buf    Ring buffer.
 * @param[in] opts   Value for choosing what functionality to suspend and/or resume.
 *                   Use @c RBUF_OPT_SUSPEND_ADD, @c RBUF_OPT_SUSPEND_GET,
 *                   @c RBUF_OPT_RESUME_ADD and @c RBUF_OPT_RESUME_GET with bitwise-OR.
 *
 * @retval RBUF_SUCCESS          On success.
 * @retval RBUF_ERR_ARG_INVALID  If @p buf is @c NULL, or struct magic is wrong.
 */
int rbuf_suspended_set(struct rbuf *buf, uint16_t opts);

/**
 * @brief Return current state of add/get suspension as a set of flags.
 *
 * @param      buf     Ring buffer.
 * @param[out] flags   Holds suspension state of add/get operations on return.
 *
 * @retval RBUF_SUCCESS          On success.
 * @retval RBUF_ERR_ARG_INVALID  If @p buf is @c NULL, or struct magic is wrong.
 */
int rbuf_suspended_get(struct rbuf *buf, uint16_t *flags);

/**
 * @brief Return total, used and free space of the ring buffer in bytes.
 *
 * @param      buf     Ring buffer.
 * @param[out] sizes   Holds max, used and free buffer space on return.
 *
 * @retval RBUF_SUCCESS          On success.
 * @retval RBUF_ERR_ARG_INVALID  If @p buf is @c NULL, or struct magic is wrong.
 */
int rbuf_sizes_get(struct rbuf *buf, struct rbuf_sizes *sizes);

/**
 * @brief Return string representation of a given rbuf error.
 *
 * @param[in] err   The rbuf error.
 *
 * @return  String representation of the given rbuf error.
 */
const char *rbuf_strerr(int err);
