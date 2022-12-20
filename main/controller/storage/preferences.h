#ifndef PREFERENCES_H_INCLUDED
#define PREFERENCES_H_INCLUDED


#include <stdint.h>


void preferences_save_uint16(uint16_t *value, const char *key);
int  preferences_load_uint16(uint16_t *value, const char *key);


#endif