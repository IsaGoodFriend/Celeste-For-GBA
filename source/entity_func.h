#pragma once
#include "entities.h"

int CHAR_init(unsigned int index, unsigned char* data, unsigned char* is_loading);
void CHAR_update(unsigned int);
void CHAR_render(unsigned int);

int STRAWB_init(unsigned int index, unsigned char* data, unsigned char* is_loading);
void STRAWB_update(unsigned int index);
void STRAWB_render(unsigned int index);
