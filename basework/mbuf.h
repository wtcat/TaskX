/*
 * Copyright 2023 wtcat
 */
#ifndef BASEWORK_MBUF_H_
#define BASEWORK_MBUF_H_

#include <stdint.h>

#include "basework/compiler.h"
#include "basework/container/queue.h"

#ifdef __cplusplus
extern "C"{
#endif

STAILQ_HEAD(rte_mbuf_list, rte_mbuf);

/* Alignment needed for various parts of the buffer definition */
#define __rte_mbuf_align __rte_aligned(sizeof(void *))

/**
 * @brief Simple network buffer representation.
 *
 * This is a simpler variant of the rte_mbuf object (in fact rte_mbuf uses
 * rte_mbuf_simple internally). It doesn't provide any kind of reference
 * counting, user data, dynamic allocation, or in general the ability to
 * pass through kernel objects such as FIFOs.
 *
 * The main use of this is for scenarios where the meta-data of the normal
 * rte_mbuf isn't needed and causes too much overhead. This could be e.g.
 * when the buffer only needs to be allocated on the stack or when the
 * access to and lifetime of the buffer is well controlled and constrained.
 */
#define RTE_MBUF_SIMPLE_MEMBERS \
	uint8_t *data; \
	uint16_t len; \
	uint16_t size; \
	uint8_t *__buf;

struct rte_mbuf_simple {
	RTE_MBUF_SIMPLE_MEMBERS
};

/**
 * @brief Initialize a rte_mbuf_simple object.
 *
 * This needs to be called after creating a rte_mbuf_simple object using
 * the NET_BUF_SIMPLE macro.
 *
 * @param buf Buffer to initialize.
 * @param reserve_head Headroom to reserve.
 */
static inline void __rte_mbuf_simple_reset(struct rte_mbuf_simple *buf,
				       size_t reserve_head)
{
	buf->data = buf->__buf + reserve_head;
	buf->len = 0U;
}

/**
 * @brief Initialize a rte_mbuf_simple object with data.
 *
 * Initialized buffer object with external data.
 *
 * @param buf Buffer to initialize.
 * @param data External data pointer
 * @param size Amount of data the pointed data buffer if able to fit.
 */
void rte_mbuf_simple_attach(struct rte_mbuf_simple *buf,
				   void *data, size_t size);

/**
 * @brief Reset buffer
 *
 * Reset buffer data so it can be reused for other purposes.
 *
 * @param buf Buffer to reset.
 */
static inline void rte_mbuf_simple_reset(struct rte_mbuf_simple *buf)
{
	__rte_mbuf_simple_reset(buf, 0);
}

/**
 * Clone buffer state, using the same data buffer.
 *
 * Initializes a buffer to point to the same data as an existing buffer.
 * Allows operations on the same data without altering the length and
 * offset of the original.
 *
 * @param original Buffer to clone.
 * @param clone The new clone.
 */
void rte_mbuf_simple_clone(const struct rte_mbuf_simple *original,
			  struct rte_mbuf_simple *clone);

/**
 * @brief Prepare data to be added at the end of the buffer
 *
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param len Number of bytes to increment the length with.
 *
 * @return The original tail of the buffer.
 */
void *rte_mbuf_simple_add(struct rte_mbuf_simple *buf, size_t len);

/**
 * @brief Copy given number of bytes from memory to the end of the buffer
 *
 * Increments the data length of the  buffer to account for more data at the
 * end.
 *
 * @param buf Buffer to update.
 * @param mem Location of data to be added.
 * @param len Length of data to be added
 *
 * @return The original tail of the buffer.
 */
void *rte_mbuf_simple_add_mem(struct rte_mbuf_simple *buf, const void *mem,
			     size_t len);

/**
 * @brief Add (8-bit) byte at the end of the buffer
 *
 * Increments the data length of the  buffer to account for more data at the
 * end.
 *
 * @param buf Buffer to update.
 * @param val byte value to be added.
 *
 * @return Pointer to the value added
 */
uint8_t *rte_mbuf_simple_add_u8(struct rte_mbuf_simple *buf, uint8_t val);

/**
 * @brief Add 16-bit value at the end of the buffer
 *
 * Adds 16-bit value in little endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 16-bit value to be added.
 */
void rte_mbuf_simple_add_le16(struct rte_mbuf_simple *buf, uint16_t val);

/**
 * @brief Add 16-bit value at the end of the buffer
 *
 * Adds 16-bit value in big endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 16-bit value to be added.
 */
void rte_mbuf_simple_add_be16(struct rte_mbuf_simple *buf, uint16_t val);

/**
 * @brief Add 24-bit value at the end of the buffer
 *
 * Adds 24-bit value in little endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 24-bit value to be added.
 */
void rte_mbuf_simple_add_le24(struct rte_mbuf_simple *buf, uint32_t val);

/**
 * @brief Add 24-bit value at the end of the buffer
 *
 * Adds 24-bit value in big endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 24-bit value to be added.
 */
void rte_mbuf_simple_add_be24(struct rte_mbuf_simple *buf, uint32_t val);

/**
 * @brief Add 32-bit value at the end of the buffer
 *
 * Adds 32-bit value in little endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 32-bit value to be added.
 */
void rte_mbuf_simple_add_le32(struct rte_mbuf_simple *buf, uint32_t val);

/**
 * @brief Add 32-bit value at the end of the buffer
 *
 * Adds 32-bit value in big endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 32-bit value to be added.
 */
void rte_mbuf_simple_add_be32(struct rte_mbuf_simple *buf, uint32_t val);

/**
 * @brief Add 48-bit value at the end of the buffer
 *
 * Adds 48-bit value in little endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 48-bit value to be added.
 */
void rte_mbuf_simple_add_le48(struct rte_mbuf_simple *buf, uint64_t val);

/**
 * @brief Add 48-bit value at the end of the buffer
 *
 * Adds 48-bit value in big endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 48-bit value to be added.
 */
void rte_mbuf_simple_add_be48(struct rte_mbuf_simple *buf, uint64_t val);

/**
 * @brief Add 64-bit value at the end of the buffer
 *
 * Adds 64-bit value in little endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 64-bit value to be added.
 */
void rte_mbuf_simple_add_le64(struct rte_mbuf_simple *buf, uint64_t val);

/**
 * @brief Add 64-bit value at the end of the buffer
 *
 * Adds 64-bit value in big endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 64-bit value to be added.
 */
void rte_mbuf_simple_add_be64(struct rte_mbuf_simple *buf, uint64_t val);

/**
 * @brief Remove data from the end of the buffer.
 *
 * Removes data from the end of the buffer by modifying the buffer length.
 *
 * @param buf Buffer to update.
 * @param len Number of bytes to remove.
 *
 * @return New end of the buffer data.
 */
void *rte_mbuf_simple_remove_mem(struct rte_mbuf_simple *buf, size_t len);

/**
 * @brief Remove a 8-bit value from the end of the buffer
 *
 * Same idea as with rte_mbuf_simple_remove_mem(), but a helper for operating
 * on 8-bit values.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return The 8-bit removed value
 */
uint8_t rte_mbuf_simple_remove_u8(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 16 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_simple_remove_mem(), but a helper for operating
 * on 16-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 16-bit value converted from little endian to host endian.
 */
uint16_t rte_mbuf_simple_remove_le16(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 16 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_simple_remove_mem(), but a helper for operating
 * on 16-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 16-bit value converted from big endian to host endian.
 */
uint16_t rte_mbuf_simple_remove_be16(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 24 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_simple_remove_mem(), but a helper for operating
 * on 24-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 24-bit value converted from little endian to host endian.
 */
uint32_t rte_mbuf_simple_remove_le24(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 24 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_simple_remove_mem(), but a helper for operating
 * on 24-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 24-bit value converted from big endian to host endian.
 */
uint32_t rte_mbuf_simple_remove_be24(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 32 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_simple_remove_mem(), but a helper for operating
 * on 32-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 32-bit value converted from little endian to host endian.
 */
uint32_t rte_mbuf_simple_remove_le32(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 32 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_simple_remove_mem(), but a helper for operating
 * on 32-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 32-bit value converted from big endian to host endian.
 */
uint32_t rte_mbuf_simple_remove_be32(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 48 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_simple_remove_mem(), but a helper for operating
 * on 48-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 48-bit value converted from little endian to host endian.
 */
uint64_t rte_mbuf_simple_remove_le48(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 48 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_simple_remove_mem(), but a helper for operating
 * on 48-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 48-bit value converted from big endian to host endian.
 */
uint64_t rte_mbuf_simple_remove_be48(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 64 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_simple_remove_mem(), but a helper for operating
 * on 64-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 64-bit value converted from little endian to host endian.
 */
uint64_t rte_mbuf_simple_remove_le64(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 64 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_simple_remove_mem(), but a helper for operating
 * on 64-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 64-bit value converted from big endian to host endian.
 */
uint64_t rte_mbuf_simple_remove_be64(struct rte_mbuf_simple *buf);

/**
 * @brief Prepare data to be added to the start of the buffer
 *
 * Modifies the data pointer and buffer length to account for more data
 * in the beginning of the buffer.
 *
 * @param buf Buffer to update.
 * @param len Number of bytes to add to the beginning.
 *
 * @return The new beginning of the buffer data.
 */
void *rte_mbuf_simple_push(struct rte_mbuf_simple *buf, size_t len);

/**
 * @brief Copy given number of bytes from memory to the start of the buffer.
 *
 * Modifies the data pointer and buffer length to account for more data
 * in the beginning of the buffer.
 *
 * @param buf Buffer to update.
 * @param mem Location of data to be added.
 * @param len Length of data to be added.
 *
 * @return The new beginning of the buffer data.
 */
void *rte_mbuf_simple_push_mem(struct rte_mbuf_simple *buf, const void *mem,
			      size_t len);

/**
 * @brief Push 16-bit value to the beginning of the buffer
 *
 * Adds 16-bit value in little endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 16-bit value to be pushed to the buffer.
 */
void rte_mbuf_simple_push_le16(struct rte_mbuf_simple *buf, uint16_t val);

/**
 * @brief Push 16-bit value to the beginning of the buffer
 *
 * Adds 16-bit value in big endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 16-bit value to be pushed to the buffer.
 */
void rte_mbuf_simple_push_be16(struct rte_mbuf_simple *buf, uint16_t val);

/**
 * @brief Push 8-bit value to the beginning of the buffer
 *
 * Adds 8-bit value the beginning of the buffer.
 *
 * @param buf Buffer to update.
 * @param val 8-bit value to be pushed to the buffer.
 */
void rte_mbuf_simple_push_u8(struct rte_mbuf_simple *buf, uint8_t val);

/**
 * @brief Push 24-bit value to the beginning of the buffer
 *
 * Adds 24-bit value in little endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 24-bit value to be pushed to the buffer.
 */
void rte_mbuf_simple_push_le24(struct rte_mbuf_simple *buf, uint32_t val);

/**
 * @brief Push 24-bit value to the beginning of the buffer
 *
 * Adds 24-bit value in big endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 24-bit value to be pushed to the buffer.
 */
void rte_mbuf_simple_push_be24(struct rte_mbuf_simple *buf, uint32_t val);

/**
 * @brief Push 32-bit value to the beginning of the buffer
 *
 * Adds 32-bit value in little endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 32-bit value to be pushed to the buffer.
 */
void rte_mbuf_simple_push_le32(struct rte_mbuf_simple *buf, uint32_t val);

/**
 * @brief Push 32-bit value to the beginning of the buffer
 *
 * Adds 32-bit value in big endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 32-bit value to be pushed to the buffer.
 */
void rte_mbuf_simple_push_be32(struct rte_mbuf_simple *buf, uint32_t val);

/**
 * @brief Push 48-bit value to the beginning of the buffer
 *
 * Adds 48-bit value in little endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 48-bit value to be pushed to the buffer.
 */
void rte_mbuf_simple_push_le48(struct rte_mbuf_simple *buf, uint64_t val);

/**
 * @brief Push 48-bit value to the beginning of the buffer
 *
 * Adds 48-bit value in big endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 48-bit value to be pushed to the buffer.
 */
void rte_mbuf_simple_push_be48(struct rte_mbuf_simple *buf, uint64_t val);

/**
 * @brief Push 64-bit value to the beginning of the buffer
 *
 * Adds 64-bit value in little endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 64-bit value to be pushed to the buffer.
 */
void rte_mbuf_simple_push_le64(struct rte_mbuf_simple *buf, uint64_t val);

/**
 * @brief Push 64-bit value to the beginning of the buffer
 *
 * Adds 64-bit value in big endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 64-bit value to be pushed to the buffer.
 */
void rte_mbuf_simple_push_be64(struct rte_mbuf_simple *buf, uint64_t val);

/**
 * @brief Remove data from the beginning of the buffer.
 *
 * Removes data from the beginning of the buffer by modifying the data
 * pointer and buffer length.
 *
 * @param buf Buffer to update.
 * @param len Number of bytes to remove.
 *
 * @return New beginning of the buffer data.
 */
void *rte_mbuf_simple_pull(struct rte_mbuf_simple *buf, size_t len);

/**
 * @brief Remove data from the beginning of the buffer.
 *
 * Removes data from the beginning of the buffer by modifying the data
 * pointer and buffer length.
 *
 * @param buf Buffer to update.
 * @param len Number of bytes to remove.
 *
 * @return Pointer to the old location of the buffer data.
 */
void *rte_mbuf_simple_pull_mem(struct rte_mbuf_simple *buf, size_t len);

/**
 * @brief Remove a 8-bit value from the beginning of the buffer
 *
 * Same idea as with rte_mbuf_simple_pull(), but a helper for operating
 * on 8-bit values.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return The 8-bit removed value
 */
uint8_t rte_mbuf_simple_pull_u8(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 16 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_simple_pull(), but a helper for operating
 * on 16-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 16-bit value converted from little endian to host endian.
 */
uint16_t rte_mbuf_simple_pull_le16(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 16 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_simple_pull(), but a helper for operating
 * on 16-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 16-bit value converted from big endian to host endian.
 */
uint16_t rte_mbuf_simple_pull_be16(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 24 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_simple_pull(), but a helper for operating
 * on 24-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 24-bit value converted from little endian to host endian.
 */
uint32_t rte_mbuf_simple_pull_le24(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 24 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_simple_pull(), but a helper for operating
 * on 24-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 24-bit value converted from big endian to host endian.
 */
uint32_t rte_mbuf_simple_pull_be24(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 32 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_simple_pull(), but a helper for operating
 * on 32-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 32-bit value converted from little endian to host endian.
 */
uint32_t rte_mbuf_simple_pull_le32(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 32 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_simple_pull(), but a helper for operating
 * on 32-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 32-bit value converted from big endian to host endian.
 */
uint32_t rte_mbuf_simple_pull_be32(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 48 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_simple_pull(), but a helper for operating
 * on 48-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 48-bit value converted from little endian to host endian.
 */
uint64_t rte_mbuf_simple_pull_le48(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 48 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_simple_pull(), but a helper for operating
 * on 48-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 48-bit value converted from big endian to host endian.
 */
uint64_t rte_mbuf_simple_pull_be48(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 64 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_simple_pull(), but a helper for operating
 * on 64-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 64-bit value converted from little endian to host endian.
 */
uint64_t rte_mbuf_simple_pull_le64(struct rte_mbuf_simple *buf);

/**
 * @brief Remove and convert 64 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_simple_pull(), but a helper for operating
 * on 64-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 64-bit value converted from big endian to host endian.
 */
uint64_t rte_mbuf_simple_pull_be64(struct rte_mbuf_simple *buf);

/**
 * @brief Get the tail pointer for a buffer.
 *
 * Get a pointer to the end of the data in a buffer.
 *
 * @param buf Buffer.
 *
 * @return Tail pointer for the buffer.
 */
static inline uint8_t *rte_mbuf_simple_tail(struct rte_mbuf_simple *buf)
{
	return buf->data + buf->len;
}

/**
 * @brief Check buffer headroom.
 *
 * Check how much free space there is in the beginning of the buffer.
 *
 * buf A valid pointer on a buffer
 *
 * @return Number of bytes available in the beginning of the buffer.
 */
size_t rte_mbuf_simple_headroom(struct rte_mbuf_simple *buf);

/**
 * @brief Check buffer tailroom.
 *
 * Check how much free space there is at the end of the buffer.
 *
 * @param buf A valid pointer on a buffer
 *
 * @return Number of bytes available at the end of the buffer.
 */
size_t rte_mbuf_simple_tailroom(struct rte_mbuf_simple *buf);

/**
 * @brief Check maximum rte_mbuf_simple::len value.
 *
 * This value is depending on the number of bytes being reserved as headroom.
 *
 * @param buf A valid pointer on a buffer
 *
 * @return Number of bytes usable behind the rte_mbuf_simple::data pointer.
 */
uint16_t rte_mbuf_simple_max_len(struct rte_mbuf_simple *buf);

/**
 * @brief Parsing state of a buffer.
 *
 * This is used for temporarily storing the parsing state of a buffer
 * while giving control of the parsing to a routine which we don't
 * control.
 */
struct rte_mbuf_simple_state {
	/** Offset of the data pointer from the beginning of the storage */
	uint16_t offset;
	/** Length of data */
	uint16_t len;
};

/**
 * @brief Save the parsing state of a buffer.
 *
 * Saves the parsing state of a buffer so it can be restored later.
 *
 * @param buf Buffer from which the state should be saved.
 * @param state Storage for the state.
 */
static inline void rte_mbuf_simple_save(struct rte_mbuf_simple *buf,
				       struct rte_mbuf_simple_state *state)
{
	state->offset = rte_mbuf_simple_headroom(buf);
	state->len = buf->len;
}

/**
 * @brief Restore the parsing state of a buffer.
 *
 * Restores the parsing state of a buffer from a state previously stored
 * by rte_mbuf_simple_save().
 *
 * @param buf Buffer to which the state should be restored.
 * @param state Stored state.
 */
static inline void rte_mbuf_simple_restore(struct rte_mbuf_simple *buf,
					  struct rte_mbuf_simple_state *state)
{
	buf->data = buf->__buf + state->offset;
	buf->len = state->len;
}

/**
 * @brief Network buffer representation.
 *
 * This struct is used to represent network buffers. Such buffers are
 * normally defined through the NET_BUF_POOL_*_DEFINE() APIs and allocated
 * using the rte_mbuf_alloc() API.
 */
struct rte_mbuf {
	STAILQ_ENTRY(rte_mbuf) node;
    struct rte_mbuf *frags;
    uint16_t ref;
	uint16_t allocator;
	union {
		struct {
			RTE_MBUF_SIMPLE_MEMBERS
		};
		struct rte_mbuf_simple b;
	};
};

/**
 * @brief Reset buffer
 *
 * Reset buffer data and flags so it can be reused for other purposes.
 *
 * @param buf Buffer to reset.
 */
void rte_mbuf_reset(struct rte_mbuf *buf);


/**
 * @brief Initialize buffer with the given headroom.
 *
 * The buffer is not expected to contain any data when this API is called.
 *
 * @param buf Buffer to initialize.
 * @param reserve How much headroom to reserve.
 */
void rte_mbuf_simple_reserve(struct rte_mbuf_simple *buf, size_t reserve);

/**
 * @brief Put a buffer into a list
 *
 * If the buffer contains follow-up fragments this function will take care of
 * inserting them as well into the list.
 *
 * @param list Which list to append the buffer to.
 * @param buf Buffer.
 */
static inline void rte_mbuf_put_locked(struct rte_mbuf_list *list, 
    struct rte_mbuf *buf) {
    STAILQ_INSERT_TAIL(list, buf, node);
}

/**
 * @brief Get a buffer from a list.
 *
 * If the buffer had any fragments, these will automatically be recovered from
 * the list as well and be placed to the buffer's fragment list.
 *
 * @param list Which list to take the buffer from.
 *
 * @return New buffer or NULL if the FIFO is empty.
 */
static inline struct rte_mbuf *
__rte_must_check 
rte_mbuf_get_locked(struct rte_mbuf_list *list) {
    struct rte_mbuf *p = STAILQ_FIRST(list);
    if (p)
        STAILQ_REMOVE_HEAD(list, node);
    return p;
}

/**
 * @brief Initialize buffer with the given headroom.
 *
 * The buffer is not expected to contain any data when this API is called.
 *
 * @param buf Buffer to initialize.
 * @param reserve How much headroom to reserve.
 */
static inline void rte_mbuf_reserve(struct rte_mbuf *buf, size_t reserve)
{
	rte_mbuf_simple_reserve(&buf->b, reserve);
}

/**
 * @brief Prepare data to be added at the end of the buffer
 *
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param len Number of bytes to increment the length with.
 *
 * @return The original tail of the buffer.
 */
static inline void *rte_mbuf_add(struct rte_mbuf *buf, size_t len)
{
	return rte_mbuf_simple_add(&buf->b, len);
}

/**
 * @brief Copies the given number of bytes to the end of the buffer
 *
 * Increments the data length of the  buffer to account for more data at
 * the end.
 *
 * @param buf Buffer to update.
 * @param mem Location of data to be added.
 * @param len Length of data to be added
 *
 * @return The original tail of the buffer.
 */
static inline void *rte_mbuf_add_mem(struct rte_mbuf *buf, const void *mem,
				    size_t len)
{
	return rte_mbuf_simple_add_mem(&buf->b, mem, len);
}

/**
 * @brief Add (8-bit) byte at the end of the buffer
 *
 * Increments the data length of the  buffer to account for more data at
 * the end.
 *
 * @param buf Buffer to update.
 * @param val byte value to be added.
 *
 * @return Pointer to the value added
 */
static inline uint8_t *rte_mbuf_add_u8(struct rte_mbuf *buf, uint8_t val)
{
	return rte_mbuf_simple_add_u8(&buf->b, val);
}

/**
 * @brief Add 16-bit value at the end of the buffer
 *
 * Adds 16-bit value in little endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 16-bit value to be added.
 */
static inline void rte_mbuf_add_le16(struct rte_mbuf *buf, uint16_t val)
{
	rte_mbuf_simple_add_le16(&buf->b, val);
}

/**
 * @brief Add 16-bit value at the end of the buffer
 *
 * Adds 16-bit value in big endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 16-bit value to be added.
 */
static inline void rte_mbuf_add_be16(struct rte_mbuf *buf, uint16_t val)
{
	rte_mbuf_simple_add_be16(&buf->b, val);
}

/**
 * @brief Add 24-bit value at the end of the buffer
 *
 * Adds 24-bit value in little endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 24-bit value to be added.
 */
static inline void rte_mbuf_add_le24(struct rte_mbuf *buf, uint32_t val)
{
	rte_mbuf_simple_add_le24(&buf->b, val);
}

/**
 * @brief Add 24-bit value at the end of the buffer
 *
 * Adds 24-bit value in big endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 24-bit value to be added.
 */
static inline void rte_mbuf_add_be24(struct rte_mbuf *buf, uint32_t val)
{
	rte_mbuf_simple_add_be24(&buf->b, val);
}

/**
 * @brief Add 32-bit value at the end of the buffer
 *
 * Adds 32-bit value in little endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 32-bit value to be added.
 */
static inline void rte_mbuf_add_le32(struct rte_mbuf *buf, uint32_t val)
{
	rte_mbuf_simple_add_le32(&buf->b, val);
}

/**
 * @brief Add 32-bit value at the end of the buffer
 *
 * Adds 32-bit value in big endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 32-bit value to be added.
 */
static inline void rte_mbuf_add_be32(struct rte_mbuf *buf, uint32_t val)
{
	rte_mbuf_simple_add_be32(&buf->b, val);
}

/**
 * @brief Add 48-bit value at the end of the buffer
 *
 * Adds 48-bit value in little endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 48-bit value to be added.
 */
static inline void rte_mbuf_add_le48(struct rte_mbuf *buf, uint64_t val)
{
	rte_mbuf_simple_add_le48(&buf->b, val);
}

/**
 * @brief Add 48-bit value at the end of the buffer
 *
 * Adds 48-bit value in big endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 48-bit value to be added.
 */
static inline void rte_mbuf_add_be48(struct rte_mbuf *buf, uint64_t val)
{
	rte_mbuf_simple_add_be48(&buf->b, val);
}

/**
 * @brief Add 64-bit value at the end of the buffer
 *
 * Adds 64-bit value in little endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 64-bit value to be added.
 */
static inline void rte_mbuf_add_le64(struct rte_mbuf *buf, uint64_t val)
{
	rte_mbuf_simple_add_le64(&buf->b, val);
}

/**
 * @brief Add 64-bit value at the end of the buffer
 *
 * Adds 64-bit value in big endian format at the end of buffer.
 * Increments the data length of a buffer to account for more data
 * at the end.
 *
 * @param buf Buffer to update.
 * @param val 64-bit value to be added.
 */
static inline void rte_mbuf_add_be64(struct rte_mbuf *buf, uint64_t val)
{
	rte_mbuf_simple_add_be64(&buf->b, val);
}

/**
 * @brief Remove data from the end of the buffer.
 *
 * Removes data from the end of the buffer by modifying the buffer length.
 *
 * @param buf Buffer to update.
 * @param len Number of bytes to remove.
 *
 * @return New end of the buffer data.
 */
static inline void *rte_mbuf_remove_mem(struct rte_mbuf *buf, size_t len)
{
	return rte_mbuf_simple_remove_mem(&buf->b, len);
}

/**
 * @brief Remove a 8-bit value from the end of the buffer
 *
 * Same idea as with rte_mbuf_remove_mem(), but a helper for operating on
 * 8-bit values.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return The 8-bit removed value
 */
static inline uint8_t rte_mbuf_remove_u8(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_remove_u8(&buf->b);
}

/**
 * @brief Remove and convert 16 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_remove_mem(), but a helper for operating on
 * 16-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 16-bit value converted from little endian to host endian.
 */
static inline uint16_t rte_mbuf_remove_le16(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_remove_le16(&buf->b);
}

/**
 * @brief Remove and convert 16 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_remove_mem(), but a helper for operating on
 * 16-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 16-bit value converted from big endian to host endian.
 */
static inline uint16_t rte_mbuf_remove_be16(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_remove_be16(&buf->b);
}

/**
 * @brief Remove and convert 24 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_remove_mem(), but a helper for operating on
 * 24-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 24-bit value converted from big endian to host endian.
 */
static inline uint32_t rte_mbuf_remove_be24(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_remove_be24(&buf->b);
}

/**
 * @brief Remove and convert 24 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_remove_mem(), but a helper for operating on
 * 24-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 24-bit value converted from little endian to host endian.
 */
static inline uint32_t rte_mbuf_remove_le24(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_remove_le24(&buf->b);
}

/**
 * @brief Remove and convert 32 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_remove_mem(), but a helper for operating on
 * 32-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 32-bit value converted from little endian to host endian.
 */
static inline uint32_t rte_mbuf_remove_le32(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_remove_le32(&buf->b);
}

/**
 * @brief Remove and convert 32 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_remove_mem(), but a helper for operating on
 * 32-bit big endian data.
 *
 * @param buf A valid pointer on a buffer
 *
 * @return 32-bit value converted from big endian to host endian.
 */
static inline uint32_t rte_mbuf_remove_be32(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_remove_be32(&buf->b);
}

/**
 * @brief Remove and convert 48 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_remove_mem(), but a helper for operating on
 * 48-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 48-bit value converted from little endian to host endian.
 */
static inline uint64_t rte_mbuf_remove_le48(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_remove_le48(&buf->b);
}

/**
 * @brief Remove and convert 48 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_remove_mem(), but a helper for operating on
 * 48-bit big endian data.
 *
 * @param buf A valid pointer on a buffer
 *
 * @return 48-bit value converted from big endian to host endian.
 */
static inline uint64_t rte_mbuf_remove_be48(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_remove_be48(&buf->b);
}

/**
 * @brief Remove and convert 64 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_remove_mem(), but a helper for operating on
 * 64-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 64-bit value converted from little endian to host endian.
 */
static inline uint64_t rte_mbuf_remove_le64(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_remove_le64(&buf->b);
}

/**
 * @brief Remove and convert 64 bits from the end of the buffer.
 *
 * Same idea as with rte_mbuf_remove_mem(), but a helper for operating on
 * 64-bit big endian data.
 *
 * @param buf A valid pointer on a buffer
 *
 * @return 64-bit value converted from big endian to host endian.
 */
static inline uint64_t rte_mbuf_remove_be64(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_remove_be64(&buf->b);
}

/**
 * @brief Prepare data to be added at the start of the buffer
 *
 * Modifies the data pointer and buffer length to account for more data
 * in the beginning of the buffer.
 *
 * @param buf Buffer to update.
 * @param len Number of bytes to add to the beginning.
 *
 * @return The new beginning of the buffer data.
 */
static inline void *rte_mbuf_push(struct rte_mbuf *buf, size_t len)
{
	return rte_mbuf_simple_push(&buf->b, len);
}

/**
 * @brief Copies the given number of bytes to the start of the buffer
 *
 * Modifies the data pointer and buffer length to account for more data
 * in the beginning of the buffer.
 *
 * @param buf Buffer to update.
 * @param mem Location of data to be added.
 * @param len Length of data to be added.
 *
 * @return The new beginning of the buffer data.
 */
static inline void *rte_mbuf_push_mem(struct rte_mbuf *buf, const void *mem,
				     size_t len)
{
	return rte_mbuf_simple_push_mem(&buf->b, mem, len);
}

/**
 * @brief Push 8-bit value to the beginning of the buffer
 *
 * Adds 8-bit value the beginning of the buffer.
 *
 * @param buf Buffer to update.
 * @param val 8-bit value to be pushed to the buffer.
 */
static inline void rte_mbuf_push_u8(struct rte_mbuf *buf, uint8_t val)
{
	rte_mbuf_simple_push_u8(&buf->b, val);
}

/**
 * @brief Push 16-bit value to the beginning of the buffer
 *
 * Adds 16-bit value in little endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 16-bit value to be pushed to the buffer.
 */
static inline void rte_mbuf_push_le16(struct rte_mbuf *buf, uint16_t val)
{
	rte_mbuf_simple_push_le16(&buf->b, val);
}

/**
 * @brief Push 16-bit value to the beginning of the buffer
 *
 * Adds 16-bit value in big endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 16-bit value to be pushed to the buffer.
 */
static inline void rte_mbuf_push_be16(struct rte_mbuf *buf, uint16_t val)
{
	rte_mbuf_simple_push_be16(&buf->b, val);
}

/**
 * @brief Push 24-bit value to the beginning of the buffer
 *
 * Adds 24-bit value in little endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 24-bit value to be pushed to the buffer.
 */
static inline void rte_mbuf_push_le24(struct rte_mbuf *buf, uint32_t val)
{
	rte_mbuf_simple_push_le24(&buf->b, val);
}

/**
 * @brief Push 24-bit value to the beginning of the buffer
 *
 * Adds 24-bit value in big endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 24-bit value to be pushed to the buffer.
 */
static inline void rte_mbuf_push_be24(struct rte_mbuf *buf, uint32_t val)
{
	rte_mbuf_simple_push_be24(&buf->b, val);
}

/**
 * @brief Push 32-bit value to the beginning of the buffer
 *
 * Adds 32-bit value in little endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 32-bit value to be pushed to the buffer.
 */
static inline void rte_mbuf_push_le32(struct rte_mbuf *buf, uint32_t val)
{
	rte_mbuf_simple_push_le32(&buf->b, val);
}

/**
 * @brief Push 32-bit value to the beginning of the buffer
 *
 * Adds 32-bit value in big endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 32-bit value to be pushed to the buffer.
 */
static inline void rte_mbuf_push_be32(struct rte_mbuf *buf, uint32_t val)
{
	rte_mbuf_simple_push_be32(&buf->b, val);
}

/**
 * @brief Push 48-bit value to the beginning of the buffer
 *
 * Adds 48-bit value in little endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 48-bit value to be pushed to the buffer.
 */
static inline void rte_mbuf_push_le48(struct rte_mbuf *buf, uint64_t val)
{
	rte_mbuf_simple_push_le48(&buf->b, val);
}

/**
 * @brief Push 48-bit value to the beginning of the buffer
 *
 * Adds 48-bit value in big endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 48-bit value to be pushed to the buffer.
 */
static inline void rte_mbuf_push_be48(struct rte_mbuf *buf, uint64_t val)
{
	rte_mbuf_simple_push_be48(&buf->b, val);
}

/**
 * @brief Push 64-bit value to the beginning of the buffer
 *
 * Adds 64-bit value in little endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 64-bit value to be pushed to the buffer.
 */
static inline void rte_mbuf_push_le64(struct rte_mbuf *buf, uint64_t val)
{
	rte_mbuf_simple_push_le64(&buf->b, val);
}

/**
 * @brief Push 64-bit value to the beginning of the buffer
 *
 * Adds 64-bit value in big endian format to the beginning of the
 * buffer.
 *
 * @param buf Buffer to update.
 * @param val 64-bit value to be pushed to the buffer.
 */
static inline void rte_mbuf_push_be64(struct rte_mbuf *buf, uint64_t val)
{
	rte_mbuf_simple_push_be64(&buf->b, val);
}

/**
 * @brief Remove data from the beginning of the buffer.
 *
 * Removes data from the beginning of the buffer by modifying the data
 * pointer and buffer length.
 *
 * @param buf Buffer to update.
 * @param len Number of bytes to remove.
 *
 * @return New beginning of the buffer data.
 */
static inline void *rte_mbuf_pull(struct rte_mbuf *buf, size_t len)
{
	return rte_mbuf_simple_pull(&buf->b, len);
}

/**
 * @brief Remove data from the beginning of the buffer.
 *
 * Removes data from the beginning of the buffer by modifying the data
 * pointer and buffer length.
 *
 * @param buf Buffer to update.
 * @param len Number of bytes to remove.
 *
 * @return Pointer to the old beginning of the buffer data.
 */
static inline void *rte_mbuf_pull_mem(struct rte_mbuf *buf, size_t len)
{
	return rte_mbuf_simple_pull_mem(&buf->b, len);
}

/**
 * @brief Remove a 8-bit value from the beginning of the buffer
 *
 * Same idea as with rte_mbuf_pull(), but a helper for operating on
 * 8-bit values.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return The 8-bit removed value
 */
static inline uint8_t rte_mbuf_pull_u8(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_pull_u8(&buf->b);
}

/**
 * @brief Remove and convert 16 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_pull(), but a helper for operating on
 * 16-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 16-bit value converted from little endian to host endian.
 */
static inline uint16_t rte_mbuf_pull_le16(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_pull_le16(&buf->b);
}

/**
 * @brief Remove and convert 16 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_pull(), but a helper for operating on
 * 16-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 16-bit value converted from big endian to host endian.
 */
static inline uint16_t rte_mbuf_pull_be16(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_pull_be16(&buf->b);
}

/**
 * @brief Remove and convert 24 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_pull(), but a helper for operating on
 * 24-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 24-bit value converted from little endian to host endian.
 */
static inline uint32_t rte_mbuf_pull_le24(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_pull_le24(&buf->b);
}

/**
 * @brief Remove and convert 24 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_pull(), but a helper for operating on
 * 24-bit big endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 24-bit value converted from big endian to host endian.
 */
static inline uint32_t rte_mbuf_pull_be24(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_pull_be24(&buf->b);
}

/**
 * @brief Remove and convert 32 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_pull(), but a helper for operating on
 * 32-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 32-bit value converted from little endian to host endian.
 */
static inline uint32_t rte_mbuf_pull_le32(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_pull_le32(&buf->b);
}

/**
 * @brief Remove and convert 32 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_pull(), but a helper for operating on
 * 32-bit big endian data.
 *
 * @param buf A valid pointer on a buffer
 *
 * @return 32-bit value converted from big endian to host endian.
 */
static inline uint32_t rte_mbuf_pull_be32(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_pull_be32(&buf->b);
}

/**
 * @brief Remove and convert 48 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_pull(), but a helper for operating on
 * 48-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 48-bit value converted from little endian to host endian.
 */
static inline uint64_t rte_mbuf_pull_le48(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_pull_le48(&buf->b);
}

/**
 * @brief Remove and convert 48 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_pull(), but a helper for operating on
 * 48-bit big endian data.
 *
 * @param buf A valid pointer on a buffer
 *
 * @return 48-bit value converted from big endian to host endian.
 */
static inline uint64_t rte_mbuf_pull_be48(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_pull_be48(&buf->b);
}

/**
 * @brief Remove and convert 64 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_pull(), but a helper for operating on
 * 64-bit little endian data.
 *
 * @param buf A valid pointer on a buffer.
 *
 * @return 64-bit value converted from little endian to host endian.
 */
static inline uint64_t rte_mbuf_pull_le64(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_pull_le64(&buf->b);
}

/**
 * @brief Remove and convert 64 bits from the beginning of the buffer.
 *
 * Same idea as with rte_mbuf_pull(), but a helper for operating on
 * 64-bit big endian data.
 *
 * @param buf A valid pointer on a buffer
 *
 * @return 64-bit value converted from big endian to host endian.
 */
static inline uint64_t rte_mbuf_pull_be64(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_pull_be64(&buf->b);
}

/**
 * @brief Check buffer tailroom.
 *
 * Check how much free space there is at the end of the buffer.
 *
 * @param buf A valid pointer on a buffer
 *
 * @return Number of bytes available at the end of the buffer.
 */
static inline size_t rte_mbuf_tailroom(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_tailroom(&buf->b);
}

/**
 * @brief Check buffer headroom.
 *
 * Check how much free space there is in the beginning of the buffer.
 *
 * buf A valid pointer on a buffer
 *
 * @return Number of bytes available in the beginning of the buffer.
 */
static inline size_t rte_mbuf_headroom(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_headroom(&buf->b);
}

/**
 * @brief Check maximum rte_mbuf::len value.
 *
 * This value is depending on the number of bytes being reserved as headroom.
 *
 * @param buf A valid pointer on a buffer
 *
 * @return Number of bytes usable behind the rte_mbuf::data pointer.
 */
static inline uint16_t rte_mbuf_max_len(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_max_len(&buf->b);
}

/**
 * @brief Get the tail pointer for a buffer.
 *
 * Get a pointer to the end of the data in a buffer.
 *
 * @param buf Buffer.
 *
 * @return Tail pointer for the buffer.
 */
static inline uint8_t *rte_mbuf_tail(struct rte_mbuf *buf)
{
	return rte_mbuf_simple_tail(&buf->b);
}

/**
 * @brief Increment the reference count of a buffer.
 *
 * @param buf A valid pointer on a buffer
 *
 * @return the buffer newly referenced
 */
struct rte_mbuf *__rte_must_check rte_mbuf_ref(struct rte_mbuf *buf);

/**
 * @brief Find the last fragment in the fragment list.
 *
 * @return Pointer to last fragment in the list.
 */
static inline struct rte_mbuf *rte_mbuf_frag_last(struct rte_mbuf *buf) {
	while (buf->frags) 
		buf = buf->frags;
	return buf;
}

/**
 * @brief Insert a new fragment to a chain of bufs.
 *
 * Insert a new fragment into the buffer fragments list after the parent.
 *
 * Note: This function takes ownership of the fragment reference so the
 * caller is not required to unref.
 *
 * @param parent Parent buffer/fragment.
 * @param frag Fragment to insert.
 */
static inline void 
rte_mbuf_frag_insert(struct rte_mbuf *parent, struct rte_mbuf *frag) {
	if (parent->frags) 
		rte_mbuf_frag_last(frag)->frags = parent->frags;
	/* Take ownership of the fragment reference */
	parent->frags = frag;
}

/**
 * @brief Add a new fragment to the end of a chain of bufs.
 *
 * Append a new fragment into the buffer fragments list.
 *
 * Note: This function takes ownership of the fragment reference so the
 * caller is not required to unref.
 *
 * @param head Head of the fragment chain.
 * @param frag Fragment to add.
 *
 * @return New head of the fragment chain. Either head (if head
 *         was non-NULL) or frag (if head was NULL).
 */
static inline struct rte_mbuf *
rte_mbuf_frag_add(struct rte_mbuf *head, struct rte_mbuf *frag) {
	if (!head) 
		return rte_mbuf_ref(frag);
	rte_mbuf_frag_insert(rte_mbuf_frag_last(head), frag);
	return head;
}

/**
 * @brief Delete existing fragment from a chain of bufs.
 *
 * @param parent Parent buffer/fragment, or NULL if there is no parent.
 * @param frag Fragment to delete.
 *
 * @return Pointer to the buffer following the fragment, or NULL if it
 *         had no further fragments.
 */
struct rte_mbuf *rte_mbuf_frag_del(struct rte_mbuf *parent, struct rte_mbuf *frag);


/**
 * @brief Skip N number of bytes in a rte_mbuf
 *
 * @details Skip N number of bytes starting from fragment's offset. If the total
 * length of data is placed in multiple fragments, this function will skip from
 * all fragments until it reaches N number of bytes.  Any fully skipped buffers
 * are removed from the rte_mbuf list.
 *
 * @param buf Network buffer.
 * @param len Total length of data to be skipped.
 *
 * @return Pointer to the fragment or
 *         NULL and pos is 0 after successful skip,
 *         NULL and pos is 0xffff otherwise.
 */
static inline struct rte_mbuf *rte_mbuf_skip(struct rte_mbuf *buf, size_t len)
{
	while (buf && len--) {
		rte_mbuf_pull_u8(buf);
		if (!buf->len) 
			buf = rte_mbuf_frag_del(NULL, buf);
	}
	return buf;
}

/**
 * @brief Calculate amount of bytes stored in fragments.
 *
 * Calculates the total amount of data stored in the given buffer and the
 * fragments linked to it.
 *
 * @param buf Buffer to start off with.
 *
 * @return Number of bytes in the buffer and its fragments.
 */
static inline size_t rte_mbuf_frags_len(struct rte_mbuf *buf)
{
	size_t bytes = 0;
	while (buf) {
		bytes += buf->len;
		buf = buf->frags;
	}
	return bytes;
}

#ifdef __cplusplus
}
#endif
#endif /* BASEWORK_MBUF_H_ */
