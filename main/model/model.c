#include <string.h>
#include <assert.h>
#include "model.h"


void model_init(model_t *pmodel) {
    assert(pmodel != NULL);

    pmodel->test_codes[0] = 2;
    pmodel->test_codes[1] = 3;
    pmodel->test_codes[2] = 4;
    pmodel->test_codes[3] = 5;
    pmodel->test_codes[4] = 6;
    pmodel->num_tests     = 5;

    pmodel->run.last_test           = 0;
    pmodel->run.communication_error = 0;
    pmodel->run.board_state         = BOARD_STATE_READY;
    pmodel->run.test_state          = TEST_STATE_DONE;
    pmodel->run.test_result         = TEST_RESULT_OK;

    pmodel->run.test_index = 0;

    memset(pmodel->run.test_result_history, 0, sizeof(pmodel->run.test_result_history));
    memset(pmodel->run.test_done_history, 0, sizeof(pmodel->run.test_result_history));
}


uint16_t model_get_test_code(model_t *pmodel, size_t num) {
    assert(pmodel != NULL);
    return pmodel->test_codes[num];
}


uint16_t model_get_current_test_code(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->test_codes[pmodel->run.test_index];
}


void model_reset_test_sequence(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->run.test_index = 0;
    memset(pmodel->run.test_done_history, 0, sizeof(pmodel->run.test_result_history));
}


uint8_t model_is_test_sequence_done(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->run.test_index >= model_get_num_tests(pmodel);
}


uint8_t model_next_test(model_t *pmodel) {
    assert(pmodel != NULL);
    if (pmodel->run.test_index < model_get_num_tests(pmodel)) {
        pmodel->run.test_index++;
    }

    return model_is_test_sequence_done(pmodel);
}


uint16_t model_get_test_result_history(model_t *pmodel, size_t num) {
    assert(pmodel != NULL);
    return pmodel->run.test_result_history[num];
}


uint8_t model_get_test_done_history(model_t *pmodel, size_t num) {
    assert(pmodel != NULL);
    return pmodel->run.test_done_history[num];
}


uint8_t model_set_test_status(model_t *pmodel, uint16_t last_executed_test, board_state_t board_state,
                              uint16_t test_state, uint16_t test_result) {
    assert(pmodel != NULL);
    uint8_t update = 0;

    update |= model_set_board_state(pmodel, board_state);
    update |= model_set_last_test(pmodel, last_executed_test);
    update |= model_set_test_result(pmodel, test_result);
    update |= model_set_test_state(pmodel, test_state);

    return update;
}


uint8_t model_set_test_state(model_t *pmodel, test_state_t state) {
    assert(pmodel != NULL);
    uint8_t update = 0;
    uint8_t done   = state == TEST_STATE_DONE;

    if (model_get_current_test_code(pmodel) == model_get_last_test(pmodel) && !model_is_test_sequence_done(pmodel) &&
        pmodel->run.test_done_history[model_get_test_index(pmodel)] != done) {
        pmodel->run.test_done_history[model_get_test_index(pmodel)] = done;
        update                                                      = 1;
    }

    if (pmodel->run.test_state != state) {
        pmodel->run.test_state = state;
        update                 = 1;
    }

    return update;
}


uint8_t model_set_test_result(model_t *pmodel, test_result_t result) {
    assert(pmodel != NULL);
    uint8_t update = 0;


    if (!model_is_test_sequence_done(pmodel) &&
        pmodel->run.test_result_history[model_get_test_index(pmodel)] != result) {
        pmodel->run.test_result_history[model_get_test_index(pmodel)] = result;
        update                                                        = 1;
    }

    if (pmodel->run.test_result != result) {
        pmodel->run.test_result = result;
        update                  = 1;
    }

    return update;
}


uint16_t model_get_test_index(model_t *pmodel) {
    assert(pmodel != NULL);

    if (model_is_test_sequence_done(pmodel)) {
        return 0;
    } else {
        return pmodel->run.test_index;
    }
}