#include "lv_page_manager.h"
#include "model/model.h"
#include "view/view.h"
#include "view/style/style.h"
#include "view/common.h"
#include "log.h"


#define LOG_PANEL_HEIGHT      116
#define PAGE_LOG(format, ...) log_info(format, ##__VA_ARGS__)


LV_IMG_DECLARE(img_icon_connection);
LV_IMG_DECLARE(img_icon_settings);
LV_IMG_DECLARE(img_icon_play);
LV_IMG_DECLARE(img_icon_pause);
LV_IMG_DECLARE(img_icon_reset);
LV_IMG_DECLARE(img_icon_download);


enum {
    CONNECTION_BTN_ID,
    SETTINGS_BTN_ID,
    PLAY_PAUSE_BTN_ID,
    RESET_BTN_ID,
    DOWNLOAD_BTN_ID,
};


typedef struct {
    lv_obj_t *obj;
    lv_obj_t *result_cb;
    lv_obj_t *spinner;
} test_widget_t;


typedef enum {
    PAGE_STATE_STARTED,
    PAGE_STATE_RUNNING,
    PAGE_STATE_PAUSED,
} page_state_t;


struct page_data {
    lv_obj_t *btn_connection;
    lv_obj_t *btn_play_pause;
    lv_obj_t *btn_reset;
    lv_obj_t *btn_download;

    lv_obj_t *test_interface;
    lv_obj_t *board_missing_msg;
    lv_obj_t *board_open_msg;

    test_widget_t test_widgets[MAX_TEST_SUITE_LENGTH];

    uint8_t      pending_request;
    test_state_t previous_test_state;
    uint16_t     previous_test_code;
    page_state_t state;
};


static void                     update_page(model_t *pmodel, struct page_data *pdata);
static test_widget_t            test_widget_create(lv_obj_t *root, size_t num, uint16_t code);
static lv_pman_controller_msg_t next_test(model_t *pmodel, struct page_data *pdata);


static void *create_page(void *args, void *extra) {
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);

    pdata->pending_request = 0;

    return pdata;
}


