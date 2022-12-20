#ifndef CONFIGURATION_H_INCLUDED
#define CONFIGURATION_H_INCLUDED


#include "model/model.h"


void configuration_init(model_t *pmodel);
void configuration_save_tests(model_t *pmodel);


extern const char *CONFIGURATION_TEST_INDEX_KEY;


#endif