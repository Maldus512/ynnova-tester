#include <stdio.h>
#include <unistd.h>
#include "lvgl.h"
#include "sdl/sdl.h"
#include "utils/system_time.h"
#include "model/model.h"
#include "view/view.h"
#include "controller/controller.h"


int main(void) {
    static unsigned long last_invoked = 0;
    model_t              model        = {};

    lv_init();
    sdl_init();

    model_init(&model);
    view_init(&model, controller_manage_message);
    controller_init(&model);

    printf("Begin main loop\n");
    for (;;) {
        // Run LVGL engine
        if (last_invoked > 0) {
            lv_tick_inc(get_millis() - last_invoked);
        }
        last_invoked = get_millis();
        lv_timer_handler();

        controller_manage(&model);

        usleep(1000);
    }

    return 0;
}
