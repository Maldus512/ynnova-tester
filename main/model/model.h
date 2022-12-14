#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>


#define MAX_TEST_SUITE_LENGTH 32


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


typedef struct {
    uint16_t test_codes[MAX_TEST_SUITE_LENGTH];
    uint16_t num_tests;

    struct {
        uint16_t      last_test;
        uint8_t       communication_error;
        board_state_t board_state;
        test_state_t  test_state;
        test_result_t test_result;

        size_t   test_index;
        uint16_t test_result_history[MAX_TEST_SUITE_LENGTH];
        uint8_t  test_done_history[MAX_TEST_SUITE_LENGTH];
    } run;
} model_t;


void     model_init(model_t *pmodel);
uint16_t model_get_test_code(model_t *pmodel, size_t num);
uint16_t model_get_current_test_code(model_t *pmodel);
void     model_reset_test_sequence(model_t *pmodel);
uint8_t  model_next_test(model_t *pmodel);
uint16_t model_get_test_result_history(model_t *pmodel, size_t num);
uint8_t  model_set_test_result(model_t *pmodel, test_result_t result);
uint8_t  model_get_test_done_history(model_t *pmodel, size_t num);
uint8_t  model_set_test_state(model_t *pmodel, test_state_t state);
uint8_t  model_set_test_status(model_t *pmodel, uint16_t last_executed_test, board_state_t board_state,
                               uint16_t test_state, uint16_t test_result);
uint8_t  model_is_test_sequence_done(model_t *pmodel);
uint16_t model_get_test_index(model_t *pmodel);

GETTERNSETTER(last_test, run.last_test);
GETTERNSETTER(communication_error, run.communication_error);
GETTERNSETTER(board_state, run.board_state);
GETTERNSETTER(num_tests, num_tests);

GETTER(test_result, run.test_result);
GETTER(test_state, run.test_state);

SETTER(test_index, run.test_index);

#endif