#include "lv_page_manager.h"
#include "model/model.h"
#include "view/view.h"
#include "view/style/style.h"
#include "view/common.h"
#include "log.h"
#include "view/descriptions/test_code.h"
#include "view/descriptions/error.h"


#define TEST_CONT_COLLAPSED_HEIGHT 80
#define TEST_CONT_EXTENDED_HEIGHT  (TEST_CONT_COLLAPSED_HEIGHT + 56)
#define LOG_PANEL_HEIGHT           116
#define PAGE_LOG(format, ...)      log_info(format, ##__VA_ARGS__)


LV_IMG_DECLARE(img_icon_connection);
LV_IMG_DECLARE(img_icon_settings);
LV_IMG_DECLARE(img_icon_play);
LV_IMG_DECLARE(img_icon_reset);
LV_IMG_DECLARE(img_icon_restart);


enum {
    CONNECTION_BTN_ID,
    SETTINGS_BTN_ID,
    PLAY_BTN_ID,
    RESET_BTN_ID,
    DOWNLOAD_BTN_ID,
    TEXTAREA_ID,
    KEYBOARD_ID,
    SKIP_BTN_ID,
};


typedef struct {
    lv_obj_t *obj;
    lv_obj_t *result_cb;
    lv_obj_t *spinner;
    lv_obj_t *lbl;
    lv_obj_t *lbl_err;
    lv_obj_t *btn_err;
} test_widget_t;


struct page_data {
    lv_obj_t *btn_connection;
    lv_obj_t *btn_play;
    lv_obj_t *btn_reset;
    lv_obj_t *lbl_status;

    lv_obj_t *test_interface;
    lv_obj_t *curtain;

    lv_obj_t *textarea;
    lv_obj_t *keyboard;

    test_widget_t test_widgets[MAX_TEST_SUITE_LENGTH];
    test_widget_t download_widget;

    uint8_t imei_submitted;
    char    imei[32];
};


static void          update_page(model_t *pmodel, struct page_data *pdata);
static void          update_test_interface(model_t *pmodel, struct page_data *pdata);
static test_widget_t test_widget_create(lv_obj_t *root, uint16_t code);
static test_widget_t step_widget_create(lv_obj_t *root, const char *text);
static uint8_t       test_is_selected(model_t *pmodel, size_t num);
static void          successful_step(test_widget_t widget);
static void          failed_step(test_widget_t widget);


static void *create_page(void *args, void *extra) {
    (void)args;
    (void)extra;
    struct page_data *pdata = lv_mem_alloc(sizeof(struct page_data));
    assert(pdata != NULL);

    memset(pdata->imei, 0, sizeof(pdata->imei));
    pdata->imei_submitted = 0;

    return pdata;
}


