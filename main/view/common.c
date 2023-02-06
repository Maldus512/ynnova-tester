#include "view.h"
#include "style/style.h"
#include "common.h"
#include "cJSON.h"


static void delete_obj_timer(lv_timer_t *timer);


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


lv_obj_t *view_common_toast(const char *msg) {
    lv_obj_t *obj = view_common_toast_with_parent(msg, lv_layer_top());
    return obj;
}


lv_obj_t *view_common_toast_with_parent(const char *msg, lv_obj_t *parent) {
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_width(obj, 400);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(obj, (lv_style_t *)&style_toast, LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(obj);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_label_set_text(lbl, msg);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(lbl, 380);

    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, -40);

    lv_timer_t *timer = lv_timer_create(delete_obj_timer, 5000, obj);
    lv_timer_set_repeat_count(timer, 1);

    return obj;
}


uint8_t view_validate_json_input(lv_obj_t *textarea) {
    const char *query = lv_textarea_get_text(textarea);

    cJSON *json = cJSON_Parse(query);
    if (json != NULL) {
        cJSON *json_part_number = cJSON_GetObjectItem(json, "partNumber");
        if (cJSON_IsString(json_part_number)) {
            lv_textarea_set_text(textarea, cJSON_GetStringValue(json_part_number));
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}


static void delete_obj_timer(lv_timer_t *timer) {
    lv_obj_del(timer->user_data);
}