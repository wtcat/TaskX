/*
 * Copyright (c) 2020 Actions Technology Co., Ltd
 * Copyright (c) 2023 wtcat
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file img_label.h
 *
 * use the charactor images to draw the label 
 *
 */

#ifndef BASEWORK_UI_WIDGET_IMG_LABEL_H_
#define BASEWORK_UI_WIDGET_IMG_LABEL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>
#include "basework/ui/widget/imglabel_ldops.h"


/*********************
 *      DEFINES
 *********************/

/** maximum images in the image label object */
#define IMG_LABEL_MAX_COUNT    15
#define IMG_LABEL_INVALID_IDX  255

/**********************
 *      TYPEDEFS
 **********************/

/** Data of image area */
typedef struct {
	lv_obj_t obj;

	uint8_t indices[IMG_LABEL_MAX_COUNT]; /* indices of the charactors to show */
	uint8_t count; /* number of indices */

	const lv_img_dsc_t *src_chars; /* image src array */
	uint8_t num_chars;

	imglabel_loader_t loader; /* image resource operations */
	void (*refresh)(lv_obj_t * obj);

	uint8_t same_height : 1; /* current draw charactors have the same height */
	lv_align_t align; /* how to align the charactors inside the object area */
	lv_area_t area;   /* the relative area for the charactors to show, decided by align and the which images to show */
} lvgl_imglabel_t;

extern const lv_obj_class_t lvgl_imglabel_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an image label object
 *
 * @param parent pointer to an object, it will be the parent of the new object
 * @return pointer to the created image label object
 */
lv_obj_t * lvgl_imglabel_create(lv_obj_t * parent);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set align of the image label
 *
 * @param obj pointer to an image label object
 * @param align `LV_ALIGN_...`, align the digit number image inside the object
 */
void lvgl_imglabel_set_align(lv_obj_t *obj, lv_align_t align);

/**
 * Set the image charactor image array
 *
 * @param obj pointer to an image label object
 * @param chars charactor src array, every src should all be opaque or not
 * @param cnt number of image srcs
 */
void lvgl_imglabel_set_src(lv_obj_t *obj, const lv_img_dsc_t * chars, uint8_t cnt);

/**
 * Set the text charactor indices array
 *
 * @param obj pointer to an image label object
 * @param indices the indices of charactors which compose the text
 * @param cnt number of image indices
 */
void lvgl_imglabel_set_text(lv_obj_t *obj, const uint8_t * indices, uint8_t cnt);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the number of charactors
 *
 * @param obj pointer to an image label object
 * @return the number of charactors
 */
uint32_t lvgl_imglabel_get_size(lv_obj_t *obj);

/*=====================
 * Other functions
 *====================*/

/**
 * Split the src into several small charactor srcs
 *
 * @param obj pointer to an image label object, can be NULL
 * @param chars charactor image array
 * @param src the src contains all the charactors
 * @param cnt number of charactors in the src 
 * @param vertical whether the charactors are arranged vertically in the src
 */
void lvgl_imglabel_split_src(lv_obj_t *obj, lv_img_dsc_t * chars, const lv_img_dsc_t * src, uint8_t cnt, bool vertical);

/* User extension */
void lvgl_imglabel_set_src_nocache(lv_obj_t *obj, const lv_img_dsc_t * chars, uint8_t cnt,
	const imglabel_loader_t *ops);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BASEWORK_UI_WIDGET_IMG_LABEL_H_ */
