#include <stdio.h>
#include "controller.h"
#include "model/model.h"
#include "view/view.h"
#include "lv_page_manager.h"
#include "machine/machine.h"
#include "storage/configuration.h"
#include "storage/report.h"
#include "log.h"
#include "utils/system_time.h"
#include "gel/timer/timecheck.h"
#include "stflash.h"
#include "view/common.h"
#include "observer.h"


static void reset_test_sequence(model_t *pmodel, const char *name);


static uint8_t downloaded = 0;


void controller_init(model_t *pmodel) {
    machine_init();
    stflash_init();
    configuration_init(pmodel);
    observer_init(pmodel);

    machine_reset_test();

    view_change_page(pmodel, page_initial_choice);
}


void controller_manage_message(void *args, lv_pman_controller_msg_t msg) {
    model_t *pmodel = args;

    switch (msg.tag) {
        case LV_PMAN_CONTROLLER_MSG_TAG_NONE:
            break;

        case LV_PMAN_CONTROLLER_MSG_TAG_SAVE:
            configuration_save_tests(pmodel);
            break;

        case LV_PMAN_CONTROLLER_MSG_TAG_DOWNLOAD:
            model_set_cycle_state(pmodel, CYCLE_STATE_DOWNLOADING);
            stflash_run();
            view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
            break;

        case LV_PMAN_CONTROLLER_MSG_TAG_RESET_TEST:
            reset_test_sequence(pmodel, msg.imei);
            break;

        case LV_PMAN_CONTROLLER_MSG_TAG_RESTART_COMMUNICATION:
            machine_restart_communication();
            break;

        case LV_PMAN_CONTROLLER_MSG_TAG_START_TEST_UNIT:
            if (model_is_test_sequence_done(pmodel)) {
                // Do nothing
            } else {
                machine_start_test(model_get_current_test_code(pmodel));
                model_set_test_state(pmodel, TEST_STATE_STARTING);
                model_set_cycle_state(pmodel, CYCLE_STATE_TESTING);
                view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
            }
            break;
    }
}


void controller_manage(model_t *pmodel) {
    static uint16_t      old_test_result = TEST_RESULT_OK;
    static unsigned long status_ts       = 0;

    machine_response_message_t machine_msg = {0};

    if (machine_get_response(&machine_msg) > 0) {
        switch (machine_msg.tag) {
            case MACHINE_RESPONSE_MESSAGE_TAG_ERROR:
                log_error("Machine message error");
                if (model_set_communication_error(pmodel, 1)) {
                    model_set_cycle_state(pmodel, CYCLE_STATE_INTERRUPTED);
                    view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
                }
                break;

            case MACHINE_RESPONSE_MESSAGE_TAG_STATUS: {
                uint8_t update = model_set_communication_error(pmodel, 0);
                update |= model_set_test_status(pmodel, machine_msg.last_executed_test, machine_msg.board_state,
                                                machine_msg.test_state, machine_msg.test_result);

                if (update) {
                    log_info("New test state: %i %i %i", model_get_test_result(pmodel), model_get_test_state(pmodel),
                             model_get_last_test(pmodel));

                    if (old_test_result != model_get_test_result(pmodel)) {
                        if (model_get_test_result(pmodel) != TEST_RESULT_OK && model_get_test_result(pmodel) != 0) {
                            machine_test_error();
                        }
                        old_test_result = model_get_test_result(pmodel);
                    }

                    if (model_get_cycle_state(pmodel) == CYCLE_STATE_TESTING &&
                        model_get_test_state(pmodel) == TEST_STATE_DONE &&
                        model_get_last_test(pmodel) == model_get_current_test_code(pmodel)) {
                        if (model_get_test_result(pmodel) != TEST_RESULT_OK) {
                            log_warn("Interrupting due to error");
                            model_set_cycle_state(pmodel, CYCLE_STATE_INTERRUPTED);
                        } else {
                            if (model_next_test(pmodel) == 0) {
                                if (model_get_current_test_code(pmodel) == TEST_CODE_60_PROG &&
                                    model_get_downloading_state(pmodel) == DOWNLOADING_STATE_NONE) {
                                    // Before moving on program the board
                                    model_set_cycle_state(pmodel, CYCLE_STATE_DOWNLOADING);
                                    log_info("Startint test for download");
                                } else {
                                    log_info("Running test %i", model_get_current_test_code(pmodel));
                                }
                                machine_start_test(model_get_current_test_code(pmodel));
                            } else if (model_get_downloading_state(pmodel) == DOWNLOADING_STATE_NONE) {
                                // As last step program the board
                                model_set_cycle_state(pmodel, CYCLE_STATE_DOWNLOADING);
                                stflash_run();
                            } else {
                                log_info("Test done!");
                                machine_test_done();
                                model_set_cycle_state(pmodel, CYCLE_STATE_STOP);
                            }
                        }
                    } else if (model_get_cycle_state(pmodel) == CYCLE_STATE_DOWNLOADING && downloaded == 0 &&
                               model_get_last_test(pmodel) == TEST_CODE_60_PROG) {
                        log_info("Downloading");
                        stflash_run();
                        downloaded = 1;
                    }

                    view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
                }
                break;
            }
        }
    }

    stflash_response_t response;

    if (stflash_get_response(&response)) {
        switch (response) {
            case STFLASH_RESPONSE_OK:
                model_set_downloading_state(pmodel, DOWNLOADING_STATE_SUCCESSFUL);
                // view_common_toast("Firmware caricato con successo");
                break;

            case STFLASH_RESPONSE_FAIL:
                model_set_downloading_state(pmodel, DOWNLOADING_STATE_FAILED);
                model_set_cycle_state(pmodel, CYCLE_STATE_INTERRUPTED);
                // view_common_toast("Caricamento del firmware fallito");
                machine_test_error();
                break;
        }

        if (model_get_cycle_state(pmodel) != CYCLE_STATE_INTERRUPTED) {
            model_next_test(pmodel);
            if (model_is_test_sequence_done(pmodel)) {
                machine_test_done();
                model_set_cycle_state(pmodel, CYCLE_STATE_INTERRUPTED);
            } else {
                machine_start_test(model_get_current_test_code(pmodel));
                model_set_cycle_state(pmodel, CYCLE_STATE_TESTING);
            }
        }

        view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
    }

    if (is_expired(status_ts, get_millis(), 100UL)) {
        machine_read_status();

        status_ts = get_millis();
    }

    observer_observe(pmodel);
}


static void reset_test_sequence(model_t *pmodel, const char *name) {
    report_save(name, pmodel);

    model_reset_test_sequence(pmodel);
    downloaded = 0;

    model_set_cycle_state(pmodel, CYCLE_STATE_STOP);
    machine_reset_test();

    view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
}