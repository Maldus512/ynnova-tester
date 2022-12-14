#include <stdio.h>
#include "controller.h"
#include "model/model.h"
#include "view/view.h"
#include "lv_page_manager.h"
#include "machine/machine.h"
#include "log.h"
#include "utils/system_time.h"
#include "gel/timer/timecheck.h"


void controller_init(model_t *pmodel) {
    machine_init();

    machine_reset_test();

    view_change_page(pmodel, page_main);
}


void controller_manage_message(void *args, lv_pman_controller_msg_t msg) {
    model_t *pmodel = args;

    switch (msg.tag) {
        case LV_PMAN_CONTROLLER_MSG_TAG_NONE:
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
    }
}


void controller_manage(model_t *pmodel) {
    static unsigned long       status_ts   = 0;
    machine_response_message_t machine_msg = {0};

    if (machine_get_response(&machine_msg) > 0) {
        switch (machine_msg.tag) {
            case MACHINE_RESPONSE_MESSAGE_TAG_ERROR:
                log_error("Machine message error");
                if (model_set_communication_error(pmodel, 1)) {
                    view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
                }
                break;

            case MACHINE_RESPONSE_MESSAGE_TAG_STATUS: {
                uint8_t update = model_set_communication_error(pmodel, 0);
                update |= model_set_test_status(pmodel, machine_msg.last_executed_test, machine_msg.board_state,
                                                machine_msg.test_state, machine_msg.test_result);

                if (update) {
                    view_simple_event(LV_PMAN_USER_EVENT_TAG_UPDATE);
                }
                break;
            }
        }
    }

    if (is_expired(status_ts, get_millis(), 200UL)) {
        machine_read_status();

        status_ts = get_millis();
    }
}