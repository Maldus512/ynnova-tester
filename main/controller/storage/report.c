#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "report.h"
#include "model/model.h"
#include "config/app_config.h"
#include "log.h"
#include "view/descriptions/test_code.h"
#include "view/descriptions/error.h"


#define LOGS_DIR APP_CONFIG_DATA_PATH "/logs"


void report_save(const char *name, model_t *pmodel) {
    char path[128] = {0};
    snprintf(path, sizeof(path), LOGS_DIR "/%s.csv", name);

    DIR *dir = opendir(LOGS_DIR);
    if (dir) {
        /* Directory exists. */
        closedir(dir);
    } else if (ENOENT == errno) {
        /* Directory does not exist. */
        mkdir(LOGS_DIR, 0731);
    }

    FILE *f = fopen(path, "w");
    if (f == NULL) {
        log_error("Error while opening %s: %s", path, strerror(errno));
        return;
    }

    size_t num_tests = model_get_num_tests_in_current_unit(pmodel);

    for (size_t i = 0; i < num_tests; i++) {
        uint16_t code   = model_get_test_code_from_current_unit(pmodel, i);
        uint8_t  done   = model_get_test_done_history(pmodel, i);
        uint16_t result = model_get_test_result_history(pmodel, i);

        char line[256] = {0};
        if (done) {
            const char *test_string  = test_code_to_string(code);
            const char *error_string = error_to_string(result);
            snprintf(line, sizeof(line), "%i, %s, %i, %s\n", code, test_string == NULL ? "" : test_string, result,
                     error_string == NULL ? "" : error_string);
        }

        fwrite(line, 1, strlen(line), f);
    }

    if (model_get_downloading_state(pmodel) == DOWNLOADING_STATE_SUCCESSFUL) {
        const char *line = ", Programmazione, , Successo";
        fwrite(line, 1, strlen(line), f);
    } else {
        const char *line = ", Programmazione, , Fallimento";
        fwrite(line, 1, strlen(line), f);
    }

    fclose(f);
}