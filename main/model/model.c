#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "model.h"
#include "log.h"


test_code_t test_codes[31] = {
    TEST_CODE_REQUIRED_02, TEST_CODE_REQUIRED_03, TEST_CODE_REQUIRED_04, TEST_CODE_REQUIRED_05, TEST_CODE_REQUIRED_06,
    TEST_CODE_REQUIRED_07, TEST_CODE_08,          TEST_CODE_09,          TEST_CODE_11,          TEST_CODE_13,
    TEST_CODE_15,          TEST_CODE_17,          TEST_CODE_19,          TEST_CODE_20,          TEST_CODE_21,
    TEST_CODE_24,          TEST_CODE_27,          TEST_CODE_35,          TEST_CODE_43,          TEST_CODE_45,
    TEST_CODE_47,          TEST_CODE_49,          TEST_CODE_51,          TEST_CODE_53,          TEST_CODE_55,
    TEST_CODE_57,          TEST_CODE_58,          TEST_CODE_59,          TEST_CODE_60_PROG,     TEST_CODE_61,
    TEST_CODE_62,
};


void model_init(model_t *pmodel) {
    assert(pmodel != NULL);

    memset(pmodel->config.test_units, 0, sizeof(pmodel->config.test_units));
    memset(pmodel->config.test_unit_names, 0, sizeof(pmodel->config.test_unit_names));
    snprintf(pmodel->config.test_unit_names[DEFAULT_TEST_CONFIGURATION], TEST_UNIT_NAME_LENGTH, "DEFAULT");

    // Always at least the default test unit
    pmodel->config.test_unit_index       = DEFAULT_TEST_CONFIGURATION;
    pmodel->config.num_custom_test_units = 0;

    for (size_t i = 0; i < sizeof(test_codes) / sizeof(test_codes[0]); i++) {
        pmodel->config.test_units[DEFAULT_TEST_CONFIGURATION] |= (1ULL << test_codes[i]);
    }

    pmodel->run.last_test           = 0;
    pmodel->run.communication_error = 0;
    pmodel->run.board_state         = BOARD_STATE_READY;
    pmodel->run.test_state          = TEST_STATE_DONE;
    pmodel->run.test_result         = TEST_RESULT_OK;
    pmodel->run.cycle_state         = CYCLE_STATE_STOP;
    pmodel->run.downloading_state   = DOWNLOADING_STATE_NONE;
    pmodel->run.to_save             = 0;

    pmodel->run.test_index = 0;

    memset(pmodel->run.test_result_history, 0, sizeof(pmodel->run.test_result_history));
    memset(pmodel->run.test_done_history, 0, sizeof(pmodel->run.test_result_history));
}


uint16_t model_get_num_tests_in_current_unit(model_t *pmodel) {
    assert(pmodel != NULL);
    uint16_t count = 0;

    for (size_t i = 0; i < sizeof(pmodel->config.test_units[pmodel->config.test_unit_index]) * 8; i++) {
        if (model_is_test_required(i) ||
            (pmodel->config.test_units[pmodel->config.test_unit_index] & (1ULL << i)) > 0) {
            count++;
        }
    }

    return count;
}


uint8_t model_is_test_configured_in_current_unit(model_t *pmodel, test_code_t code) {
    assert(pmodel != NULL);
    return model_is_test_configured(pmodel, pmodel->config.test_unit_index, code);
}


uint8_t model_is_test_configured(model_t *pmodel, size_t test_unit_index, test_code_t code) {
    assert(pmodel != NULL);
    return model_is_test_required(code) || (pmodel->config.test_units[test_unit_index] & (1ULL << code)) > 0;
}


void model_toggle_test_configured(model_t *pmodel, size_t test_unit_index, test_code_t code) {
    assert(pmodel != NULL);
    if (!model_is_test_required(code)) {
        if (model_is_test_configured(pmodel, test_unit_index, code)) {
            pmodel->config.test_units[test_unit_index] &= ~(1ULL << code);
        } else {
            pmodel->config.test_units[test_unit_index] |= 1ULL << code;
        }
    }
}


uint16_t model_get_test_code_from_current_unit(model_t *pmodel, size_t num) {
    assert(pmodel != NULL);
    size_t count = 0;

    if (num == 0 && model_is_test_configured_in_current_unit(pmodel, test_codes[0])) {
        return test_codes[0];
    }

    size_t i = 0;
    for (i = 0; i < sizeof(test_codes) / sizeof(test_codes[0]); i++) {
        if (count == num && model_is_test_configured_in_current_unit(pmodel, test_codes[i])) {
            break;
        }
        if (model_is_test_configured_in_current_unit(pmodel, test_codes[i])) {
            count++;
        }
    }

    return test_codes[i];
}


uint16_t model_get_current_test_code(model_t *pmodel) {
    assert(pmodel != NULL);
    return model_get_test_code_from_current_unit(pmodel, pmodel->run.test_index);
}


