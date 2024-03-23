/*
 * Copyright 2023 wtcat
 */
#ifndef BASEWORK_UI_RES_RECYCLE_H_
#define BASEWORK_UI_RES_RECYCLE_H_

#include <lvgl/lvgl_bitmap_font.h>
#include <lvgl/lvgl_res_loader.h>

#include "basework/container/list.h"

#ifdef __cplusplus
extern "C"{
#endif

struct resource_node;
typedef void (*res_release_t)(struct resource_node *rn);

enum uires_type {
    kAnyResource,
    kPictureResource,
    kFontResource,
    kStringResource
};

struct resource_node {
    struct list_head node;
    res_release_t release;
};

typedef struct lv_font_res {
    lv_font_t font;
    struct resource_node rnode;
} lv_font_res_t;


#define RESOURCE_MANAGER \
    static LIST_HEAD(__res_manager)

#define uires_lvgl_bitmap_font_open(_font, _path) \
    _uires_lvgl_bitmap_font_open(&__res_manager, &(_font)->font, _path)

#define uires_release(_type) \
    _uires_lvgl_resource_release(&__res_manager, _type)


void _uires_resource_add(struct list_head *head, struct resource_node *rn, 
    res_release_t release);
void _uires_resource_release(struct list_head *head, res_release_t type);
int _uires_lvgl_resource_release(struct list_head *head, int filter);
int _uires_lvgl_bitmap_font_open(struct list_head *head, 
    lv_font_res_t *font, const char *font_path);

#ifdef __cplusplus
}
#endif
#endif /* BASEWORK_UI_RES_RECYCLE_H_ */
