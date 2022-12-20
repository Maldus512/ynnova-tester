#include "lv_page_manager.h"
#include "model/model.h"
#include "view/view.h"
#include "view/style/style.h"
#include "view/common.h"
#include "log.h"


LV_IMG_DECLARE(img_icon_home);


enum {
    HOME_BTN_ID,
    TOGGLE_TEST_BTN_ID,
};

typedef struct {
    lv_obj_t *obj;
    lv_obj_t *selection_cb;
} test_widget_t;

struct page_data {
    size_t        test_unit_index;
    test_widget_t test_widgets[MAX_TEST_SUITE_LENGTH];
};


static void          update_page(model_t *pmodel, struct page_data *pdata);
static test_widget_t test_widget_create(lv_obj_t *root, size_t num, uint16_t code);


static void *create_page(void *args, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);

    pdata->test_unit_index = (size_t)(uintptr_t)extra;

    return pdata;
}


static void open_page(void *args, void *data) {
    lv_obj_t         *btn;
    struct page_data *pdata  = data;
    model_t          *pmodel = args;

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(cont, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cont, 4, LV_STATE_DEFAULT);

    lv_obj_t *right_panel = lv_obj_create(cont);
    lv_obj_add_style(right_panel, (lv_style_t *)&style_panel, LV_STATE_DEFAULT);
    lv_obj_set_size(right_panel, 100, LV_PCT(100));
    lv_obj_align(right_panel, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_flex_flow(right_panel, LV_FLEX_FLOW_COLUMN_REVERSE);
    lv_obj_set_flex_align(right_panel, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    btn = view_common_icon_button_create(right_panel, &img_icon_home);
    lv_pman_register_obj_id(btn, HOME_BTN_ID);

    lv_obj_t *left_panel = lv_obj_create(cont);
    lv_obj_add_style(left_panel, (lv_style_t *)&style_panel, LV_STATE_DEFAULT);
    lv_obj_set_size(left_panel, LV_HOR_RES - 132, LV_PCT(100));
    lv_obj_align(left_panel, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *obj = lv_obj_create(left_panel);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(obj, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    size_t count = 0;
    for (size_t i = 0; i < sizeof(test_codes) / sizeof(test_codes[0]); i++) {
        if (!model_is_test_required(test_codes[i])) {
            pdata->test_widgets[count] = test_widget_create(obj, i, test_codes[i]);
            lv_pman_register_obj_id_and_number(pdata->test_widgets[count].obj, TOGGLE_TEST_BTN_ID, i);
            count++;
        }
    }

    update_page(pmodel, pdata);
}


static lv_pman_msg_t process_page_event(void *args, void *data, lv_pman_event_t event) {
    lv_pman_msg_t     msg    = {0};
    model_t          *pmodel = args;
    struct page_data *pdata  = data;

    switch (event.tag) {
        case LV_PMAN_EVENT_TAG_OPEN:
            break;

        case LV_PMAN_EVENT_TAG_LVGL: {
            switch (event.lvgl.event) {
                case LV_EVENT_CLICKED: {
                    switch (event.lvgl.id) {
                        case HOME_BTN_ID:
                            msg.vmsg.tag = LV_PMAN_VIEW_MSG_TAG_BACK;
                            break;

                        case TOGGLE_TEST_BTN_ID:
                            model_set_to_save(pmodel, 1);
                            model_toggle_test_configured(pmodel, pdata->test_unit_index, test_codes[event.lvgl.number]);
                            update_page(pmodel, pdata);
                            break;
                    }
                    break;
                }

                default:
                    break;
            }
            break;
        }

        case LV_PMAN_EVENT_TAG_USER: {
            switch (event.user_event.tag) {
                case LV_PMAN_USER_EVENT_TAG_UPDATE:
                    break;
            }
            break;
        }
    }

    return msg;
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
    size_t count = 0;

    for (size_t i = 0; i < sizeof(test_codes) / sizeof(test_codes[0]); i++) {
        if (!model_is_test_required(test_codes[i])) {
            view_common_set_checked(pdata->test_widgets[count].selection_cb,
                                    model_is_test_configured(pmodel, pdata->test_unit_index, test_codes[i]));
            count++;
        }
    }
}


static test_widget_t test_widget_create(lv_obj_t *root, size_t num, uint16_t code) {
    lv_obj_t *cont = lv_btn_create(root);
    lv_obj_set_size(cont, LV_PCT(95), 96);
    lv_obj_add_style(cont, (lv_style_t *)&style_unselected, LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(cont);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text_fmt(lbl, "%3zu: %i", num + 1, code);

    lv_obj_t *cb = lv_checkbox_create(cont);
    lv_obj_clear_flag(cb, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_color(cb, STYLE_FG_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(cb, 2, LV_PART_INDICATOR);
    lv_checkbox_set_text(cb, "");
    lv_obj_align(cb, LV_ALIGN_RIGHT_MID, 0, 0);

    return (test_widget_t){.obj = cont, .selection_cb = cb};
}


const lv_pman_page_t page_test_unit = {
    .create        = create_page,
    .destroy       = lv_pman_destroy_all,
    .open          = open_page,
    .close         = lv_pman_close_all,
    .process_event = process_page_event,
};