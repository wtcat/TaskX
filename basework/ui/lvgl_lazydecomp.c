/*
 * Copyright 2024 wtcat
 * 
 * Note: All APIs has no thread-safe
 */
#include <assert.h>

#include "basework/container/list.h"
#include "basework/ui/lvgl_lazydecomp.h"

#ifdef CONFIG_LAZYCACHE_MAX_NODES
#define MAX_IMAGE_NODES CONFIG_LAZYCACHE_MAX_NODES
#else
#define MAX_IMAGE_NODES 32
#endif

#define CACHE_ALIGNED   4
#define CACHE_LIMIT(n)  (size_t)(((uintptr_t)(n) * 2) / 3)

#define ALIGNED_UP_ADD(p, size, align) \
	(char *)(((uintptr_t)p + size + align - 1) & ~(align - 1))

struct image_node {
	uint32_t         key;	 /* Picture offset address */
	const void      *data;   /* Point to picture data */
	size_t           size;	 /* Picture data size */
	struct rte_list  lru;
	struct rte_hnode node;
};

struct lazy_cache {
#define HASH_LOG 4
#define HASH_SIZE (1 << HASH_LOG)
#define HASH_MASK (HASH_SIZE - 1)
#define KEY_HASH(key) (((uintptr_t)(key) ^ ((uintptr_t)(key) >> 8)) & HASH_MASK)
	struct rte_list   lru;
	struct rte_hlist  slots[HASH_SIZE];
	struct image_node inodes[MAX_IMAGE_NODES];
	size_t            cache_limited;
	uint16_t          policy;
	uint16_t          cache_hits;
	uint16_t          cache_misses;
	uint16_t          cache_resets;
	uint16_t          node_misses;
	uint8_t           invalid_pending;
	uint8_t           cache_dirty;
	char             *first_avalible;

	/*
	 * Main memory pool 
	 */
	char             *area;
	char             *start;
	char             *end;
	char             *freeptr;
	void             (*release)(void *p);

#ifdef CONFIG_LVGL_LAZYDECOMP_AUXMEM 
	/*
	 * Aux memory pool
	 */
	char             *aux_area;
	char             *aux_start;
	char             *aux_end;
	char             *aux_freeptr;
	void             (*aux_release)(void *p);
#endif

	/*
	 * Hardware accelarator 
	 */
	void             (*wait_complete)(void *context, int devid);
};

static struct lazy_cache cache_controller;

static inline void *alloc_cache_buffer(struct lazy_cache *cache, size_t size, void ***phead) {
	void *p;

	if (cache->end - cache->freeptr >= size) {
		*phead = (void **)&cache->freeptr;
		p = cache->freeptr;
		cache->freeptr = ALIGNED_UP_ADD(cache->freeptr, size, CACHE_ALIGNED);
		return p;
	}

#ifdef CONFIG_LVGL_LAZYDECOMP_AUXMEM 
	if (cache->aux_end - cache->aux_freeptr >= size) {
		*phead = (void **)&cache->aux_freeptr;
		p = cache->aux_freeptr;
		cache->aux_freeptr = ALIGNED_UP_ADD(cache->aux_freeptr, size, CACHE_ALIGNED);
		return p;
	}
#endif /* CONFIG_LVGL_LAZYDECOMP_AUXMEM */

	return NULL;
}

static inline struct image_node *alloc_node(struct lazy_cache *cache) {
	char *p = cache->first_avalible;
	if (p != NULL)
		cache->first_avalible = *(char **)p;
	return (struct image_node *)p;
}

static void lazy_cache_reset(struct lazy_cache *cache) {
	if (cache->cache_dirty) {
		cache->first_avalible = NULL;
		cache->freeptr        = cache->start;
		cache->cache_dirty    = 0;
		cache->cache_hits     = 0;
		cache->cache_misses   = 0;
		cache->node_misses    = 0;
		cache->cache_resets++;

#ifdef CONFIG_LVGL_LAZYDECOMP_AUXMEM
		cache->aux_freeptr    = cache->aux_start;
#endif

		char *p = (char *)cache->inodes;
		for (size_t i = 0; i < MAX_IMAGE_NODES; i++) {
			*(char **)p = cache->first_avalible;
			cache->first_avalible = p;
			p += sizeof(struct image_node);
		}

		RTE_INIT_LIST(&cache->lru);
		memset(cache->slots, 0, sizeof(cache->slots));
	}
}

