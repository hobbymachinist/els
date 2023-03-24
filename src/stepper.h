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
void els_stepper_sync(void);

void els_stepper_move_x(double mm, double speed_mm_s);
void els_stepper_move_z(double mm, double speed_mm_s);

void els_stepper_move_x_no_accel(double mm, double speed_mm_s);
void els_stepper_move_z_no_accel(double mm, double speed_mm_s);

void els_stepper_move_xz(double x_mm, double z_mm, double speed_mm_s);

void els_stepper_x_backlash_fix(void);
void els_stepper_z_backlash_fix(void);

// clockwise arc quadrants.
void els_stepper_move_arc_q2_cw(double arc_center_z, double arc_center_x, double radius, double xd, double speed_mm_s);

// counter clockwise arc quadrants.
void els_stepper_move_arc_q3_ccw(double arc_center_z, double arc_center_x, double radius, double xd, double speed_mm_s);
void els_stepper_move_arc_q4_ccw(double arc_center_z, double arc_center_x, double radius, double xd, double speed_mm_s);

void els_stepper_zero_x(void);
void els_stepper_zero_z(void);

void els_stepper_disable_x(void);
void els_stepper_disable_z(void);

typedef struct {
  double xpos;
  double zpos;
  int8_t xdir;
  int8_t zdir;
  bool   xbusy;
  bool   zbusy;
} els_stepper_t;

extern els_stepper_t *els_stepper;

#ifdef __cplusplus
}
#endif