static void open_page(lv_pman_handle_t handle, void *args, void *data) {
    lv_obj_t         *btn, *obj, *lbl;
    struct page_data *pdata  = data;
    model_t          *pmodel = args;

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(cont, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cont, 4, LV_STATE_DEFAULT);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

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
    lv_pman_register_obj_id(handle, btn, CONNECTION_BTN_ID);
    pdata->btn_connection = btn;

    btn = view_common_icon_button_create(right_panel, &img_icon_settings);
    lv_pman_register_obj_id(handle, btn, SETTINGS_BTN_ID);

    btn = view_common_icon_button_create(right_panel, &img_icon_play);
    lv_pman_register_obj_id(handle, btn, PLAY_BTN_ID);
    pdata->btn_play = btn;

    btn = view_common_icon_button_create(right_panel, &img_icon_restart);
    lv_pman_register_obj_id(handle, btn, RESET_BTN_ID);
    pdata->btn_reset = btn;

    lv_obj_t *left_panel = lv_obj_create(cont);
    lv_obj_add_style(left_panel, (lv_style_t *)&style_panel, LV_STATE_DEFAULT);
    lv_obj_set_size(left_panel, MAIN_PANEL_WIDTH, LV_VER_RES - 64);
    lv_obj_clear_flag(left_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(left_panel, LV_ALIGN_TOP_LEFT, 0, 0);

    obj = lv_obj_create(left_panel);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_add_style(obj, (lv_style_t *)&style_transparent_cont, LV_STATE_DEFAULT);
    lv_obj_add_style(obj, (lv_style_t *)&style_scrollbar, LV_PART_SCROLLBAR);

    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    uint8_t programming_step_shown = 0;
    size_t  num_tests              = model_get_num_tests_in_current_unit(pmodel);
    for (size_t i = 0; i < num_tests; i++) {
        uint16_t code = model_get_test_code_from_current_unit(pmodel, i);

        // If we are past the programming step or at the end add the programming step widget
        if (!programming_step_shown && (code >= TEST_PROGRAMMING_THRESHOLD || (i + 1 == num_tests))) {
            pdata->download_widget = step_widget_create(obj, "Programmazione dispositivo");
            lv_pman_register_obj_id(handle, pdata->download_widget.btn_err, SKIP_BTN_ID);
            programming_step_shown = 1;
        }

        pdata->test_widgets[i] = test_widget_create(obj, code);
        lv_pman_register_obj_id(handle, pdata->test_widgets[i].btn_err, SKIP_BTN_ID);
    }

    pdata->test_interface = obj;


    obj = lv_obj_create(left_panel);
    lv_obj_set_size(obj, MAIN_PANEL_WIDTH, LV_VER_RES - 64);
    lv_obj_set_style_bg_color(obj, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, LV_OPA_50, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_STATE_DEFAULT);
    lv_obj_center(obj);

    pdata->curtain = obj;


    lv_obj_t *status_panel = lv_obj_create(cont);
    lv_obj_clear_flag(status_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(status_panel, (lv_style_t *)&style_panel, LV_STATE_DEFAULT);
    lv_obj_set_size(status_panel, MAIN_PANEL_WIDTH + 16, 64);
    lv_obj_align(status_panel, LV_ALIGN_BOTTOM_LEFT, -16, 16);

    lbl = lv_label_create(status_panel);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(lbl, MAIN_PANEL_WIDTH);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 16, -8);
    pdata->lbl_status = lbl;


    lv_obj_t *textarea = lv_textarea_create(left_panel);
    lv_obj_set_size(textarea, LV_PCT(100), 64);
    lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, -8);
    lv_textarea_set_text(textarea, "");
    lv_textarea_set_max_length(textarea, TEST_UNIT_NAME_LENGTH - 1);
    lv_textarea_set_one_line(textarea, 1);
    lv_pman_register_obj_id(handle, textarea, TEXTAREA_ID);
    lv_obj_add_flag(textarea, LV_OBJ_FLAG_CLICKABLE);
    lv_group_focus_obj(textarea);
    pdata->textarea = textarea;

    lv_obj_t *keyboard = lv_keyboard_create(lv_scr_act());
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
                        case SKIP_BTN_ID:
                            if (model_is_stuck_on_download(pmodel)) {
                                // Do nothing, we can't move on if we are stuck on download
                            } else {
                                if (model_next_test(pmodel) == 0) {
                                    update_page(pmodel, pdata);
                                    msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_START_TEST_UNIT;
                                }
                            }
                            break;

                        case PLAY_BTN_ID:
                            switch (model_get_cycle_state(pmodel)) {
                                case CYCLE_STATE_INTERRUPTED:
                                    if (model_is_stuck_on_download(pmodel)) {
                                        msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_DOWNLOAD;
                                    } else {
                                        msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_START_TEST_UNIT;
                                    }
                                    break;

                                case CYCLE_STATE_STOP:
                                    msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_START_TEST_UNIT;
                                    break;

                                default:
                                    break;
                            }
                            break;

                        case RESET_BTN_ID:
                            snprintf(msg.cmsg.imei, sizeof(msg.cmsg.imei), "%s", pdata->imei);
                            msg.cmsg.tag = LV_PMAN_CONTROLLER_MSG_TAG_RESET_TEST;

                            pdata->imei_submitted = 0;
                            lv_group_focus_obj(pdata->textarea);
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

                        case TEXTAREA_ID:
                            lv_group_focus_obj(pdata->textarea);
                            break;
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
                        case TEXTAREA_ID:
                        case KEYBOARD_ID:
                            snprintf(pdata->imei, sizeof(pdata->imei), "%s", lv_textarea_get_text(pdata->textarea));
                            pdata->imei_submitted = 1;
                            view_common_set_hidden(pdata->keyboard, 1);
                            update_page(pmodel, pdata);
                            break;
                    }
                    break;
                }

                case LV_EVENT_VALUE_CHANGED: {
                    switch (event.lvgl.id) {
                        case TEXTAREA_ID: {
                            break;
                        }
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
                    switch (model_get_cycle_state(pmodel)) {
                        case CYCLE_STATE_DOWNLOADING:
                            lv_obj_scroll_to_y(pdata->test_interface, lv_obj_get_y(pdata->download_widget.obj) - 64,
                                               LV_ANIM_ON);
                            break;

                        case CYCLE_STATE_STOP:
                            lv_obj_scroll_to_y(pdata->test_interface, 0, LV_ANIM_ON);
                            break;

                        case CYCLE_STATE_INTERRUPTED:
                            break;

                        case CYCLE_STATE_TESTING:
                            lv_obj_scroll_to_y(pdata->test_interface,
                                               lv_obj_get_y(pdata->test_widgets[model_get_test_index(pmodel)].obj) - 64,
                                               LV_ANIM_ON);
                            break;
                    }
                    update_page(pmodel, pdata);
                    break;
            }
            break;
        }
    }

    return msg;
}


