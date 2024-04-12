/*
 * Copyright 2024 wtcat  
 * 
 * Note: All API has no thread-safe
 */
#ifndef BASEWORK_UI_LVGL_LAZYDECOMP_H_
#define BASEWORK_UI_LVGL_LAZYDECOMP_H_

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Cache replace policy
 */
#define LAZY_CACHE_NONE 0
#define LAZY_CACHE_LRU 1

/* lv_img_dsc_t reserved filed */
#define LV_IMG_LAZYDECOMP_MARKER 0x3

struct lazy_decomp {
	/* 
	 * The offset of file data 
	 */
	uint32_t offset;

	/*
	 * Decompress picture to destination buffer
	 */
	int (*decompress)(const struct lazy_decomp *ld, void *dst, size_t dstsize);
};

struct lazy_cache_statistics {
	uint16_t cache_hits;
	uint16_t cache_misses;
	uint16_t cache_resets;
};

/*
 * lazy_cache_init - Initialize cache cnotroller
 * 
 * @area point to cache memory area
 * @size memory size
 * @release point to memory free function
 * return 0 if success
 */
int lazy_cache_init(void *area, size_t size, void (*release)(void *p));

/*
 * lazy_cache_renew - Reinitialize cache controller with new size
 *
 * @size memory size
 * @alloc point to memory allocate function
 * return 0 if success
 */
int lazy_cache_renew(size_t size, void *(*alloc)(size_t));

	/*
 * lazy_cache_invalid - Clear all cached data
 */
void lazy_cache_invalid(void);

/*
 * lazy_cache_set_policy - Update cache controller replace policy
 * 
 * @policy replace policy
 */
void lazy_cache_set_policy(int policy);

/*
 * lazy_cache_decomp - Decompress picture data with cache policy
 *
 * @src point to image that will be draw
 * @imgbuf point to temporary image buffer
 * return 0 if success
 */
int  lazy_cache_decomp(const lv_img_dsc_t **src, lv_img_dsc_t *imgbuf);


#ifdef CONFIG_LVGL_LAZYDECOMP_AUXMEM
/*
 * lazy_cache_aux_mempool_init - Initialize aux memory pool for cache controller
 *
 * @area point to cache memory area
 * @size memory size
 * @release point to memory free function
 * return 0 if success
 */
int  lazy_cache_aux_mempool_init(void *area, size_t size, void (*release)(void *p));

/*
 * lazy_cache_aux_mempool_uninit - Destroy aux memory pool
 */
void lazy_cache_aux_mempool_uninit(void);

#endif /* CONFIG_LVGL_LAZYDECOMP_AUXMEM */

/*
 *  lazy_cache_get_information - Get cache controller work status
 * 
 * @sta point to status buffer
 * return 0 if success
 */
int lazy_cache_get_information(struct lazy_cache_statistics *sta);

#ifdef __cplusplus
}
#endif
#endif /* BASEWORK_UI_LVGL_LAZYDECOMP_H_ */
