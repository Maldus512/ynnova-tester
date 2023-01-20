#include "lvgl.h"
#include "style.h"


static const lv_style_const_prop_t style_transparent_cont_props[] = {
    LV_STYLE_CONST_BG_OPA(LV_OPA_TRANSP),
    LV_STYLE_CONST_RADIUS(0),
    LV_STYLE_CONST_BORDER_WIDTH(0),
};
LV_STYLE_CONST_INIT(style_transparent_cont, style_transparent_cont_props);


static const lv_style_const_prop_t style_panel_props[] = {
    LV_STYLE_CONST_PAD_TOP(16),
    LV_STYLE_CONST_PAD_BOTTOM(16),
    LV_STYLE_CONST_PAD_LEFT(8),
    LV_STYLE_CONST_PAD_RIGHT(8),
};
LV_STYLE_CONST_INIT(style_panel, style_panel_props);


static const lv_style_const_prop_t style_unselected_props[] = {
    LV_STYLE_CONST_BORDER_WIDTH(4),
    LV_STYLE_CONST_BORDER_COLOR(STYLE_FG_COLOR),
    LV_STYLE_CONST_BORDER_OPA(LV_OPA_TRANSP),
};
LV_STYLE_CONST_INIT(style_unselected, style_unselected_props);


static const lv_style_const_prop_t style_selected_props[] = {
    LV_STYLE_CONST_BORDER_OPA(LV_OPA_COVER),
};
LV_STYLE_CONST_INIT(style_selected, style_selected_props);


static const lv_style_const_prop_t style_done_props[] = {
    LV_STYLE_CONST_BORDER_OPA(LV_OPA_COVER),
    LV_STYLE_CONST_BORDER_WIDTH(2),
};
LV_STYLE_CONST_INIT(style_done, style_done_props);


static const lv_style_const_prop_t style_toast_props[] = {
    LV_STYLE_CONST_PAD_BOTTOM(8),
    LV_STYLE_CONST_PAD_TOP(8),
    LV_STYLE_CONST_PAD_LEFT(16),
    LV_STYLE_CONST_PAD_RIGHT(16),
};
LV_STYLE_CONST_INIT(style_toast, style_toast_props);


static const lv_style_const_prop_t style_scrollbar_props[] = {
    LV_STYLE_CONST_PAD_BOTTOM(8),
    LV_STYLE_CONST_PAD_TOP(8),
    LV_STYLE_CONST_PAD_LEFT(4),
    LV_STYLE_CONST_PAD_RIGHT(4),
    LV_STYLE_CONST_WIDTH(16),
    LV_STYLE_CONST_OPA(LV_OPA_80),
    LV_STYLE_CONST_BG_COLOR(STYLE_FG_COLOR),
};
LV_STYLE_CONST_INIT(style_scrollbar, style_scrollbar_props);


lv_style_t style_icon = {0};


void style_init(void) {
    lv_style_init(&style_icon);
    lv_style_set_img_recolor_opa(&style_icon, LV_OPA_COVER);
    lv_style_set_img_recolor(&style_icon, STYLE_FG_COLOR);
}