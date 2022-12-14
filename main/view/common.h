#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED


#include "lvgl.h"


void      view_common_set_hidden(lv_obj_t *obj, int hidden);
void      view_common_set_checked(lv_obj_t *obj, uint8_t checked);
void      view_common_set_disabled(lv_obj_t *obj, uint8_t disabled);
lv_obj_t *view_common_icon_button_create(lv_obj_t *root, const lv_img_dsc_t *dsc);


#endif