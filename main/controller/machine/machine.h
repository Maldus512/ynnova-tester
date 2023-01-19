#ifndef MACHINE_H_INCLUDED
#define MACHINE_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
#include "model/model.h"


typedef enum {
    MACHINE_RESPONSE_MESSAGE_TAG_ERROR,
    MACHINE_RESPONSE_MESSAGE_TAG_STATUS,
} machine_response_message_tag_t;


typedef struct {
    machine_response_message_tag_t tag;

    union {
        struct {
            uint16_t      last_executed_test;
            board_state_t board_state;
            test_state_t  test_state;
            test_result_t test_result;
        };
    };
} machine_response_message_t;


void machine_init(void);
int  machine_get_response(machine_response_message_t *msg);
void machine_read_status(void);
void machine_restart_communication(void);
void machine_start_test(uint16_t code);
void machine_reset_test(void);
void machine_test_done(void);
void machine_test_error(void);


#endif