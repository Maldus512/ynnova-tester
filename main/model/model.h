#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>


#define MAX_NUM_TEST_UNITS         8
#define MAX_TEST_SUITE_LENGTH      64
#define TEST_UNIT_NAME_LENGTH      32
#define DEFAULT_TEST_CONFIGURATION 0


#define GETTER(name, field)                                                                                            \
    static inline                                                                                                      \
        __attribute__((always_inline, const)) typeof(((model_t *)0)->field) model_get_##name(model_t *pmodel) {        \
        assert(pmodel != NULL);                                                                                        \
        return pmodel->field;                                                                                          \
    }

#define SETTER(name, field)                                                                                            \
    static inline __attribute__((always_inline))                                                                       \
    uint8_t model_set_##name(model_t *pmodel, typeof(((model_t *)0)->field) value) {                                   \
        assert(pmodel != NULL);                                                                                        \
        if (pmodel->field != value) {                                                                                  \
            pmodel->field = value;                                                                                     \
            return 1;                                                                                                  \
        } else {                                                                                                       \
            return 0;                                                                                                  \
        }                                                                                                              \
    }

#define TOGGLER(name, field)                                                                                           \
    static inline __attribute__((always_inline)) void model_toggle_##name(model_t *pmodel) {                           \
        assert(pmodel != NULL);                                                                                        \
        pmodel->field = !pmodel->field;                                                                                \
    }

#define GETTERNSETTER(name, field)                                                                                     \
    GETTER(name, field)                                                                                                \
    SETTER(name, field)


typedef enum {
    TEST_CODE_REQUIRED_02 = 2,
    TEST_CODE_REQUIRED_03 = 3,
    TEST_CODE_REQUIRED_04 = 4,
    TEST_CODE_REQUIRED_05 = 5,
    TEST_CODE_REQUIRED_06 = 6,
    TEST_CODE_REQUIRED_07 = 7,
    TEST_CODE_08          = 8,
    TEST_CODE_09          = 9,
    TEST_CODE_11          = 11,
    TEST_CODE_13          = 13,
    TEST_CODE_15          = 15,
    TEST_CODE_17          = 17,
    TEST_CODE_19          = 19,
    TEST_CODE_20          = 20,
    TEST_CODE_21          = 21,
    TEST_CODE_24          = 24,
    TEST_CODE_27          = 27,
    TEST_CODE_35          = 35,
    TEST_CODE_43          = 43,
    TEST_CODE_45          = 45,
    TEST_CODE_47          = 47,
    TEST_CODE_49          = 49,
    TEST_CODE_51          = 51,
    TEST_CODE_53          = 53,
    TEST_CODE_55          = 55,
    TEST_CODE_57          = 57,
    TEST_CODE_58          = 58,
    TEST_CODE_59          = 59,
    TEST_CODE_61          = 61,
    TEST_CODE_62          = 62,
} test_code_t;


typedef enum {
    BOARD_STATE_ABSENT = 0,
    BOARD_STATE_PRESENT_OPEN,
    BOARD_STATE_READY,
} board_state_t;


typedef enum {
    TEST_STATE_DONE,
    TEST_STATE_STARTING,
    TEST_STATE_IN_PROGRESS,
    TEST_STATE_FAILED,
} test_state_t;


typedef enum {
    TEST_RESULT_OK = 1,
} test_result_t;


typedef enum {
    CYCLE_STATE_STOP = 0,
    CYCLE_STATE_RUNNING,
    CYCLE_STATE_PROGRAMMING,
} cycle_state_t;


typedef struct {
    struct {
        uint64_t test_units[MAX_NUM_TEST_UNITS];
        char     test_unit_names[MAX_NUM_TEST_UNITS][TEST_UNIT_NAME_LENGTH];
        uint16_t test_unit_index;     // Selected test unit
        uint16_t num_custom_test_units;
    } config;

    struct {
        uint16_t      last_test;
        uint8_t       communication_error;
        board_state_t board_state;
        cycle_state_t cycle_state;
        test_state_t  test_state;
        test_result_t test_result;
        uint8_t       downloading;
        uint8_t       to_save;

        size_t   test_index;     // Test index in current test unit
        uint16_t test_result_history[MAX_TEST_SUITE_LENGTH];
        uint8_t  test_done_history[MAX_TEST_SUITE_LENGTH];
    } run;
} model_t;


void        model_init(model_t *pmodel);
uint16_t    model_get_test_code_from_current_unit(model_t *pmodel, size_t num);
uint16_t    model_get_current_test_code(model_t *pmodel);
void        model_reset_test_sequence(model_t *pmodel);
uint8_t     model_next_test(model_t *pmodel);
uint16_t    model_get_test_result_history(model_t *pmodel, size_t num);
uint8_t     model_set_test_result(model_t *pmodel, test_result_t result);
uint8_t     model_get_test_done_history(model_t *pmodel, size_t num);
uint8_t     model_set_test_state(model_t *pmodel, test_state_t state);
uint8_t     model_set_test_status(model_t *pmodel, uint16_t last_executed_test, board_state_t board_state,
                                  uint16_t test_state, uint16_t test_result);
uint8_t     model_is_test_sequence_done(model_t *pmodel);
uint16_t    model_get_test_index(model_t *pmodel);
uint8_t     model_is_test_configured_in_current_unit(model_t *pmodel, test_code_t code);
uint16_t    model_get_num_tests_in_current_unit(model_t *pmodel);
void        model_add_default_test_unit(model_t *pmodel);
const char *model_get_test_unit_name(model_t *pmodel, size_t test_unit_index);
uint16_t    model_get_num_test_units(model_t *pmodel);
void        model_add_test_unit(model_t *pmodel, const char *name, uint64_t tests);
uint8_t     model_is_test_configured(model_t *pmodel, size_t test_unit_index, test_code_t code);
void        model_remove_test_unit(model_t *pmodel, size_t test_unit_index);
void        model_toggle_test_configured(model_t *pmodel, size_t test_unit_index, test_code_t code);
uint8_t     model_is_test_required(test_code_t code);
void        model_set_test_unit_name(model_t *pmodel, size_t test_unit_index, const char *name);
uint8_t     model_get_test_done(model_t *pmodel);
uint8_t     model_get_test_ok(model_t *pmodel);

GETTERNSETTER(last_test, run.last_test);
GETTERNSETTER(communication_error, run.communication_error);
GETTERNSETTER(board_state, run.board_state);
GETTERNSETTER(downloading, run.downloading);
GETTERNSETTER(test_unit_index, config.test_unit_index);
GETTERNSETTER(num_custom_test_units, config.num_custom_test_units);
GETTERNSETTER(to_save, run.to_save);
GETTERNSETTER(cycle_state, run.cycle_state);

GETTER(test_result, run.test_result);
GETTER(test_state, run.test_state);

SETTER(test_index, run.test_index);


extern test_code_t test_codes[30];


#endif