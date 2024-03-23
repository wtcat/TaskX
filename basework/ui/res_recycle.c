/*
 * Copyright 2023 wtcat
 */
#include <errno.h>
#include <assert.h>

#include "basework/ui/res_recycle.h"


static void font_resource_release(struct resource_node *rn) {
    lv_font_res_t *font = container_of(rn, lv_font_res_t, rnode.node);
    lvgl_bitmap_font_close(&font->font);
}

void _uires_resource_add(struct list_head *head, struct resource_node *rn, 
    res_release_t release) {
    rn->release = release;
    list_add_tail(&rn->node, head);
}

void _uires_resource_release(struct list_head *head, res_release_t type) {
    struct list_head *pos, *next;
    list_for_each_safe(pos, next, head) {
        struct resource_node *rn = container_of(pos, struct resource_node, node);
        assert(rn->release != NULL);
        list_del(pos);
        if (!type || rn->release == type)
            rn->release(rn);
    }
}

int _uires_lvgl_resource_release(struct list_head *head, int filter) {
    if (!head)
        return -EINVAL;
    switch (filter) {
    case kAnyResource:
        _uires_resource_release(head, NULL);
        break;
    case kPictureResource:
        break;
    case kFontResource:
         _uires_resource_release(head, font_resource_release);
        break;
    case kStringResource:
        break;
    default:
        break;
    }
    return 0;
}

int _uires_lvgl_bitmap_font_open(struct list_head *head, 
    lv_font_res_t *font, const char *font_path) {
    int err;
    if (!font || !font_path)
        return -EINVAL;
    err = lvgl_bitmap_font_open(&font->font, font_path);
    if (!err)
        _uires_resource_add(head, &font->rnode, font_resource_release);
    return err;
}
