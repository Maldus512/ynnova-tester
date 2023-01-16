#ifndef LV_PMAN_CONF_H_INCLUDED
#define LV_PMAN_CONF_H_INCLUDED


#include "controller/message.h"


#define LV_PMAN_PAGE_STACK_DEPTH 16


//TODO: reverse the dependency
typedef enum {
    LV_PMAN_USER_EVENT_TAG_UPDATE,
} lv_pman_user_event_tag_t;


typedef struct {
    lv_pman_user_event_tag_t tag;
} lv_pman_user_event_t;



#endif