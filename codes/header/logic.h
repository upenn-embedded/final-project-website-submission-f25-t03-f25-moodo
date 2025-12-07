#ifndef LOGIC_H
#define LOGIC_H

#include <stdint.h>

void logic_init(void);

void logic_update(uint8_t hour,
                  uint8_t minute,
                  uint8_t second,
                  int8_t  temp,
                  uint8_t water,
                  uint8_t mood);   

#endif
