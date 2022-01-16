#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio.h"

typedef enum {
  ELS_S_DIRECTION_IDLE = 1,
  ELS_S_DIRECTION_CW   = 2,
  ELS_S_DIRECTION_CCW  = 3
} els_spindle_direction_t;

void els_spindle_setup(void);
void els_spindle_update(void);
float els_spindle_get_angle(void);
uint16_t els_spindle_get_counter(void);
uint16_t els_spindle_get_rpm(void);
els_spindle_direction_t els_spindle_get_direction(void);

#ifdef __cplusplus
}
#endif

