#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED


#include "model/model.h"
#include "lv_page_manager.h"



void controller_init(model_t *pmodel);
void controller_manage_message(void *args, lv_pman_controller_msg_t msg);
void controller_manage(model_t *pmodel);


#endif