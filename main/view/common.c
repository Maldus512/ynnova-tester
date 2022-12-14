#include "view.h"
#include "style/style.h"


void view_common_set_hidden(lv_obj_t *obj, int hidden) {
    if (((obj->flags & LV_OBJ_FLAG_HIDDEN) == 0) && hidden) {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else if (((obj->flags & LV_OBJ_FLAG_HIDDEN) > 0) && !hidden) {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}


lv_obj_t *view_common_icon_button_create(lv_obj_t *root, const lv_img_dsc_t *dsc) {
    lv_obj_t *btn = lv_btn_create(root);
    lv_obj_set_size(btn, 80, 80);

    lv_obj_t *img = lv_img_create(btn);
    lv_obj_add_style(img, (lv_style_t *)&style_icon, LV_STATE_DEFAULT);
    lv_img_set_src(img, dsc);
    lv_obj_center(img);

    return btn;
}


void view_common_set_checked(lv_obj_t *obj, uint8_t checked) {
    if ((lv_obj_get_state(obj) & LV_STATE_CHECKED) > 0 && !checked) {
        lv_obj_clear_state(obj, LV_STATE_CHECKED);
    } else if ((lv_obj_get_state(obj) & LV_STATE_CHECKED) == 0 && checked) {
        lv_obj_add_state(obj, LV_STATE_CHECKED);
    }
}


void view_common_set_disabled(lv_obj_t *obj, uint8_t disabled) {
    if ((lv_obj_get_state(obj) & LV_STATE_DISABLED) > 0 && !disabled) {
        lv_obj_clear_state(obj, LV_STATE_DISABLED);
    } else if ((lv_obj_get_state(obj) & LV_STATE_DISABLED) == 0 && disabled) {
        lv_obj_add_state(obj, LV_STATE_DISABLED);
    }
}