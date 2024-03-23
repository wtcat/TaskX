
/*
 * Copyright (c) 2015-2019 Intel Corporation
 * Copyright (c) 2023 wtcat
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#define pr_fmt(fmt) "<mbuf>: " fmt
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "basework/byteorder.h"
#include "basework/mbuf.h"
#include "basework/log.h"

#ifndef UNALIGNED_GET
#define UNALIGNED_GET(p)						\
	__extension__ ({							\
		struct  __attribute__((__packed__)) {				\
			__typeof__(*(p)) __v;					\
		} *__p = (__typeof__(__p)) (p);					\
		__p->__v;							\
	})
#endif


void rte_mbuf_simple_attach(struct rte_mbuf_simple *buf,
	void *data, size_t size)
{
	buf->__buf = data;
	buf->data  = data;
	buf->size  = size;
	buf->len   = size;
}

struct rte_mbuf *rte_mbuf_ref(struct rte_mbuf *buf) 
{
    buf->ref++;
    return buf;
}

// void rte_mbuf_unref(struct rte_mbuf *buf)
// {
// 	assert(buf != NULL);
// 	while (buf) {
// 		struct rte_mbuf *frags = buf->frags;
// 		struct net_buf_pool *pool;

// 		if (--buf->ref > 0)
// 			return;

// 		if (buf->__buf) {
// 			data_unref(buf, buf->__buf);
// 			buf->__buf = NULL;
// 		}
// 		buf->data = NULL;
// 		buf->frags = NULL;

// 		pool = net_buf_pool_get(buf->pool_id);
// 		if (pool->destroy) {
// 			pool->destroy(buf);
// 		} else {
// 			net_buf_destroy(buf);
// 		}
// 		buf = frags;
// 	}
// }

void rte_mbuf_simple_reserve(struct rte_mbuf_simple *buf, size_t reserve)
{
	assert(buf);
	assert(buf->len == 0U);
	pr_dbg("buf %p reserve %zu", buf, reserve);
	buf->data = buf->__buf + reserve;
}

void rte_mbuf_simple_clone(const struct rte_mbuf_simple *original,
			  struct rte_mbuf_simple *clone)
{
	memcpy(clone, original, sizeof(struct rte_mbuf_simple));
}

void *rte_mbuf_simple_add(struct rte_mbuf_simple *buf, size_t len)
{
	uint8_t *tail = rte_mbuf_simple_tail(buf);

	pr_dbg("buf %p len %zu", buf, len);
	assert(rte_mbuf_simple_tailroom(buf) >= len);
	buf->len += len;
	return tail;
}

void *rte_mbuf_simple_add_mem(struct rte_mbuf_simple *buf, const void *mem,
			     size_t len)
{
	pr_dbg("buf %p len %zu", buf, len);
	return memcpy(rte_mbuf_simple_add(buf, len), mem, len);
}

uint8_t *rte_mbuf_simple_add_u8(struct rte_mbuf_simple *buf, uint8_t val)
{
	uint8_t *u8;

	pr_dbg("buf %p val 0x%02x", buf, val);
	u8 = rte_mbuf_simple_add(buf, 1);
	*u8 = val;
	return u8;
}

void rte_mbuf_simple_add_le16(struct rte_mbuf_simple *buf, uint16_t val)
{
	pr_dbg("buf %p val %u", buf, val);
	rte_put_le16(val, rte_mbuf_simple_add(buf, sizeof(val)));
}

void rte_mbuf_simple_add_be16(struct rte_mbuf_simple *buf, uint16_t val)
{
	pr_dbg("buf %p val %u", buf, val);
	rte_put_be16(val, rte_mbuf_simple_add(buf, sizeof(val)));
}

void rte_mbuf_simple_add_le32(struct rte_mbuf_simple *buf, uint32_t val)
{
	pr_dbg("buf %p val %u", buf, val);
	rte_put_le32(val, rte_mbuf_simple_add(buf, sizeof(val)));
}

void rte_mbuf_simple_add_be32(struct rte_mbuf_simple *buf, uint32_t val)
{
	pr_dbg("buf %p val %u", buf, val);
	rte_put_be32(val, rte_mbuf_simple_add(buf, sizeof(val)));
}

void rte_mbuf_simple_add_le64(struct rte_mbuf_simple *buf, uint64_t val)
{
	pr_dbg("buf %p val %" PRIu64, buf, val);
	rte_put_le64(val, rte_mbuf_simple_add(buf, sizeof(val)));
}

void rte_mbuf_simple_add_be64(struct rte_mbuf_simple *buf, uint64_t val)
{
	pr_dbg("buf %p val %" PRIu64, buf, val);
	rte_put_be64(val, rte_mbuf_simple_add(buf, sizeof(val)));
}

void *rte_mbuf_simple_remove_mem(struct rte_mbuf_simple *buf, size_t len)
{
	pr_dbg("buf %p len %zu", buf, len);
	assert(buf->len >= len);
	buf->len -= len;
	return buf->data + buf->len;
}

uint8_t rte_mbuf_simple_remove_u8(struct rte_mbuf_simple *buf)
{
	uint8_t val;
	void *ptr;

	ptr = rte_mbuf_simple_remove_mem(buf, sizeof(val));
	val = *(uint8_t *)ptr;
	return val;
}

uint16_t rte_mbuf_simple_remove_le16(struct rte_mbuf_simple *buf)
{
	uint16_t val;
	void *ptr;

	ptr = rte_mbuf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((uint16_t *)ptr);
	return rte_le16_to_cpu(val);
}

uint16_t rte_mbuf_simple_remove_be16(struct rte_mbuf_simple *buf)
{
	uint16_t val;
	void *ptr;

	ptr = rte_mbuf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((uint16_t *)ptr);
	return rte_be16_to_cpu(val);
}

uint32_t rte_mbuf_simple_remove_le32(struct rte_mbuf_simple *buf)
{
	uint32_t val;
	void *ptr;

	ptr = rte_mbuf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((uint32_t *)ptr);

	return rte_le32_to_cpu(val);
}

uint32_t rte_mbuf_simple_remove_be32(struct rte_mbuf_simple *buf)
{
	uint32_t val;
	void *ptr;

	ptr = rte_mbuf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((uint32_t *)ptr);

	return rte_be32_to_cpu(val);
}

uint64_t rte_mbuf_simple_remove_le64(struct rte_mbuf_simple *buf)
{
	uint64_t val;
	void *ptr;

	ptr = rte_mbuf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((uint64_t *)ptr);
	return rte_le64_to_cpu(val);
}

uint64_t rte_mbuf_simple_remove_be64(struct rte_mbuf_simple *buf)
{
	uint64_t val;
	void *ptr;

	ptr = rte_mbuf_simple_remove_mem(buf, sizeof(val));
	val = UNALIGNED_GET((uint64_t *)ptr);
	return rte_be64_to_cpu(val);
}

void *rte_mbuf_simple_push(struct rte_mbuf_simple *buf, size_t len)
{
	pr_dbg("buf %p len %zu", buf, len);
	assert(rte_mbuf_simple_headroom(buf) >= len);

	buf->data -= len;
	buf->len += len;
	return buf->data;
}

void *rte_mbuf_simple_push_mem(struct rte_mbuf_simple *buf, const void *mem,
			      size_t len)
{
	pr_dbg("buf %p len %zu", buf, len);
	return memcpy(rte_mbuf_simple_push(buf, len), mem, len);
}

void rte_mbuf_simple_push_le16(struct rte_mbuf_simple *buf, uint16_t val)
{
	pr_dbg("buf %p val %u", buf, val);
	rte_put_le16(val, rte_mbuf_simple_push(buf, sizeof(val)));
}

void rte_mbuf_simple_push_be16(struct rte_mbuf_simple *buf, uint16_t val)
{
	pr_dbg("buf %p val %u", buf, val);
	rte_put_be16(val, rte_mbuf_simple_push(buf, sizeof(val)));
}

void rte_mbuf_simple_push_u8(struct rte_mbuf_simple *buf, uint8_t val)
{
	uint8_t *data = rte_mbuf_simple_push(buf, 1);
	*data = val;
}

void rte_mbuf_simple_push_le24(struct rte_mbuf_simple *buf, uint32_t val)
{
	pr_dbg("buf %p val %u", buf, val);
	rte_put_le24(val, rte_mbuf_simple_push(buf, 3));
}

void rte_mbuf_simple_push_be24(struct rte_mbuf_simple *buf, uint32_t val)
{
	pr_dbg("buf %p val %u", buf, val);
	rte_put_be24(val, rte_mbuf_simple_push(buf, 3));
}

void rte_mbuf_simple_push_le32(struct rte_mbuf_simple *buf, uint32_t val)
{
	pr_dbg("buf %p val %u", buf, val);
	rte_put_le32(val, rte_mbuf_simple_push(buf, sizeof(val)));
}

void rte_mbuf_simple_push_be32(struct rte_mbuf_simple *buf, uint32_t val)
{
	pr_dbg("buf %p val %u", buf, val);
	rte_put_be32(val, rte_mbuf_simple_push(buf, sizeof(val)));
}

void rte_mbuf_simple_push_le48(struct rte_mbuf_simple *buf, uint64_t val)
{
	pr_dbg("buf %p val %" PRIu64, buf, val);
	rte_put_le48(val, rte_mbuf_simple_push(buf, 6));
}

void rte_mbuf_simple_push_be48(struct rte_mbuf_simple *buf, uint64_t val)
{
	pr_dbg("buf %p val %" PRIu64, buf, val);
	rte_put_be48(val, rte_mbuf_simple_push(buf, 6));
}

void rte_mbuf_simple_push_le64(struct rte_mbuf_simple *buf, uint64_t val)
{
	pr_dbg("buf %p val %" PRIu64, buf, val);

	rte_put_le64(val, rte_mbuf_simple_push(buf, sizeof(val)));
}

void rte_mbuf_simple_push_be64(struct rte_mbuf_simple *buf, uint64_t val)
{
	pr_dbg("buf %p val %" PRIu64, buf, val);
	rte_put_be64(val, rte_mbuf_simple_push(buf, sizeof(val)));
}

void *rte_mbuf_simple_pull(struct rte_mbuf_simple *buf, size_t len)
{
	pr_dbg("buf %p len %zu", buf, len);
	assert(buf->len >= len);
	buf->len -= len;
	return buf->data += len;
}

void *rte_mbuf_simple_pull_mem(struct rte_mbuf_simple *buf, size_t len)
{
	void *data = buf->data;
	pr_dbg("buf %p len %zu", buf, len);
	assert(buf->len >= len);
	buf->len -= len;
	buf->data += len;
	return data;
}

uint8_t rte_mbuf_simple_pull_u8(struct rte_mbuf_simple *buf)
{
	uint8_t val;

	val = buf->data[0];
	rte_mbuf_simple_pull(buf, 1);
	return val;
}

uint16_t rte_mbuf_simple_pull_le16(struct rte_mbuf_simple *buf)
{
	uint16_t val;

	val = UNALIGNED_GET((uint16_t *)buf->data);
	rte_mbuf_simple_pull(buf, sizeof(val));
	return rte_le16_to_cpu(val);
}

uint16_t rte_mbuf_simple_pull_be16(struct rte_mbuf_simple *buf)
{
	uint16_t val;

	val = UNALIGNED_GET((uint16_t *)buf->data);
	rte_mbuf_simple_pull(buf, sizeof(val));
	return rte_be16_to_cpu(val);
}

uint32_t rte_mbuf_simple_pull_le32(struct rte_mbuf_simple *buf)
{
	uint32_t val;

	val = UNALIGNED_GET((uint32_t *)buf->data);
	rte_mbuf_simple_pull(buf, sizeof(val));
	return rte_le32_to_cpu(val);
}

uint32_t rte_mbuf_simple_pull_be32(struct rte_mbuf_simple *buf)
{
	uint32_t val;

	val = UNALIGNED_GET((uint32_t *)buf->data);
	rte_mbuf_simple_pull(buf, sizeof(val));
	return rte_be32_to_cpu(val);
}

uint64_t rte_mbuf_simple_pull_le64(struct rte_mbuf_simple *buf)
{
	uint64_t val;

	val = UNALIGNED_GET((uint64_t *)buf->data);
	rte_mbuf_simple_pull(buf, sizeof(val));
	return rte_le64_to_cpu(val);
}

uint64_t rte_mbuf_simple_pull_be64(struct rte_mbuf_simple *buf)
{
	uint64_t val;

	val = UNALIGNED_GET((uint64_t *)buf->data);
	rte_mbuf_simple_pull(buf, sizeof(val));
	return rte_be64_to_cpu(val);
}

size_t rte_mbuf_simple_headroom(struct rte_mbuf_simple *buf)
{
	return buf->data - buf->__buf;
}

size_t rte_mbuf_simple_tailroom(struct rte_mbuf_simple *buf)
{
	return buf->size - rte_mbuf_simple_headroom(buf) - buf->len;
}

uint16_t rte_mbuf_simple_max_len(struct rte_mbuf_simple *buf)
{
	return buf->size - rte_mbuf_simple_headroom(buf);
}
