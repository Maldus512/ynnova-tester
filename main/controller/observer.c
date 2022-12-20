#include "gel/data_structures/watcher.h"
#include "storage/preferences.h"
#include "model/model.h"
#include "storage/configuration.h"
#include "utils/system_time.h"


#define NUM_OBSERVED_VARIABLES 1


static watcher_t watchlist[NUM_OBSERVED_VARIABLES + 1] = {0};


void observer_init(model_t *pmodel) {
    size_t i = 0;

    watchlist[i++] =
        WATCHER(&pmodel->config.test_unit_index, preferences_save_uint16, CONFIGURATION_TEST_INDEX_KEY);

    assert(NUM_OBSERVED_VARIABLES == i);
    watchlist[i++] = WATCHER_NULL;

    watcher_list_init(watchlist);
}


void observer_observe(model_t *pmodel) {
    (void)pmodel;
    watcher_process_changes(watchlist, get_millis());
}