void model_reset_test_sequence(model_t *pmodel) {
    assert(pmodel != NULL);
    pmodel->run.test_index        = 0;
    pmodel->run.downloading_state = DOWNLOADING_STATE_NONE;
    memset(pmodel->run.test_done_history, 0, sizeof(pmodel->run.test_done_history));
    memset(pmodel->run.test_result_history, 0, sizeof(pmodel->run.test_result_history));
}


uint8_t model_is_test_sequence_done(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->run.test_index >= model_get_num_tests_in_current_unit(pmodel);
}


uint8_t model_next_test(model_t *pmodel) {
    assert(pmodel != NULL);
    if (pmodel->run.test_index < model_get_num_tests_in_current_unit(pmodel)) {
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


uint8_t model_get_test_done(model_t *pmodel) {
    assert(pmodel != NULL);
    size_t num_tests = model_get_num_tests_in_current_unit(pmodel);
    for (size_t i = 0; i < num_tests; i++) {
        if (!model_get_test_done_history(pmodel, i)) {
            return 0;
        }
    }
    return 1;
}


uint8_t model_get_test_ok(model_t *pmodel) {
    assert(pmodel != NULL);
    size_t num_tests = model_get_num_tests_in_current_unit(pmodel);
    for (size_t i = 0; i < num_tests; i++) {
        if (model_get_test_done_history(pmodel, i) && model_get_test_result_history(pmodel, i) != TEST_RESULT_OK) {
            return 0;
        }
    }
    return 1;
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


void model_add_default_test_unit(model_t *pmodel) {
    assert(pmodel != NULL);

    uint64_t tests                       = 0;
    char     name[TEST_UNIT_NAME_LENGTH] = {0};

    size_t new_index = model_get_num_test_units(pmodel);
    snprintf(name, TEST_UNIT_NAME_LENGTH, "Unita' test %zu", new_index);

    for (size_t i = 0; i < sizeof(test_codes) / sizeof(test_codes[0]); i++) {
        tests |= (1ULL << test_codes[i]);
    }

    model_add_test_unit(pmodel, name, tests);
}


void model_add_test_unit(model_t *pmodel, const char *name, uint64_t tests) {
    assert(pmodel != NULL);

    if (model_get_num_test_units(pmodel) < MAX_NUM_TEST_UNITS) {
        size_t new_index = model_get_num_test_units(pmodel);

        pmodel->config.num_custom_test_units++;
        snprintf(pmodel->config.test_unit_names[new_index], TEST_UNIT_NAME_LENGTH, "%s", name);

        pmodel->config.test_units[new_index] = tests;
    }
}


void model_remove_test_unit(model_t *pmodel, size_t test_unit_index) {
    assert(pmodel != NULL && test_unit_index != 0 && test_unit_index < model_get_num_test_units(pmodel));

    for (size_t i = test_unit_index; i < (size_t)model_get_num_test_units(pmodel) - 1; i++) {
        pmodel->config.test_units[i] = pmodel->config.test_units[i + 1];
        strcpy(pmodel->config.test_unit_names[i], pmodel->config.test_unit_names[i + 1]);
    }

    pmodel->config.num_custom_test_units--;
    if (pmodel->config.test_unit_index >= pmodel->config.num_custom_test_units) {
        pmodel->config.test_unit_index = 0;
    }
}


const char *model_get_test_unit_name(model_t *pmodel, size_t test_unit_index) {
    assert(pmodel != NULL);
    return (const char *)pmodel->config.test_unit_names[test_unit_index];
}


void model_set_test_unit_name(model_t *pmodel, size_t test_unit_index, const char *name) {
    assert(pmodel != NULL);
    snprintf(pmodel->config.test_unit_names[test_unit_index], sizeof(pmodel->config.test_unit_names[test_unit_index]),
             "%s", name);
}


uint16_t model_get_num_test_units(model_t *pmodel) {
    assert(pmodel != NULL);
    return pmodel->config.num_custom_test_units + 1;
}


uint8_t model_is_test_required(test_code_t code) {
    switch (code) {
        case TEST_CODE_REQUIRED_02:
        case TEST_CODE_REQUIRED_03:
        case TEST_CODE_REQUIRED_04:
        case TEST_CODE_REQUIRED_05:
        case TEST_CODE_REQUIRED_06:
        case TEST_CODE_REQUIRED_07:
            return 1;

        default:
            return 0;
    }
}


uint8_t model_is_stuck_on_download(model_t *pmodel) {
    assert(pmodel != NULL);
    if (model_get_cycle_state(pmodel) == CYCLE_STATE_INTERRUPTED) {
        if (model_get_downloading_state(pmodel) == DOWNLOADING_STATE_NONE) {
            return 0;
        } else {
            // Downloading has failed and we are stopped on that
            return model_get_downloading_state(pmodel) == DOWNLOADING_STATE_FAILED &&
                   model_get_current_test_code(pmodel) == TEST_CODE_60_PROG;
        }
    } else {
        return 0;
    }
}