static void open_page(void *args, void *data) {
    lv_obj_t         *btn, *obj, *lbl;
    struct page_data *pdata  = data;
    model_t          *pmodel = args;

    pdata->state = PAGE_STATE_PAUSED;

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
    lv_obj_set_style_pad_top(right_panel, 2, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(right_panel, 2, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_gap(right_panel, 8, LV_STATE_DEFAULT);

    btn = view_common_icon_button_create(right_panel, &img_icon_connection);
    lv_pman_register_obj_id(btn, CONNECTION_BTN_ID);
    pdata->btn_connection = btn;

    btn = view_common_icon_button_create(right_panel, &img_icon_settings);
    lv_pman_register_obj_id(btn, SETTINGS_BTN_ID);

    btn = view_common_icon_button_create(right_panel, &img_icon_play);
    lv_pman_register_obj_id(btn, PLAY_PAUSE_BTN_ID);
    pdata->btn_play_pause = btn;

    btn = view_common_icon_button_create(right_panel, &img_icon_reset);
    lv_pman_register_obj_id(btn, RESET_BTN_ID);
    pdata->btn_reset = btn;

    btn = view_common_icon_button_create(right_panel, &img_icon_download);
    lv_pman_register_obj_id(btn, DOWNLOAD_BTN_ID);
    pdata->btn_download = btn;


    lv_obj_t *left_panel = lv_obj_create(cont);
    lv_obj_add_style(left_panel, (lv_style_t *)&style_panel, LV_STATE_DEFAULT);
    lv_obj_set_size(left_panel, LV_HOR_RES - 132, LV_PCT(100));
    lv_obj_align(left_panel, LV_ALIGN_TOP_LEFT, 0, 0);


    obj = lv_obj_create(left_panel);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(obj, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);
    lv_obj_add_style(obj, (lv_style_t *)&style_scrollbar, LV_PART_SCROLLBAR);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    for (size_t i = 0; i < model_get_num_tests_in_current_unit(pmodel); i++) {
        pdata->test_widgets[i] = test_widget_create(obj, i, model_get_test_code(pmodel, i));
    }

    pdata->test_interface = obj;


    obj = lv_obj_create(left_panel);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(obj, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);

    lbl = lv_label_create(obj);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_set_width(lbl, LV_PCT(70));
    lv_obj_center(lbl);
    lv_label_set_text(lbl, "Inserire la scheda nel sistema");

    pdata->board_missing_msg = obj;


    obj = lv_obj_create(left_panel);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(obj, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);

    lbl = lv_label_create(obj);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_set_width(lbl, LV_PCT(70));
    lv_obj_center(lbl);
    lv_label_set_text(lbl, "Chiudere il letto ad aghi");

    pdata->board_open_msg = obj;

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
                        case PLAY_PAUSE_BTN_ID:
                            switch (pdata->state) {
                                case PAGE_STATE_PAUSED:
                                    pdata->state = PAGE_STATE_STARTED;

                                    if (model_is_test_sequence_done(pmodel)) {
                                        model_reset_test_sequence(pmodel);
                                    }

                                    msg.cmsg = next_test(pmodel, pdata);
                                    break;

                                default:
                                    pdata->state = PAGE_STATE_PAUSED;
                                    break;
                            }

                            update_page(pmodel, pdata);
                            break;

                        case RESET_BTN_ID:
                            model_reset_test_sequence(pmodel);
                            msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_RESET_TEST;
                            update_page(pmodel, pdata);
                            break;

                        case CONNECTION_BTN_ID:
                            msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_RESTART_COMMUNICATION;
                            break;

                        case SETTINGS_BTN_ID:
                            msg.vmsg.tag  = LV_PMAN_VIEW_MSG_TAG_CHANGE_PAGE;
                            msg.vmsg.page = &page_settings;
                            break;

                        case DOWNLOAD_BTN_ID:
                            msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_DOWNLOAD;
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
                    if (pdata->previous_test_state != model_get_test_state(pmodel) ||
                        pdata->previous_test_code != model_get_last_test(pmodel)) {

                        pdata->pending_request = 0;

                        pdata->previous_test_state = model_get_test_state(pmodel);
                        pdata->previous_test_code  = model_get_last_test(pmodel);
                    }

                    if (pdata->state != PAGE_STATE_PAUSED) {
                        if (model_get_test_state(pmodel) == TEST_STATE_DONE && !pdata->pending_request) {
                            if (model_get_test_result(pmodel) != TEST_RESULT_OK) {
                                pdata->state = PAGE_STATE_PAUSED;
                            } else {
                                if (model_next_test(pmodel) == 0) {
                                    msg.cmsg = next_test(pmodel, pdata);
                                } else {
                                    pdata->state = PAGE_STATE_PAUSED;
                                }
                            }

                            lv_obj_scroll_to_view(pdata->test_widgets[model_get_test_index(pmodel)].obj, LV_ANIM_ON);
                        }
                    }

                    update_page(pmodel, pdata);
                    break;
            }
            break;
        }
    }

    return msg;
}


