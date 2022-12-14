#include "lv_page_manager.h"
#include "model/model.h"
#include "view/view.h"
#include "view/style/style.h"
#include "view/common.h"
#include "log.h"


#define LOG_PANEL_HEIGHT 116


LV_IMG_DECLARE(img_icon_home);


enum {
    HOME_BTN_ID,
};


struct page_data {};


static void update_page(model_t *pmodel, struct page_data *pdata);


static void *create_page(lv_pman_page_handle_t page, void *args, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);

    return pdata;
}


static void open_page(lv_pman_page_handle_t page, void *args, void *data) {
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
    lv_pman_register_obj_id(page, btn, HOME_BTN_ID);

    lv_obj_t *left_panel = lv_obj_create(cont);
    lv_obj_add_style(left_panel, (lv_style_t *)&style_panel, LV_STATE_DEFAULT);
    lv_obj_set_size(left_panel, LV_HOR_RES - 132, LV_PCT(100));
    lv_obj_align(left_panel, LV_ALIGN_TOP_LEFT, 0, 0);


    update_page(pmodel, pdata);
}


static lv_pman_msg_t process_page_event(lv_pman_page_handle_t page, void *args, void *data, lv_pman_event_t event) {
    lv_pman_msg_t     msg    = {0};
    model_t          *pmodel = args;
    struct page_data *pdata  = data;

    if (!lv_pman_is_page_open(page)) {
        // Do not handle events if not open
        return msg;
    }

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


static void update_page(model_t *pmodel, struct page_data *pdata) {}


const lv_pman_page_t page_test_sequence = {
    .create        = create_page,
    .destroy       = lv_pman_destroy_all,
    .open          = open_page,
    .close         = lv_pman_close_all,
    .process_event = process_page_event,
};