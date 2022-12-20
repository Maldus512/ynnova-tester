#ifndef STFLASH_H_INCLUDED
#define STFLASH_H_INCLUDED


#include <stdint.h>


typedef enum {
    STFLASH_RESPONSE_OK = 0,
    STFLASH_RESPONSE_FAIL,
} stflash_response_t;


void    stflash_init(void);
void    stflash_run(void);
uint8_t stflash_get_response(stflash_response_t *response);


#endif