static lv_pman_controller_msg_t next_test(model_t *pmodel, struct page_data *pdata) {
    lv_pman_controller_msg_t msg = {.tag = LV_PMAN_CONTROLLER_MSG_TAG_NONE};

    msg.tag                = LV_PMAN_CONTROLLER_MSG_TAG_START_TEST;
    msg.test               = model_get_current_test_code(pmodel);
    pdata->pending_request = 1;

    PAGE_LOG("Inizio test numero %i, codice %i", model_get_test_index(pmodel), msg.test);

    return msg;
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

    lv_obj_t *spinner = lv_spinner_create(cont, 2000, 60);
    lv_obj_set_style_arc_color(spinner, STYLE_FG_COLOR, LV_PART_INDICATOR);
    lv_obj_set_size(spinner, 56, 56);
    lv_obj_align_to(spinner, cb, LV_ALIGN_OUT_LEFT_MID, -32, 0);

    return (test_widget_t){.obj = cont, .result_cb = cb, .spinner = spinner};
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
    lv_obj_set_style_img_recolor(lv_obj_get_child(pdata->btn_connection, 0),
                                 model_get_communication_error(pmodel) ? STYLE_RED : STYLE_FG_COLOR, LV_STATE_DEFAULT);

    view_common_set_disabled(pdata->btn_reset, pdata->state != PAGE_STATE_PAUSED || model_get_downloading(pmodel));
    view_common_set_disabled(pdata->btn_download, pdata->state != PAGE_STATE_PAUSED || model_get_downloading(pmodel));

    switch (model_get_board_state(pmodel)) {
        case BOARD_STATE_ABSENT:
            view_common_set_hidden(pdata->test_interface, 1);
            view_common_set_hidden(pdata->board_missing_msg, 0);
            view_common_set_hidden(pdata->board_open_msg, 1);
            break;

        case BOARD_STATE_PRESENT_OPEN:
            view_common_set_hidden(pdata->test_interface, 1);
            view_common_set_hidden(pdata->board_missing_msg, 1);
            view_common_set_hidden(pdata->board_open_msg, 0);
            break;

        case BOARD_STATE_READY:
            view_common_set_hidden(pdata->test_interface, 0);
            view_common_set_hidden(pdata->board_missing_msg, 1);
            view_common_set_hidden(pdata->board_open_msg, 1);
            break;
    }

    for (size_t i = 0; i < model_get_num_tests_in_current_unit(pmodel); i++) {
        if (model_get_test_done_history(pmodel, i)) {
            view_common_set_checked(pdata->test_widgets[i].result_cb, 1);

            switch (model_get_test_result_history(pmodel, i)) {
                case TEST_RESULT_OK:
                    lv_obj_set_style_bg_img_src(pdata->test_widgets[i].result_cb, LV_SYMBOL_OK,
                                                LV_STATE_CHECKED | LV_PART_INDICATOR);
                    break;

                default:
                    lv_obj_set_style_bg_img_src(pdata->test_widgets[i].result_cb, LV_SYMBOL_CLOSE,
                                                LV_STATE_CHECKED | LV_PART_INDICATOR);
                    break;
            }
        } else {
            view_common_set_checked(pdata->test_widgets[i].result_cb, 0);
        }

        if (model_get_test_index(pmodel) == i) {
            lv_obj_add_style(pdata->test_widgets[i].obj, (lv_style_t *)&style_selected, LV_STATE_DEFAULT);
        } else {
            lv_obj_remove_style(pdata->test_widgets[i].obj, (lv_style_t *)&style_selected, LV_STATE_DEFAULT);
        }

        if (model_get_test_index(pmodel) == i) {
            switch (model_get_test_state(pmodel)) {
                case TEST_STATE_STARTING:
                case TEST_STATE_IN_PROGRESS:
                    view_common_set_disabled(pdata->btn_play_pause, 1);
                    view_common_set_hidden(pdata->test_widgets[i].spinner, 0);
                    break;

                default:
                    view_common_set_disabled(pdata->btn_play_pause,
                                             model_get_communication_error(pmodel) || model_get_downloading(pmodel));
                    view_common_set_hidden(pdata->test_widgets[i].spinner, 1);
                    break;
            }
        } else {
            view_common_set_hidden(pdata->test_widgets[i].spinner, 1);
        }
    }

    lv_img_set_src(lv_obj_get_child(pdata->btn_play_pause, 0),
                   pdata->state == PAGE_STATE_PAUSED ? &img_icon_play : &img_icon_pause);
}


const lv_pman_page_t page_main = {
    .create        = create_page,
    .destroy       = lv_pman_destroy_all,
    .open          = open_page,
    .close         = lv_pman_close_all,
    .process_event = process_page_event,
};