static test_widget_t test_widget_create(lv_obj_t *root, uint16_t code) {
    char string[64] = {0};
    snprintf(string, sizeof(string), "%i: %s", code, test_code_to_string(code));
    return step_widget_create(root, string);
}


static test_widget_t step_widget_create(lv_obj_t *root, const char *text) {
    lv_obj_t *cont = lv_btn_create(root);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(cont, LV_PCT(98), TEST_CONT_COLLAPSED_HEIGHT);
    lv_obj_add_style(cont, (lv_style_t *)&style_unselected, LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(cont);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_anim_speed(lbl, 20, LV_STATE_DEFAULT);
    lv_obj_set_width(lbl, LV_PCT(94));
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text(lbl, text);

    lv_obj_t *lbl_err = lv_label_create(cont);
    lv_label_set_long_mode(lbl_err, LV_LABEL_LONG_SCROLL);
    lv_obj_set_width(lbl_err, LV_PCT(80));
    lv_obj_set_style_anim_speed(lbl_err, 20, LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(lbl_err, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    lv_obj_align(lbl_err, LV_ALIGN_BOTTOM_LEFT, 0, -16);

    lv_obj_t *btn_err = lv_btn_create(cont);
    lv_obj_t *btn_lbl = lv_label_create(btn_err);
    lv_obj_set_size(btn_err, 128, 48);
    lv_obj_set_style_bg_color(btn_err, STYLE_RED, LV_STATE_DEFAULT);
    lv_label_set_text(btn_lbl, "Salta");
    lv_obj_center(btn_lbl);
    lv_obj_align(btn_err, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    lv_obj_t *cb = lv_checkbox_create(cont);
    lv_obj_clear_flag(cb, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_color(cb, STYLE_FG_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(cb, 2, LV_PART_INDICATOR);
    lv_checkbox_set_text(cb, "");
    lv_obj_align(cb, LV_ALIGN_RIGHT_MID, 16, 0);

    lv_obj_t *spinner = lv_spinner_create(cont, 2000, 60);
    lv_obj_set_style_arc_color(spinner, STYLE_FG_COLOR, LV_PART_INDICATOR);
    lv_obj_set_size(spinner, 56, 56);
    lv_obj_align_to(spinner, cb, LV_ALIGN_CENTER, 0, 0);

    return (test_widget_t){
        .obj       = cont,
        .result_cb = cb,
        .spinner   = spinner,
        .lbl_err   = lbl_err,
        .lbl       = lbl,
        .btn_err   = btn_err,
    };
}


static void update_page(model_t *pmodel, struct page_data *pdata) {
    lv_obj_set_style_img_recolor(lv_obj_get_child(pdata->btn_connection, 0),
                                 model_get_communication_error(pmodel) ? STYLE_RED : STYLE_FG_COLOR, LV_STATE_DEFAULT);

    uint8_t control_disabled = model_get_cycle_state(pmodel) == CYCLE_STATE_TESTING ||
                               model_get_cycle_state(pmodel) == CYCLE_STATE_DOWNLOADING ||
                               model_get_communication_error(pmodel) || !pdata->imei_submitted;

    view_common_set_disabled(pdata->btn_play, control_disabled);
    view_common_set_disabled(pdata->btn_reset, control_disabled);

    if (pdata->imei_submitted) {
        lv_textarea_set_text(pdata->textarea, "");
        view_common_set_hidden(pdata->textarea, 1);
        view_common_set_hidden(pdata->keyboard, 1);
        view_common_set_hidden(pdata->test_interface, 0);
        update_test_interface(pmodel, pdata);

        switch (model_get_board_state(pmodel)) {
            case BOARD_STATE_ABSENT:
                lv_label_set_text(pdata->lbl_status, "Inserire la scheda nel sistema");
                view_common_set_hidden(pdata->curtain, 0);
                break;

            case BOARD_STATE_PRESENT_OPEN:
                lv_label_set_text(pdata->lbl_status, "Chiudere il letto ad aghi");
                view_common_set_hidden(pdata->curtain, 0);
                break;

            case BOARD_STATE_READY:
                view_common_set_hidden(pdata->curtain, 1);
                break;
        }
    } else {
        lv_label_set_text(pdata->lbl_status, "Scannerizzare o inserire il codice del dispositivo");
        lv_group_focus_obj(pdata->textarea);

        view_common_set_hidden(pdata->textarea, 0);
        view_common_set_hidden(pdata->curtain, 1);
        view_common_set_hidden(pdata->test_interface, 1);
    }
}


static void update_test_interface(model_t *pmodel, struct page_data *pdata) {
    view_common_set_hidden(pdata->download_widget.btn_err, 1);

    switch (model_get_downloading_state(pmodel)) {
        case DOWNLOADING_STATE_NONE:
            lv_obj_set_style_border_opa(pdata->download_widget.obj, LV_OPA_TRANSP, LV_STATE_DEFAULT);

            lv_obj_set_height(pdata->download_widget.obj, TEST_CONT_COLLAPSED_HEIGHT);
            lv_obj_align(pdata->download_widget.lbl, LV_ALIGN_LEFT_MID, 0, 0);
            lv_obj_align(pdata->download_widget.result_cb, LV_ALIGN_RIGHT_MID, 16, 0);

            view_common_set_hidden(pdata->download_widget.lbl_err, 1);
            view_common_set_hidden(pdata->download_widget.result_cb, 0);
            view_common_set_hidden(pdata->download_widget.spinner, 1);
            view_common_set_checked(pdata->download_widget.result_cb, 0);
            break;

        case DOWNLOADING_STATE_SUCCESSFUL:
            successful_step(pdata->download_widget);
            break;

        case DOWNLOADING_STATE_FAILED:
            failed_step(pdata->download_widget);

            lv_label_set_text(pdata->download_widget.lbl_err, "Programmazione fallita!");
            break;
    }

    size_t num_tests = model_get_num_tests_in_current_unit(pmodel);
    for (size_t i = 0; i < num_tests; i++) {
        if (model_get_test_done_history(pmodel, i) && model_get_cycle_state(pmodel) != CYCLE_STATE_STOP) {
            view_common_set_checked(pdata->test_widgets[i].result_cb, 1);

            uint16_t result = model_get_test_result_history(pmodel, i);
            switch (result) {
                case TEST_RESULT_OK:
                    successful_step(pdata->test_widgets[i]);
                    break;

                default:
                    failed_step(pdata->test_widgets[i]);

                    view_common_set_hidden(pdata->test_widgets[i].btn_err, model_get_test_index(pmodel) != i);

                    lv_label_set_text_fmt(pdata->test_widgets[i].lbl_err, "Errore %i: %s", result,
                                          error_to_string(result));

                    if (model_is_test_required(model_get_test_code_from_current_unit(pmodel, i)) ||
                        i == num_tests - 1) {
                        view_common_set_disabled(pdata->test_widgets[i].btn_err, 1);
                    } else {
                        view_common_set_disabled(pdata->test_widgets[i].btn_err, 0);
                    }
                    break;
            }
        } else {
            lv_obj_set_height(pdata->test_widgets[i].obj, TEST_CONT_COLLAPSED_HEIGHT);
            view_common_set_hidden(pdata->test_widgets[i].lbl_err, 1);
            view_common_set_hidden(pdata->test_widgets[i].btn_err, 1);
            view_common_set_checked(pdata->test_widgets[i].result_cb, 0);
        }

        if (test_is_selected(pmodel, i)) {
            lv_obj_set_style_border_opa(pdata->test_widgets[i].obj, LV_OPA_COVER, LV_STATE_DEFAULT);
        } else if (model_get_test_done_history(pmodel, i)) {
            // Do nothing
        } else {
            lv_obj_set_style_border_opa(pdata->test_widgets[i].obj, LV_OPA_TRANSP, LV_STATE_DEFAULT);
        }

        if (test_is_selected(pmodel, i)) {
            switch (model_get_test_state(pmodel)) {
                case TEST_STATE_STARTING:
                case TEST_STATE_IN_PROGRESS:
                    view_common_set_hidden(pdata->test_widgets[i].spinner, 0);
                    view_common_set_hidden(pdata->test_widgets[i].result_cb, 1);
                    break;

                default:
                    view_common_set_hidden(pdata->test_widgets[i].spinner, 1);
                    view_common_set_hidden(pdata->test_widgets[i].result_cb, 0);
                    break;
            }
        } else {
            view_common_set_hidden(pdata->test_widgets[i].spinner, 1);
            view_common_set_hidden(pdata->test_widgets[i].result_cb, 0);
        }
    }

    switch (model_get_cycle_state(pmodel)) {
        case CYCLE_STATE_STOP:
            lv_img_set_src(lv_obj_get_child(pdata->btn_play, 0), &img_icon_play);

            if (model_get_test_done(pmodel)) {
                if (model_get_test_ok(pmodel)) {
                    lv_obj_set_style_text_color(pdata->lbl_status, STYLE_GREEN, LV_STATE_DEFAULT);
                    lv_label_set_text(pdata->lbl_status, "Test concluso con successo");
                } else {
                    lv_obj_set_style_text_color(pdata->lbl_status, STYLE_RED, LV_STATE_DEFAULT);
                    lv_label_set_text(pdata->lbl_status, "Errori durante il test");
                }
            } else {
                lv_obj_set_style_text_color(pdata->lbl_status, STYLE_WHITE, LV_STATE_DEFAULT);
                lv_label_set_text(pdata->lbl_status, "Pronto all'esecuzione");
            }
            break;

        case CYCLE_STATE_INTERRUPTED:
            lv_img_set_src(lv_obj_get_child(pdata->btn_play, 0), &img_icon_reset);

            if ((model_get_test_index(pmodel) == 0 && !model_get_test_done_history(pmodel, 0)) ||
                model_is_stuck_on_download(pmodel)) {
                lv_obj_set_style_text_color(pdata->lbl_status, STYLE_WHITE, LV_STATE_DEFAULT);
                lv_label_set_text(pdata->lbl_status, "Riprova");
            } else {
                lv_obj_set_style_text_color(pdata->lbl_status, STYLE_WHITE, LV_STATE_DEFAULT);
                lv_label_set_text(pdata->lbl_status, "Test interrotto");
            }
            break;

        case CYCLE_STATE_TESTING:
            lv_obj_set_style_text_color(pdata->lbl_status, STYLE_WHITE, LV_STATE_DEFAULT);
            lv_label_set_text(pdata->lbl_status, "Test in corso");
            break;

        case CYCLE_STATE_DOWNLOADING:
            view_common_set_hidden(pdata->download_widget.lbl_err, 1);
            view_common_set_hidden(pdata->download_widget.spinner, 0);
            view_common_set_hidden(pdata->download_widget.result_cb, 1);
            lv_label_set_text(pdata->lbl_status, "Caricamento del firmware in corso");
            break;
    }

    if (model_get_communication_error(pmodel)) {
        lv_obj_set_style_text_color(pdata->lbl_status, STYLE_RED, LV_STATE_DEFAULT);
        lv_label_set_text(pdata->lbl_status, "Errore di comunicazione!");
    }
}


static uint8_t test_is_selected(model_t *pmodel, size_t num) {
    if (model_get_test_index(pmodel) == num) {
        switch (model_get_cycle_state(pmodel)) {
            case CYCLE_STATE_DOWNLOADING:
                // During download no test is selected
                return 0;

            case CYCLE_STATE_TESTING:
                // No download in progress, this is the current test
                return 1;

            case CYCLE_STATE_INTERRUPTED:
                // Downloading has failed and we are stopped on that
                return !(model_get_downloading_state(pmodel) == DOWNLOADING_STATE_FAILED &&
                         !model_get_test_done_history(pmodel, num));

            default:
                return 0;
        }
    } else {
        return 0;
    }
}


static void successful_step(test_widget_t widget) {
    lv_obj_set_style_border_opa(widget.obj, LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(widget.obj, STYLE_GREEN, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(widget.result_cb, LV_SYMBOL_OK, LV_STATE_CHECKED | LV_PART_INDICATOR);

    lv_obj_set_height(widget.obj, TEST_CONT_COLLAPSED_HEIGHT);
    lv_obj_align(widget.lbl, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_align(widget.result_cb, LV_ALIGN_RIGHT_MID, 16, 0);

    view_common_set_hidden(widget.lbl_err, 1);
    view_common_set_hidden(widget.spinner, 1);
    view_common_set_hidden(widget.result_cb, 0);
    view_common_set_checked(widget.result_cb, 1);
}


static void failed_step(test_widget_t widget) {
    lv_obj_set_height(widget.obj, TEST_CONT_EXTENDED_HEIGHT);
    lv_obj_align(widget.lbl, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_align(widget.result_cb, LV_ALIGN_TOP_RIGHT, 16, 0);
    view_common_set_hidden(widget.lbl_err, 0);

    lv_obj_set_style_border_opa(widget.obj, LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(widget.obj, STYLE_RED, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(widget.result_cb, LV_SYMBOL_CLOSE, LV_STATE_CHECKED | LV_PART_INDICATOR);

    view_common_set_hidden(widget.lbl_err, 0);
    view_common_set_hidden(widget.spinner, 1);
    view_common_set_hidden(widget.result_cb, 0);
    view_common_set_checked(widget.result_cb, 1);
}


const lv_pman_page_t page_main = {
    .create        = create_page,
    .destroy       = lv_pman_destroy_all,
    .open          = open_page,
    .close         = lv_pman_close_all,
    .process_event = process_page_event,
};