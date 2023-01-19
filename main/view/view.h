#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED


#include "model/model.h"
#include "lv_page_manager.h"


void view_init(model_t *pmodel, void (*controller_cb)(void *, lv_pman_controller_msg_t));
void view_change_page(model_t *pmodel, lv_pman_page_t page);
void view_simple_event(lv_pman_user_event_tag_t tag);


extern const lv_pman_page_t page_main, page_settings, page_test_unit, page_initial_choice;

#endif