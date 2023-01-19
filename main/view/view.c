#include "lv_page_manager.h"
#include "lvgl.h"
#include "sdl/sdl.h"
#include "model/model.h"
#include "style/style.h"


static lv_pman_t page_manager = {0};


void view_init(model_t *pmodel, void (*controller_cb)(void *, lv_pman_controller_msg_t)) {
#define BUFFER_SIZE (SDL_HOR_RES * SDL_VER_RES)
    /*A static or global variable to store the buffers*/
    static lv_disp_draw_buf_t disp_buf;

    /*Static or global buffer(s). The second buffer is optional*/
    static lv_color_t *buf_1[BUFFER_SIZE] = {0};

    /*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL instead buf_2 */
    lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, BUFFER_SIZE);

    static lv_disp_drv_t disp_drv;         /*A variable to hold the drivers. Must be static or global.*/
    lv_disp_drv_init(&disp_drv);           /*Basic initialization*/
    disp_drv.draw_buf = &disp_buf;         /*Set an initialized buffer*/
    disp_drv.flush_cb = sdl_display_flush; /*Set a flush callback to draw to the display*/
    disp_drv.hor_res  = SDL_HOR_RES;       /*Set the horizontal resolution in pixels*/
    disp_drv.ver_res  = SDL_VER_RES;       /*Set the vertical resolution in pixels*/

    style_init();

    lv_disp_t *disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
    lv_theme_default_init(disp, STYLE_PRIMARY_COLOR, lv_color_make(0x14, 0x14, 0x3C), 1, lv_font_default());

    static lv_indev_drv_t mouse_indev_drv;
    lv_indev_drv_init(&mouse_indev_drv); /*Basic initialization*/
    mouse_indev_drv.type    = LV_INDEV_TYPE_POINTER;
    mouse_indev_drv.read_cb = sdl_mouse_read;

    static lv_indev_t *indev = NULL;
    indev                    = lv_indev_drv_register(&mouse_indev_drv);

    static lv_indev_drv_t keyboard_indev_drv;
    lv_indev_drv_init(&keyboard_indev_drv);
    keyboard_indev_drv.type    = LV_INDEV_TYPE_KEYPAD;
    keyboard_indev_drv.read_cb = sdl_keyboard_read;
    lv_indev_t *kb_indev       = lv_indev_drv_register(&keyboard_indev_drv);

    static lv_indev_drv_t scroll_indev_drv;
    lv_indev_drv_init(&scroll_indev_drv);
    scroll_indev_drv.type        = LV_INDEV_TYPE_ENCODER;
    scroll_indev_drv.read_cb     = sdl_mousewheel_read;
    lv_indev_t *mousewheel_indev = lv_indev_drv_register(&scroll_indev_drv);

    lv_group_t *g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(kb_indev, g);
    lv_indev_set_group(mousewheel_indev, g);

    lv_pman_init(&page_manager, pmodel, indev, controller_cb);
}


void view_change_page(model_t *pmodel, lv_pman_page_t page) {
    lv_pman_change_page(&page_manager, pmodel, page);
}


void view_simple_event(lv_pman_user_event_tag_t tag) {
    lv_pman_event(&page_manager, (lv_pman_event_t){.tag = LV_PMAN_EVENT_TAG_USER, .user_event = {.tag = tag}});
}