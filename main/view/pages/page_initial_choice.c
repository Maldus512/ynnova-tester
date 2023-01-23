#define _GNU_SOURCE
#include <string.h>
#include "lv_page_manager.h"
#include "model/model.h"
#include "view/view.h"
#include "view/style/style.h"
#include "view/common.h"
#include "log.h"


enum {
    SELECT_TEST_UNIT_BTN_ID,
    TEXTAREA_ID,
    KEYBOARD_ID,
};

typedef struct {
    lv_obj_t *obj;
} test_unit_widget_t;


struct page_data {
    test_unit_widget_t test_unit_widgets[MAX_NUM_TEST_UNITS];
    lv_obj_t          *list_test_units;
    lv_pman_handle_t   handle;

    lv_obj_t *textarea;
    lv_obj_t *keyboard;
    lv_obj_t *lbl_msg;
};


static void               update_page(model_t *pmodel, struct page_data *pdata);
static test_unit_widget_t test_unit_widget_create(lv_obj_t *root, const char *name);
static void               test_units_list_update(model_t *pmodel, struct page_data *pdata);
static uint8_t            perfect_fit(model_t *pmodel, struct page_data *pdata, size_t *choice);


static void *create_page(void *args, void *extra) {
    (void)args;
    (void)extra;
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);

    for (size_t i = 0; i < MAX_NUM_TEST_UNITS; i++) {
        pdata->test_unit_widgets[i].obj = NULL;
    }

    return pdata;
}


