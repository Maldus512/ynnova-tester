#include <stdio.h>
#include "controller.h"
#include "model/model.h"
#include "view/view.h"
#include "lv_page_manager.h"
#include "machine/machine.h"
#include "storage/configuration.h"
#include "log.h"
#include "utils/system_time.h"
#include "gel/timer/timecheck.h"
#include "stflash.h"
#include "view/common.h"
#include "observer.h"


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
            model_set_downloading(pmodel, 1);
            stflash_run();
            view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
            break;

        case LV_PMAN_CONTROLLER_MSG_TAG_RESET_TEST:
            machine_reset_test();
            break;

        case LV_PMAN_CONTROLLER_MSG_TAG_START_TEST:
            machine_start_test(msg.test);
            break;

        case LV_PMAN_CONTROLLER_MSG_TAG_RESTART_COMMUNICATION:
            machine_restart_communication();
            break;

        case LV_PMAN_CONTROLLER_MSG_TAG_START_TEST_UNIT:
            machine_start_test(model_get_current_test_code(pmodel));
            model_set_cycle_state(pmodel, CYCLE_STATE_RUNNING);
            view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
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
                    model_set_cycle_state(pmodel, CYCLE_STATE_STOP);
                    view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
                }
                break;

            case MACHINE_RESPONSE_MESSAGE_TAG_STATUS: {
                uint8_t update = model_set_communication_error(pmodel, 0);
                update |= model_set_test_status(pmodel, machine_msg.last_executed_test, machine_msg.board_state,
                                                machine_msg.test_state, machine_msg.test_result);

                if (update) {
                    if (old_test_result != model_get_test_result(pmodel)) {
                        machine_test_error();
                    }

                    if (model_get_cycle_state(pmodel) == CYCLE_STATE_RUNNING &&
                        model_get_test_state(pmodel) == TEST_STATE_DONE) {
                        if (model_get_test_result(pmodel) != TEST_RESULT_OK) {
                            model_set_cycle_state(pmodel, CYCLE_STATE_STOP);
                        } else {
                            if (model_next_test(pmodel) == 0) {
                                machine_start_test(model_get_current_test_code(pmodel));
                            } else {
                                machine_test_done();
                                model_set_cycle_state(pmodel, CYCLE_STATE_STOP);
                            }
                        }
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
                model_set_downloading(pmodel, 0);
                view_common_toast("Firmware caricato con successo");
                break;

            case STFLASH_RESPONSE_FAIL:
                model_set_downloading(pmodel, 0);
                view_common_toast("Caricamento del firmware fallito");
                break;
        }
        view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
    }

    if (is_expired(status_ts, get_millis(), 200UL)) {
        machine_read_status();

        status_ts = get_millis();
    }

    observer_observe(pmodel);
}