#include "lv_page_manager.h"
#include "model/model.h"
#include "view/view.h"
#include "view/style/style.h"
#include "view/common.h"
#include "log.h"


LV_IMG_DECLARE(img_icon_home);


enum {
    HOME_BTN_ID,
    SELECT_TEST_UNIT_BTN_ID,
    ADD_TEST_UNIT_BTN_ID,
    REMOVE_TEST_UNIT_BTN_ID,
    EDIT_TEST_UNIT_BTN_ID,
};

typedef struct {
    lv_obj_t *obj;
    lv_obj_t *edit_btn;
    lv_obj_t *delete_btn;
} test_unit_widget_t;


struct page_data {
    test_unit_widget_t test_unit_widgets[MAX_NUM_TEST_UNITS];
    lv_obj_t          *list_test_units;
    lv_obj_t          *btn_add;
};


static void               update_page(model_t *pmodel, struct page_data *pdata);
static test_unit_widget_t test_unit_widget_create(lv_obj_t *root, const char *name);
static void               test_units_list_update(model_t *pmodel, struct page_data *pdata);


static void *create_page(void *args, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);

    for (size_t i = 0; i < MAX_NUM_TEST_UNITS; i++) {
        pdata->test_unit_widgets[i].obj        = NULL;
        pdata->test_unit_widgets[i].edit_btn   = NULL;
        pdata->test_unit_widgets[i].delete_btn = NULL;
    }

    return pdata;
}


static void open_page(void *args, void *data) {
    lv_obj_t         *btn, *obj, *lbl;
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


    obj = lv_obj_create(left_panel);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(obj, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    pdata->list_test_units = obj;

    test_units_list_update(pmodel, pdata);

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
                            if (model_get_to_save(pmodel)) {
                                model_set_to_save(pmodel, 0);
                                msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_SAVE;
                            }
                            break;

                        case EDIT_TEST_UNIT_BTN_ID:
                            msg.vmsg.tag   = LV_PMAN_VIEW_MSG_TAG_CHANGE_PAGE_EXTRA;
                            msg.vmsg.extra = (void *)(uintptr_t)event.lvgl.number;
                            msg.vmsg.page  = &page_test_unit;
                            break;

                        case REMOVE_TEST_UNIT_BTN_ID:
                            model_set_to_save(pmodel, 1);
                            model_remove_test_unit(pmodel, event.lvgl.number);

                            test_units_list_update(pmodel, pdata);
                            update_page(pmodel, pdata);
                            break;

                        case SELECT_TEST_UNIT_BTN_ID:
                            model_set_test_unit_index(pmodel, event.lvgl.number);

                            update_page(pmodel, pdata);
                            break;

                        case ADD_TEST_UNIT_BTN_ID:
                            model_set_to_save(pmodel, 1);
                            model_add_default_test_unit(pmodel);

                            test_units_list_update(pmodel, pdata);
                            lv_obj_scroll_to_view(pdata->btn_add, LV_ANIM_OFF);
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
    for (size_t i = 0; i < model_get_num_test_units(pmodel); i++) {
        if (pdata->test_unit_widgets[i].obj != NULL) {
            if (model_get_test_unit_index(pmodel) == i) {
                lv_obj_add_style(pdata->test_unit_widgets[i].obj, (lv_style_t *)&style_selected, LV_STATE_DEFAULT);
            } else {
                lv_obj_remove_style(pdata->test_unit_widgets[i].obj, (lv_style_t *)&style_selected, LV_STATE_DEFAULT);
            }
        }
    }
}


static test_unit_widget_t test_unit_widget_create(lv_obj_t *root, const char *name) {
    lv_obj_t *cont = lv_btn_create(root);
    lv_obj_set_size(cont, LV_PCT(95), 96);
    lv_obj_add_style(cont, (lv_style_t *)&style_unselected, LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(cont);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text(lbl, name);

    lv_obj_t *del_btn = lv_btn_create(cont);
    lv_obj_set_size(del_btn, 48, 48);
    lbl = lv_label_create(del_btn);
    lv_label_set_text(lbl, LV_SYMBOL_CLOSE);
    lv_obj_center(lbl);
    lv_obj_align(del_btn, LV_ALIGN_RIGHT_MID, -64, 0);

    lv_obj_t *edit_btn = lv_btn_create(cont);
    lv_obj_set_size(edit_btn, 48, 48);
    lbl = lv_label_create(edit_btn);
    lv_label_set_text(lbl, LV_SYMBOL_EDIT);
    lv_obj_center(lbl);
    lv_obj_align(edit_btn, LV_ALIGN_RIGHT_MID, 0, 0);

    return (test_unit_widget_t){.obj = cont, .edit_btn = edit_btn, .delete_btn = del_btn};
}


static void test_units_list_update(model_t *pmodel, struct page_data *pdata) {
    lv_obj_clean(pdata->list_test_units);

    for (size_t i = 0; i < model_get_num_test_units(pmodel); i++) {
        pdata->test_unit_widgets[i] =
            test_unit_widget_create(pdata->list_test_units, model_get_test_unit_name(pmodel, i));
        lv_pman_register_obj_id_and_number(pdata->test_unit_widgets[i].obj, SELECT_TEST_UNIT_BTN_ID, i);
        lv_pman_register_obj_id_and_number(pdata->test_unit_widgets[i].delete_btn, REMOVE_TEST_UNIT_BTN_ID, i);
        lv_pman_register_obj_id_and_number(pdata->test_unit_widgets[i].edit_btn, EDIT_TEST_UNIT_BTN_ID, i);
    }

    view_common_set_disabled(pdata->test_unit_widgets[0].delete_btn, 1);
    view_common_set_disabled(pdata->test_unit_widgets[0].edit_btn, 1);

    lv_obj_t *cont = lv_btn_create(pdata->list_test_units);
    lv_obj_set_size(cont, LV_PCT(95), 96);
    lv_obj_add_style(cont, (lv_style_t *)&style_unselected, LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(cont);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text(lbl, "Aggiungi");

    lbl = lv_label_create(cont);
    lv_label_set_text(lbl, LV_SYMBOL_PLUS);
    lv_obj_align(lbl, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_pman_register_obj_id(cont, ADD_TEST_UNIT_BTN_ID);
    pdata->btn_add = cont;

    if (model_get_num_test_units(pmodel) >= MAX_NUM_TEST_UNITS) {
        view_common_set_disabled(pdata->btn_add, 1);
    }
}


const lv_pman_page_t page_settings = {
    .create        = create_page,
    .destroy       = lv_pman_destroy_all,
    .open          = open_page,
    .close         = lv_pman_close_all,
    .process_event = process_page_event,
};