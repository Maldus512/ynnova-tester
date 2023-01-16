#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <assert.h>
#include "configuration.h"
#include "preferences.h"
#include "model/model.h"
#include "cJSON.h"
#include "app_config.h"
#include "log.h"


#define JSON_TESTS_PATH (APP_CONFIG_DATA_PATH "/test_units.json")
#define INVALID_JSON()  log_warn("Invalid JSON!");


const char        *CONFIGURATION_TEST_INDEX_KEY = "configuration_index";
static const char *JSON_NAME                    = "name";
static const char *JSON_TESTS                   = "tests";


void configuration_init(model_t *pmodel) {
    assert(pmodel != NULL);

    preferences_load_uint16(&pmodel->config.test_unit_index, CONFIGURATION_TEST_INDEX_KEY);


    DIR *dir = opendir(APP_CONFIG_DATA_PATH);
    if (dir) {
        /* Directory exists. */
        closedir(dir);
    } else if (ENOENT == errno) {
        /* Directory does not exist. */
        mkdir(APP_CONFIG_DATA_PATH, 0731);
    }


    FILE *f = fopen(JSON_TESTS_PATH, "r");
    if (f == NULL) {
        log_warn("Unable to open test units file: %s", strerror(errno));
    } else {
        fseek(f, 0L, SEEK_END);
        size_t size = ftell(f);

        uint8_t *buffer = malloc(size);
        assert(buffer != NULL);

        rewind(f);

        size = fread(buffer, 1, size, f);

        cJSON *json_list = cJSON_ParseWithLength((const char *)buffer, size);

        if (cJSON_IsArray(json_list)) {
            cJSON *json_test_unit = NULL;

            cJSON_ArrayForEach(json_test_unit, json_list) {
                if (cJSON_IsObject(json_test_unit)) {
                    uint64_t tests = 0;

                    cJSON *json_name = cJSON_GetObjectItem(json_test_unit, JSON_NAME);
                    if (!cJSON_IsString(json_name)) {
                        INVALID_JSON();
                        continue;
                    }

                    cJSON *json_tests = cJSON_GetObjectItem(json_test_unit, JSON_TESTS);
                    if (!cJSON_IsArray(json_tests)) {
                        INVALID_JSON();
                        continue;
                    }

                    cJSON *json_test_code = NULL;

                    cJSON_ArrayForEach(json_test_code, json_tests) {
                        if (!cJSON_IsNumber(json_test_code)) {
                            INVALID_JSON();
                            continue;
                        }

                        log_info("test no %i", json_test_code->valueint);
                        tests |= 1ULL << json_test_code->valueint;
                    }

                    model_add_test_unit(pmodel, json_name->valuestring, tests);
                } else {
                    INVALID_JSON();
                }
            }
        } else {
            INVALID_JSON();
        }

        cJSON_Delete(json_list);
        free(buffer);
        fclose(f);
    }
}


void configuration_save_tests(model_t *pmodel) {
    cJSON *json = cJSON_CreateArray();
    assert(json != NULL);

    for (size_t i = 0; i < model_get_num_custom_test_units(pmodel); i++) {
        cJSON *json_test_unit = cJSON_CreateObject();
        assert(json_test_unit != NULL);

        cJSON *json_name = cJSON_AddStringToObject(json_test_unit, JSON_NAME, model_get_test_unit_name(pmodel, i + 1));
        assert(json_name != NULL);

        cJSON *json_tests = cJSON_AddArrayToObject(json_test_unit, JSON_TESTS);
        assert(json_tests != NULL);

        for (size_t j = 0; j < sizeof(test_codes) / sizeof(test_codes[0]); j++) {
            if (model_is_test_configured(pmodel, i + 1, test_codes[j])) {
                cJSON *json_num = cJSON_CreateNumber(test_codes[j]);
                assert(json_num != NULL);

                cJSON_AddItemToArray(json_tests, json_num);
            }
        }

        cJSON_AddItemToArray(json, json_test_unit);
    }

    char *buffer = cJSON_Print(json);

    FILE *f = fopen(JSON_TESTS_PATH, "w");
    if (f == NULL) {
        log_warn("Unable to write test units file: %s", strerror(errno));
    } else {
        fwrite(buffer, 1, strlen(buffer), f);
        fclose(f);
    }

    cJSON_Delete(json);
}