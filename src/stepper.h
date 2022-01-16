#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

void els_stepper_setup(void);
void els_stepper_start(void);
void els_stepper_stop(void);

void els_stepper_move_x(double mm, double speed_mm_s);
void els_stepper_move_z(double mm, double speed_mm_s);
void els_stepper_move_xz(double x_mm, double z_mm, double speed_mm_s);
void els_stepper_move_cw_arc(double radius, double xd, double speed_mm_s);
void els_stepper_move_ccw_arc(double radius, double xd, double speed_mm_s);

void els_stepper_zero_x(void);
void els_stepper_zero_z(void);

typedef struct {
  double xpos;
  double zpos;
  bool   xbusy;
  bool   zbusy;
} els_stepper_t;

extern const els_stepper_t *els_stepper;

#ifdef __cplusplus
}
#endif