static void lazy_cache_add(struct lazy_cache *cache, struct image_node *img, uint32_t key,
						   const void *data) {
	cache->cache_dirty = 1;
	img->key = key;
	img->data = data;
	rte_list_add_tail(&img->lru, &cache->lru);
	rte_hlist_add_head(&img->node, &cache->slots[KEY_HASH(key)]);
}

static void *lazy_cache_get(struct lazy_cache *cache, uint32_t key) {
	struct rte_hnode *pos;
	uint32_t offset;

	if (!cache->cache_dirty)
		return NULL;

	offset = KEY_HASH(key);

	rte_hlist_foreach(pos, &cache->slots[offset]) {
		struct image_node *img = rte_container_of(pos, struct image_node, node);
		if (img->key == key) {

			/* 
			 * Move node to the head of the list when the cache is hit 
			 */
			rte_list_del(&img->lru);
			rte_list_add(&img->lru, &cache->lru);
			cache->cache_hits++;

			return (void *)img->data;
		}
	}

	cache->cache_misses++;
	return NULL;
}

static void cache_internal_init(struct lazy_cache *cache, void *area, size_t size,
								void (*release)(void *p)) {
	cache->policy          = LAZY_CACHE_NONE;
	cache->area            = area;
	cache->start           = ALIGNED_UP_ADD(area, 0, CACHE_ALIGNED);
	cache->end             = (char *)area + size;
	cache->freeptr         = cache->start;
	cache->cache_limited   = CACHE_LIMIT(cache->end - cache->start);
	cache->cache_dirty     = 1;
	cache->invalid_pending = 0;
	cache->cache_resets    = 0;
	cache->release         = release;

#ifdef CONFIG_LVGL_LAZYDECOMP_AUXMEM
	cache->aux_area        = NULL;
	cache->aux_start       = NULL;
	cache->aux_end         = NULL;
	cache->aux_freeptr     = NULL;
#endif

	cache->wait_complete   = NULL;

	lazy_cache_reset(cache);
}

int lazy_cache_decomp(const lv_img_dsc_t **src, lv_img_dsc_t *imgbuf, void *context,
	int devid) {
	const lv_img_dsc_t *deref_src = *src;
	int err = 0;

	/*
	 * If lvgl image descriptor use lazy decompress 
	 */
	if (deref_src->header.reserved == LV_IMG_LAZYDECOMP_MARKER) {
		struct lazy_cache *cache = &cache_controller;
		const struct lazy_decomp *ld = (struct lazy_decomp *)deref_src->data;
		void **phead;
		void *p;

		assert(ld->decompress != NULL);

		switch (cache->policy) {
		/*
		 * Does not use any caching policy
		 */
		case LAZY_CACHE_NONE:
			err = ld->decompress(ld, cache->start, deref_src->data_size);
			if (!err) {
				p = cache->start;
				goto _next;
			}
			goto _exit;

		case LAZY_CACHE_LRU:
			if (!cache->invalid_pending) {
				/*
				 * The first, We search image cache
				 */

				p = lazy_cache_get(cache, ld->offset);
				if (p != NULL)
					goto _next;

				/*
				 * Allocate free memory for decompress new image
				 */
				if (deref_src->data_size < cache->cache_limited) {
					struct image_node *img = NULL;
					struct rte_list *victim;

					/*
					 * Try to allocate new buffer for cache block
					 */
					p = alloc_cache_buffer(cache, deref_src->data_size, &phead);
					if (p != NULL) {
						/*
						 * Allocate a cache node 
						 */
						img = alloc_node(cache);
						if (img != NULL) {

							/*
							 * Record memory size 
							 */
							img->size = deref_src->data_size;
							goto _decomp_nowait;
						}

						/*
						 * If there are no free nodes, release the cache block
						 */
						*phead = p;

						/*
						 * Increase node misses count 
						 */
						cache->node_misses++;
					}

					/*
					 * Find victim node from LRU list
					 */
					rte_list_foreach(victim, &cache->lru) {
						img = rte_container_of(victim, struct image_node, lru);

						if (img->size >= deref_src->data_size) {
							/*
							 * If we found a valid cache block then remove it from list
							 */
							rte_list_del(&img->lru);
							rte_hlist_del(&img->node);
							p = (void *)img->data;
							goto _decomp;
						}
					}

					/*
					 * If no cache blocks are found, reset the cache controller
					 */
					p = cache->start;
					cache->invalid_pending = 1;
					img = NULL;

_decomp:
					/*
					 * Waiting for rendering to complete
					 */
					if (cache->wait_complete)
						cache->wait_complete(context, devid);

_decomp_nowait:
					err = ld->decompress(ld, p, deref_src->data_size);
					if (!err) {
						/*
						 * Add image data to cache
						 */
						if (img != NULL)
							lazy_cache_add(cache, img, ld->offset, p);
						goto _next;
					}
				} else {
					/*
					 * If the image size is large than limited value,
					 * we will not cache it and invalid cache controller
					 */
					cache->invalid_pending = 1;
				}
			} else {
				/*
				 * If cache controller has received invalid request, Reset it
				 */
				lazy_cache_reset(cache);
				cache->invalid_pending = 0;
			}

			/*
			 * Waiting for rendering to complete
			 */
			if (cache->wait_complete)
				cache->wait_complete(context, devid);

			p = cache->start;
			err = ld->decompress(ld, p, deref_src->data_size);
			if (!err)
				goto _next;
			goto _exit;

		default:
			goto _exit;
		}

_next:
		*imgbuf = *deref_src;
		imgbuf->data = p;
		*src = imgbuf;
	}

_exit:
	return err;
}

