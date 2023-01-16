#ifndef STYLE_H_INCLUDED
#define STYLE_H_INCLUDED


#include "lvgl.h"


#define STYLE_FG_COLOR      ((lv_color_t)LV_COLOR_MAKE(0xB0, 0xB0, 0xB0))
#define STYLE_RED           ((lv_color_t)LV_COLOR_MAKE(0xAA, 0x10, 0x10))
#define STYLE_GREEN         ((lv_color_t)LV_COLOR_MAKE(0x10, 0xAA, 0x10))
#define STYLE_PRIMARY_COLOR ((lv_color_t)LV_COLOR_MAKE(70, 55, 55))


extern lv_style_t       style_icon;
extern const lv_style_t style_transparent_cont;
extern const lv_style_t style_panel;
extern const lv_style_t style_selected;
extern const lv_style_t style_unselected;
extern const lv_style_t style_scrollbar;
extern const lv_style_t style_toast;


void style_init(void);


#endif