static void open_page(lv_pman_handle_t handle, void *args, void *data) {
    lv_obj_t         *obj;
    struct page_data *pdata  = data;
    model_t          *pmodel = args;

    pdata->handle = handle;

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(cont, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cont, 4, LV_STATE_DEFAULT);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *textarea = lv_textarea_create(cont);
    lv_obj_set_size(textarea, LV_HOR_RES - 16, 64);
    lv_obj_align(textarea, LV_ALIGN_TOP_LEFT, 4, 8);
    lv_textarea_set_text(textarea, "");
    lv_textarea_set_max_length(textarea, TEST_UNIT_NAME_LENGTH - 1);
    lv_textarea_set_one_line(textarea, 1);
    lv_pman_register_obj_id(handle, textarea, TEXTAREA_ID);
    lv_obj_add_flag(textarea, LV_OBJ_FLAG_CLICKABLE);
    lv_group_focus_obj(textarea);
    pdata->textarea = textarea;

    lv_obj_t *center_panel = lv_obj_create(cont);
    lv_obj_add_style(center_panel, (lv_style_t *)&style_panel, LV_STATE_DEFAULT);
    lv_obj_set_size(center_panel, LV_HOR_RES - 16, LV_VER_RES - 64 - 64 - 24);
    lv_obj_align(center_panel, LV_ALIGN_BOTTOM_MID, 0, -68);

    obj = lv_obj_create(center_panel);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(obj, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    pdata->list_test_units = obj;

    test_units_list_update(pmodel, pdata);

    lv_obj_t *bottom_panel = lv_obj_create(cont);
    lv_obj_add_style(bottom_panel, (lv_style_t *)&style_panel, LV_STATE_DEFAULT);
    lv_obj_set_size(bottom_panel, LV_HOR_RES - 16, 80);
    lv_obj_align(bottom_panel, LV_ALIGN_BOTTOM_MID, 0, 16);

    lv_obj_t *lbl = lv_label_create(bottom_panel);
    lv_label_set_text(lbl, "Inserire o scannerizzare il codice del prodotto");
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *keyboard = lv_keyboard_create(cont);
    lv_obj_set_size(keyboard, LV_HOR_RES, LV_VER_RES - 64 - 16);
    lv_keyboard_set_textarea(keyboard, textarea);
    lv_pman_register_obj_id(handle, keyboard, KEYBOARD_ID);
    view_common_set_hidden(keyboard, 1);
    pdata->keyboard = keyboard;

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
                        case TEXTAREA_ID:
                            view_common_set_hidden(pdata->keyboard, 0);
                            break;

                        case SELECT_TEST_UNIT_BTN_ID:
                            model_set_test_unit_index(pmodel, event.lvgl.number);
                            msg.vmsg.tag  = LV_PMAN_VIEW_MSG_TAG_REBASE;
                            msg.vmsg.page = &page_main;
                            break;
                    }
                    break;
                }

                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.lvgl.id) {
                        case TEXTAREA_ID: {
                            update_page(pmodel, pdata);
                            break;
                        }
                    }
                    break;
                }

                case LV_EVENT_CANCEL: {
                    switch (event.lvgl.id) {
                        case KEYBOARD_ID:
                            view_common_set_hidden(pdata->keyboard, 1);
                            break;
                    }
                    break;
                }

                case LV_EVENT_READY: {
                    switch (event.lvgl.id) {
                        case TEXTAREA_ID: {
                            size_t choice = 0;
                            if (perfect_fit(pmodel, pdata, &choice)) {
                                model_set_test_unit_index(pmodel, event.lvgl.number);
                                msg.vmsg.tag  = LV_PMAN_VIEW_MSG_TAG_REBASE;
                                msg.vmsg.page = &page_main;
                            }
                            break;
                        }

                        case KEYBOARD_ID:
                            view_common_set_hidden(pdata->keyboard, 1);
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
    const char *query = lv_textarea_get_text(pdata->textarea);
    uint8_t     any   = 0;

    for (size_t i = 0; i < model_get_num_test_units(pmodel); i++) {
        if (pdata->test_unit_widgets[i].obj != NULL) {
            if (strcasestr(model_get_test_unit_name(pmodel, i), query) != NULL) {
                view_common_set_hidden(pdata->test_unit_widgets[i].obj, 0);
                any = 1;
            } else {
                view_common_set_hidden(pdata->test_unit_widgets[i].obj, 1);
            }
        }
    }

    view_common_set_hidden(pdata->lbl_msg, any);
}


static uint8_t perfect_fit(model_t *pmodel, struct page_data *pdata, size_t *choice) {
    const char *query = lv_textarea_get_text(pdata->textarea);

    for (size_t i = 0; i < model_get_num_test_units(pmodel); i++) {
        if (pdata->test_unit_widgets[i].obj != NULL) {
            if (strcasecmp(model_get_test_unit_name(pmodel, i), query) == 0) {
                *choice = i;
                return 1;
            }
        }
    }

    return 0;
}


static test_unit_widget_t test_unit_widget_create(lv_obj_t *root, const char *name) {
    lv_obj_t *cont = lv_btn_create(root);
    lv_obj_set_size(cont, LV_PCT(95), 96);
    lv_obj_add_style(cont, (lv_style_t *)&style_unselected, LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(cont);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text(lbl, name);

    return (test_unit_widget_t){.obj = cont};
}


static void test_units_list_update(model_t *pmodel, struct page_data *pdata) {
    lv_obj_clean(pdata->list_test_units);

    for (size_t i = 0; i < model_get_num_test_units(pmodel); i++) {
        pdata->test_unit_widgets[i] =
            test_unit_widget_create(pdata->list_test_units, model_get_test_unit_name(pmodel, i));
        lv_pman_register_obj_id_and_number(pdata->handle, pdata->test_unit_widgets[i].obj, SELECT_TEST_UNIT_BTN_ID, i);
    }

    lv_obj_t *lbl = lv_label_create(pdata->list_test_units);
    lv_obj_add_flag(lbl, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_set_width(lbl, LV_PCT(70));
    lv_label_set_text(lbl, "Nessuna delle unita' di test corrisponde al codice prodotto inserito");
    lv_obj_center(lbl);

    pdata->lbl_msg = lbl;
}


const lv_pman_page_t page_initial_choice = {
    .create        = create_page,
    .destroy       = lv_pman_destroy_all,
    .open          = open_page,
    .close         = lv_pman_close_all,
    .process_event = process_page_event,
};