void lazy_cache_set_gpu_waitcb(void (*waitcb)(void *, int)) {
	cache_controller.wait_complete = waitcb;
}

void lazy_cache_invalid(void) {
	cache_controller.invalid_pending = 1;
}

void lazy_cache_set_policy(int policy) {
	lazy_cache_invalid();
	cache_controller.policy = (uint16_t)policy;
}

int lazy_cache_init(void *area, size_t size, void (*release)(void *p)) {
	struct lazy_cache *cache;

	if (area == NULL || size < 512)
		return -EINVAL;

	cache = &cache_controller;
	if (cache->area != NULL && cache->release != NULL) {
		cache->release(cache->area);
		cache->area = NULL;
	}

	cache_internal_init(cache, area, size, release);

	return 0;
}

int lazy_cache_renew(size_t size, void* (*alloc)(size_t)) {
	struct lazy_cache *cache;
	void *area;

	if (alloc == NULL || size < 512)
		return -EINVAL;

	cache = &cache_controller;
	if (cache->area != NULL && cache->release != NULL) {
		cache->release(cache->area);
		cache->area = NULL;
	}

	area = alloc(size);
	if (area == NULL)
		return -ENOMEM;

	cache_internal_init(cache, area, size, cache->release);

	return 0;
}

#ifdef CONFIG_LVGL_LAZYDECOMP_AUXMEM
int lazy_cache_aux_mempool_init(void *area, size_t size, void (*release)(void *p)) {
	struct lazy_cache *cache;

	if (area == NULL || size < 512)
		return -EINVAL;

	cache = &cache_controller;
	cache->aux_area    = area;
	cache->aux_start   = ALIGNED_UP_ADD(area, 0, CACHE_ALIGNED);
	cache->aux_end     = (char *)area + size;
	cache->aux_freeptr = cache->aux_start;
	cache->aux_release = release;

	return 0;
}

void lazy_cache_aux_mempool_uninit(void) {
	struct lazy_cache *cache = &cache_controller;

	if (cache->aux_area != NULL && cache->aux_release != NULL) {
		cache->aux_release(cache->aux_area);
		cache->aux_area    = NULL;
		cache->aux_start   = NULL;
		cache->aux_end     = NULL;
		cache->aux_freeptr = NULL;

		/*
		 * The cache controller will be reset when aux memory pool
		 * is be free
		 */
		lazy_cache_invalid();
	}
}
#endif /* CONFIG_LVGL_LAZYDECOMP_AUXMEM */

int lazy_cache_get_information(struct lazy_cache_statistics* sta) {
	struct lazy_cache *cache = &cache_controller;

	if (sta == NULL)
		return -EINVAL;

	sta->cache_hits   = cache->cache_hits;
	sta->cache_misses = cache->cache_misses;
	sta->cache_resets = cache->cache_resets;
	sta->node_misses  = cache->node_misses;

	return 0;
}
