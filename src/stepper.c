#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#include <libopencm3/cm3/dwt.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/f4/timer.h>

#include "delay.h"
#include "gpio.h"
#include "nvic.h"
#include "timer.h"

#include "stepper.h"

#include "config.h"
#include "constants.h"
#include "dro.h"
#include "utils.h"

#define ELS_TIMER                     TIM7
#define ELS_TIMER_IRQ                 NVIC_TIM7_IRQ
#define ELS_TIMER_RCC                 RCC_TIM7
#define ELS_TIMER_RST                 RST_TIM7

#define ELS_SET_ZDIR_LR               els_gpio_set(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)
#define ELS_SET_ZDIR_RL               els_gpio_clear(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)

#define ELS_SET_XDIR_BT               els_gpio_set(ELS_X_DIR_PORT, ELS_X_DIR_PIN)
#define ELS_SET_XDIR_TB               els_gpio_clear(ELS_X_DIR_PORT, ELS_X_DIR_PIN)

// at 100khz, 50 pulses per second.
#define ELS_TIMER_RELOAD_MAX          500
#define ELS_TIMER_DECEL_STEPS         250

#define ELS_TIMER_ACCEL_X             2
#define ELS_TIMER_DECEL_X             1

#define ELS_TIMER_ACCEL_Z             3
#define ELS_TIMER_DECEL_Z             1

//==============================================================================
// Internal State
//==============================================================================

typedef enum {
  STEPPER_OP_NA,
  STEPPER_OP_LINE_X,
  STEPPER_OP_LINE_Z,
  STEPPER_OP_LINE_XZ,
  STEPPER_OP_LINE_ZX,
  STEPPER_OP_ARC_Q2_CW,
  STEPPER_OP_ARC_Q3_CCW,
  STEPPER_OP_ARC_Q4_CCW
} stepper_op_t;

static struct {
  els_stepper_t pos;

  int32_t xtarget;
  int32_t ztarget;

  volatile int32_t xsteps;
  double xdelta;
  uint32_t xfeed_um;
  stepper_op_t xop;

  // linear xz movement
  int32_t xz_line_n;
  int32_t xz_line_d;
  int32_t xz_line_err;

  volatile int32_t zsteps;
  double zdelta;
  uint32_t zfeed_um;
  stepper_op_t zop;

  // linear zx movement
  int32_t zx_line_n;
  int32_t zx_line_d;
  int32_t zx_line_err;

  // cw arc movement
  double cw_circle_radius;
  double cw_circle_center_z;
  double cw_circle_center_x;
  double cw_circle_zpos;

  // ccw arc movement
  double ccw_circle_radius;
  double ccw_circle_center_z;
  double ccw_circle_center_x;
  double ccw_circle_zpos;

  uint32_t xreload_target;
  uint32_t zreload_target;
} stepper;

//==============================================================================
// Exported constant
//==============================================================================
els_stepper_t *els_stepper = &stepper.pos;

//==============================================================================
// Internal functions
//==============================================================================
static void els_stepper_configure_gpio(void);
static void els_stepper_configure_timer(void);
static void els_stepper_configure_axes(void);

static void els_stepper_enable_x(void);
static void els_stepper_enable_z(void);
static void els_stepper_disable_x(void);
static void els_stepper_disable_z(void);

static void els_stepper_timer_start(void);
static void els_stepper_timer_stop(void);

static void els_stepper_timer_isr(void) __attribute__ ((interrupt ("IRQ")));
static void els_stepper_timer_x_update(uint32_t feed_um, bool accel);
static void els_stepper_timer_z_update(uint32_t feed_um, bool accel);

static void els_stepper_move_x_accel(double mm, double speed_mm_s, bool accel);
static void els_stepper_move_z_accel(double mm, double speed_mm_s, bool accel);

//==============================================================================
// API
//==============================================================================
void els_stepper_setup(void) {
  els_stepper_configure_axes();
}

void els_stepper_start(void) {
  // gpio & timer
  els_stepper_configure_gpio();
  els_stepper_configure_timer();

  // reset isr
  els_nvic_irq_set_handler(ELS_TIMER_IRQ, els_stepper_timer_isr);

  els_stepper_enable_z();
  els_stepper_enable_x();

  // default
  timer_set_period(ELS_TIMER, ELS_TIMER_RELOAD_MAX);

  stepper.xtarget = (int32_t)round((stepper.pos.xpos * 1e3));
  stepper.ztarget = (int32_t)round((stepper.pos.zpos * 1e3));

  els_stepper_timer_start();
}

void els_stepper_stop(void) {
  els_stepper_timer_stop();
  els_stepper_disable_z();
  els_stepper_disable_x();
}

void els_stepper_move_x(double mm, double speed_mm_s) {
  els_stepper_move_x_accel(mm, speed_mm_s, true);
}

void els_stepper_move_x_no_accel(double mm, double speed_mm_s) {
  els_stepper_move_x_accel(mm, speed_mm_s, false);
}

static void els_stepper_move_x_accel(double mm, double speed_mm_s, bool accel) {
  if (stepper.pos.xbusy) {
    if (stepper.xop != STEPPER_OP_LINE_X)
      return;

    int8_t dir = (mm > 0 ? 1 : -1);
    if (speed_mm_s * 1000 != stepper.xfeed_um || dir != stepper.pos.xdir)
      return;
  }
  else {
    stepper.xtarget = els_dro.xpos_um;
  }

  stepper.pos.xbusy = true;
  stepper.xtarget += (int32_t)round(mm * 1e3);
  els_stepper_timer_x_update(speed_mm_s * 1000, accel);
  if (mm < 0) {
    int32_t delta = els_dro.xpos_um;
    ELS_SET_XDIR_BT;
    if (stepper.pos.xdir != -1) {
      stepper.pos.xdir = -1;
      els_stepper_x_backlash_fix();
    }

    // re-adjust travel to compensate for overshooting.
    if (els_config->x_closed_loop) {
      delta -= els_dro.xpos_um;
      mm += (delta / 1000.0);
    }

    stepper.pos.xbusy = true;
    stepper.xsteps += (int32_t)round(-mm * els_config->z_pulses_per_mm);
  }
  else if (mm > 0) {
    int32_t delta = els_dro.xpos_um;
    ELS_SET_XDIR_TB;
    if (stepper.pos.xdir != 1) {
      stepper.pos.xdir = 1;
      els_stepper_x_backlash_fix();
    }

    // re-adjust travel to compensate for overshooting.
    if (els_config->x_closed_loop) {
      delta -= els_dro.xpos_um;
      mm += (delta / 1000.0);
    }

    stepper.pos.xbusy = true;
    stepper.xsteps += (int32_t)round(mm * els_config->x_pulses_per_mm);
  }
  stepper.xop = STEPPER_OP_LINE_X;
}

void els_stepper_move_z(double mm, double speed_mm_s) {
  els_stepper_move_z_accel(mm, speed_mm_s, true);
}

void els_stepper_move_z_no_accel(double mm, double speed_mm_s) {
  els_stepper_move_z_accel(mm, speed_mm_s, false);
}

static void els_stepper_move_z_accel(double mm, double speed_mm_s, bool accel) {
  if (stepper.pos.zbusy) {
    if (stepper.zop != STEPPER_OP_LINE_Z)
      return;

    int8_t dir = (mm > 0 ? 1 : -1);
    if (speed_mm_s * 1000 != stepper.zfeed_um || dir != stepper.pos.zdir)
      return;
  }
  else {
    stepper.ztarget = els_dro.zpos_um;
  }

  stepper.pos.zbusy = true;
  stepper.ztarget += (int32_t)round(mm * 1e3);
  els_stepper_timer_z_update(speed_mm_s * 1000, accel);
  if (mm < 0) {
    int32_t delta = els_dro.zpos_um;
    ELS_SET_ZDIR_RL;
    if (stepper.pos.zdir != -1) {
      stepper.pos.zdir = -1;
      els_stepper_z_backlash_fix();
    }

    // re-adjust travel to compensate for overshooting.
    if (els_config->z_closed_loop) {
      delta -= els_dro.zpos_um;
      mm += (delta / 1000.0);
    }

    stepper.pos.zbusy = true;
    stepper.zsteps += (int32_t)round(-mm * els_config->z_pulses_per_mm);
  }
  else if (mm > 0) {
    int32_t delta = els_dro.zpos_um;
    ELS_SET_ZDIR_LR;
    if (stepper.pos.zdir != 1) {
      stepper.pos.zdir = 1;
      els_stepper_z_backlash_fix();
    }

    // re-adjust travel to compensate for overshooting.
    if (els_config->z_closed_loop) {
      delta -= els_dro.zpos_um;
      mm += (delta / 1000.0);
    }

    stepper.pos.zbusy = true;
    stepper.zsteps += (int32_t)round(mm * els_config->z_pulses_per_mm);
  }
  stepper.zop = STEPPER_OP_LINE_Z;
}

void els_stepper_move_xz(double x_mm, double z_mm, double speed_mm_s) {
  if (stepper.pos.xbusy || stepper.pos.zbusy)
    return;

  stepper.xop = STEPPER_OP_NA;
  stepper.zop = STEPPER_OP_NA;
  stepper.pos.xbusy = true;
  stepper.pos.zbusy = true;

  if (x_mm < 0) {
    stepper.xsteps = (-x_mm * els_config->x_pulses_per_mm);
    ELS_SET_XDIR_BT;
    if (stepper.pos.xdir != -1) {
      stepper.pos.xdir = -1;
      els_stepper_x_backlash_fix();
    }
  }
  else {
    stepper.xsteps = (x_mm * els_config->x_pulses_per_mm);
    ELS_SET_XDIR_TB;
    if (stepper.pos.xdir != 1) {
      stepper.pos.xdir = 1;
      els_stepper_x_backlash_fix();
    }
  }

  if (z_mm < 0) {
    stepper.zsteps = (-z_mm * els_config->z_pulses_per_mm);
    ELS_SET_ZDIR_RL;
    if (stepper.pos.zdir != -1) {
      stepper.pos.zdir = -1;
      els_stepper_z_backlash_fix();
    }
  }
  else {
    stepper.zsteps = (z_mm * els_config->z_pulses_per_mm);
    ELS_SET_ZDIR_LR;
    if (stepper.pos.zdir != 1) {
      stepper.pos.zdir = 1;
      els_stepper_z_backlash_fix();
    }
  }

  els_stepper_x_backlash_fix();
  els_stepper_z_backlash_fix();

  uint32_t gcd = els_gcd(stepper.xsteps, stepper.zsteps);
  if (stepper.zsteps > stepper.xsteps) {
    stepper.zx_line_n = stepper.xsteps / gcd;
    stepper.zx_line_d = stepper.zsteps / gcd;
    stepper.zx_line_err = 0;
    els_stepper_timer_z_update(speed_mm_s * 1000, false);
    stepper.zop = STEPPER_OP_LINE_ZX;
  }
  else {
    stepper.xz_line_n = stepper.zsteps / gcd;
    stepper.xz_line_d = stepper.xsteps / gcd;
    stepper.xz_line_err = 0;
    els_stepper_timer_x_update(speed_mm_s * 1000, false);
    stepper.xop = STEPPER_OP_LINE_XZ;
  }
}

// Assumptions:
//
// Circular Arc Origin
//
// (cz, cx)
//
// Circular Clockwise Arc
//
// Circle Equation
//
// (z-cz)**2 + (x-cx)**2 = r**2
//
// Movement
//
// Arc starts at x = -xd and finishes at x = 0 and the motion travel is clockwise.
//
void els_stepper_move_arc_q2_cw(double cz, double cx, double radius, double xd, double speed_mm_s) {
  if (stepper.pos.xbusy || stepper.pos.zbusy || xd <= 0)
    return;

  els_stepper_timer_x_update(speed_mm_s * 1000, false);
  ELS_SET_XDIR_TB;
  if (stepper.pos.xdir != 1) {
    stepper.pos.xdir = 1;
    els_stepper_x_backlash_fix();
  }

  ELS_SET_ZDIR_RL;
  if (stepper.pos.zdir != -1) {
    stepper.pos.zdir = -1;
    els_stepper_z_backlash_fix();
  }

  stepper.pos.xbusy = true;
  stepper.pos.zbusy = true;
  stepper.cw_circle_radius = radius;
  stepper.cw_circle_center_z = cz;
  stepper.cw_circle_center_x = cx;
  stepper.cw_circle_zpos = stepper.pos.zpos;
  stepper.xsteps = xd * els_config->x_pulses_per_mm;
  stepper.xop = STEPPER_OP_ARC_Q2_CW;
}

// Assumptions:
//
// Circular Arc Origin
//
// (cz, cx)
//
// Circle Equation
//
// (z-cz)**2 + (x-cx)**2 = r**2
//
// Movement
//
// Arc starts at x = -xd and finishes at x = 0 and the motion travel is anti-clockwise.
//
void els_stepper_move_arc_q3_ccw(double cz, double cx, double radius, double xd, double speed_mm_s) {
  if (stepper.pos.xbusy || stepper.pos.zbusy || xd <= 0)
    return;

  els_stepper_timer_x_update(speed_mm_s * 1000, false);
  ELS_SET_XDIR_TB;
  if (stepper.pos.xdir != 1) {
    stepper.pos.xdir = 1;
    els_stepper_x_backlash_fix();
  }

  ELS_SET_ZDIR_LR;
  if (stepper.pos.zdir != 1) {
    stepper.pos.zdir = 1;
    els_stepper_z_backlash_fix();
  }

  stepper.pos.xbusy = true;
  stepper.pos.zbusy = true;
  stepper.ccw_circle_radius = radius;
  stepper.ccw_circle_center_z = cz;
  stepper.ccw_circle_center_x = cx;
  stepper.ccw_circle_zpos = stepper.pos.zpos;
  stepper.xsteps = xd * els_config->x_pulses_per_mm;
  stepper.xop = STEPPER_OP_ARC_Q3_CCW;
}

// Assumptions:
//
// Circular Arc Origin
//
// (cz, cx)
//
// Circle Equation
//
// (z-cz)**2 + (x-cx)**2 = r**2
//
// Movement
//
// Arc starts at x = -xd and finishes at x = 0 and the motion travel is anti-clockwise.
//
void els_stepper_move_arc_q4_ccw(double cz, double cx, double radius, double xd, double speed_mm_s) {
  if (stepper.pos.xbusy || stepper.pos.zbusy || xd <= 0)
    return;

  els_stepper_timer_x_update(speed_mm_s * 1000, false);
  ELS_SET_XDIR_TB;
  if (stepper.pos.xdir != 1) {
    stepper.pos.xdir = 1;
    els_stepper_x_backlash_fix();
  }

  ELS_SET_ZDIR_RL;
  if (stepper.pos.zdir != -1) {
    stepper.pos.zdir = -1;
    els_stepper_z_backlash_fix();
  }

  stepper.pos.xbusy = true;
  stepper.pos.zbusy = true;
  stepper.ccw_circle_radius = radius;
  stepper.ccw_circle_center_z = cz;
  stepper.ccw_circle_center_x = cx;
  stepper.ccw_circle_zpos = stepper.pos.zpos;
  stepper.xsteps = xd * els_config->x_pulses_per_mm;
  stepper.xop = STEPPER_OP_ARC_Q4_CCW;
}

void els_stepper_zero_x(void) {
  stepper.pos.xpos = 0;
  stepper.xtarget = 0;
}

void els_stepper_zero_z(void) {
  stepper.pos.zpos = 0;
}

//==============================================================================
// X axis, backlash compensation
//==============================================================================
void els_stepper_x_backlash_fix(void) {
  if (els_config->x_closed_loop) {
    size_t n = 0, max = els_config->x_pulses_per_mm >> 2;
    int32_t dro_xpos_um = els_dro.xpos_um;
    while (els_dro.xpos_um == dro_xpos_um && n < max) {
      els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
      els_delay_microseconds(2e3);
      els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
      els_delay_microseconds(2e3);
      n++;
    }
    stepper.pos.xpos = els_dro.xpos_um * 1e-3;
  }
  else {
    if (stepper.pos.xdir != 0 && els_config->x_backlash_pulses) {
      els_printf("x backlash compensation\n");
      for (size_t n = 0; n < els_config->x_backlash_pulses; n++) {
        els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
        els_delay_microseconds(ELS_BACKLASH_DELAY_US);
        els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
        els_delay_microseconds(ELS_BACKLASH_DELAY_US);
      }
    }
  }
}

//==============================================================================
// Z axis, backlash compensation
//==============================================================================
void els_stepper_z_backlash_fix(void) {
  if (els_config->z_closed_loop) {
    size_t n = 0, max = els_config->z_pulses_per_mm >> 2;
    int32_t dro_zpos_um = els_dro.zpos_um;
    while (els_dro.zpos_um == dro_zpos_um && n < max) {
      els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
      els_delay_microseconds(2e3);
      els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
      els_delay_microseconds(2e3);
      n++;
    }
    stepper.pos.zpos = els_dro.zpos_um * 1e-3;
  }
  else {
    if (stepper.pos.zdir != 0 && els_config->z_backlash_pulses) {
      els_printf("z backlash compensation\n");
      for (size_t n = 0; n < els_config->z_backlash_pulses; n++) {
        els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
        els_delay_microseconds(ELS_BACKLASH_DELAY_US);
        els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
        els_delay_microseconds(ELS_BACKLASH_DELAY_US);
      }
    }
  }
}

//==============================================================================
// Stepper path control
//==============================================================================
static void els_stepper_timer_z_line(void) {
  static volatile bool z_pul_state = 0;

  bool pending = false;
  if (els_config->z_closed_loop) {
    pending = (stepper.pos.zdir * (stepper.ztarget - els_dro.zpos_um)) >= 5;
    // TODO: overshot target, technically should flag this as an error.
    if (stepper.zsteps <= -400)
      pending = false;
  }
  else {
    pending = stepper.zsteps > 0;
  }

  if (pending || z_pul_state) {
    if (z_pul_state)
      els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    else
      els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);

    if (z_pul_state) {
      stepper.pos.zpos += (stepper.pos.zdir * stepper.zdelta);
      stepper.zsteps--;

      // accelerate or constant velocity
      if (stepper.zsteps > ELS_TIMER_DECEL_STEPS && TIM_ARR(ELS_TIMER) != stepper.zreload_target) {
        if (TIM_ARR(ELS_TIMER) > stepper.zreload_target)
          TIM_ARR(ELS_TIMER) = MAX(TIM_ARR(ELS_TIMER) - ELS_TIMER_ACCEL_Z, stepper.zreload_target);
        else
          TIM_ARR(ELS_TIMER) = stepper.zreload_target;
      }
      // decelerate
      else if (stepper.zsteps <= ELS_TIMER_DECEL_STEPS) {
        TIM_ARR(ELS_TIMER) = MIN(TIM_ARR(ELS_TIMER) + ELS_TIMER_DECEL_Z, ELS_TIMER_RELOAD_MAX);
      }
    }

    z_pul_state = !z_pul_state;
  }
  else {
    if (els_config->z_closed_loop)
      stepper.pos.zpos = els_dro.zpos_um * 1e-3;

    stepper.zsteps = 0;
    stepper.ztarget = els_dro.zpos_um;

    z_pul_state = 0;
    stepper.pos.zbusy = false;

    stepper.zop = STEPPER_OP_NA;
    TIM_ARR(ELS_TIMER) = ELS_TIMER_RELOAD_MAX;
  }
}

static void els_stepper_timer_x_line(void) {
  static volatile bool x_pul_state = 0;

  bool pending = false;
  if (els_config->x_closed_loop) {
    pending = (stepper.pos.xdir * (stepper.xtarget - els_dro.xpos_um)) >= 5;
    // TODO: overshot target, technically should flag this as an error.
    if (stepper.xsteps <= -400)
      pending = false;
  }
  else {
    pending = stepper.xsteps > 0;
  }

  if (pending || x_pul_state) {
    if (x_pul_state)
      els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    else
      els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);

    if (x_pul_state) {
      stepper.pos.xpos += (stepper.pos.xdir * stepper.xdelta);
      stepper.xsteps--;

      // accelerate or constant velocity
      if (stepper.xsteps > ELS_TIMER_DECEL_STEPS && TIM_ARR(ELS_TIMER) != stepper.xreload_target) {
        if (TIM_ARR(ELS_TIMER) > stepper.xreload_target)
          TIM_ARR(ELS_TIMER) = MAX(TIM_ARR(ELS_TIMER) - ELS_TIMER_ACCEL_X, stepper.xreload_target);
        else
          TIM_ARR(ELS_TIMER) = stepper.xreload_target;
      }
      // decelerate
      else if (stepper.xsteps <= ELS_TIMER_DECEL_STEPS) {
        TIM_ARR(ELS_TIMER) = MIN(TIM_ARR(ELS_TIMER) + ELS_TIMER_DECEL_X, ELS_TIMER_RELOAD_MAX);
      }
    }

    x_pul_state = !x_pul_state;
  }
  else {
    if (els_config->x_closed_loop)
      stepper.pos.xpos = els_dro.xpos_um * 1e-3;

    stepper.xsteps = 0;
    stepper.xtarget = els_dro.xpos_um;

    x_pul_state = 0;
    stepper.pos.xbusy = false;

    stepper.xop = STEPPER_OP_NA;
    TIM_ARR(ELS_TIMER) = ELS_TIMER_RELOAD_MAX;
  }
}

static void els_stepper_timer_zx_line(void) {
  static volatile bool z_pul_state = 0;
  static volatile bool x_pul_state = 0;
  static volatile bool pul_pending = 0;

  if (stepper.zsteps > 0) {
    if (z_pul_state)
      els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    else
      els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);

    if (z_pul_state) {
      stepper.pos.zpos += (stepper.pos.zdir * stepper.zdelta);
      stepper.zsteps--;
      stepper.zx_line_err += stepper.zx_line_n;
    }

    z_pul_state = !z_pul_state;
  }

  if (stepper.xsteps > 0) {
    if ((stepper.zx_line_err + stepper.zx_line_n) > stepper.zx_line_d ||
         pul_pending || !stepper.zsteps) {
      if (stepper.zx_line_err + stepper.zx_line_n > stepper.zx_line_d)
        stepper.zx_line_err -= stepper.zx_line_d;

      if (x_pul_state) {
        els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
        pul_pending = 0;
      }
      else {
        els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
        pul_pending = 1;
      }

      if (x_pul_state) {
        stepper.pos.xpos += (stepper.pos.xdir * stepper.xdelta);
        stepper.xsteps--;
      }

      x_pul_state = !x_pul_state;
    }
  }

  if (stepper.xsteps == 0 && stepper.zsteps == 0) {
    if (els_config->x_closed_loop)
      stepper.pos.xpos = els_dro.xpos_um * 1e-3;

    if (els_config->z_closed_loop)
      stepper.pos.zpos = els_dro.zpos_um * 1e-3;

    stepper.xtarget = els_dro.xpos_um;
    stepper.ztarget = els_dro.zpos_um;

    x_pul_state = z_pul_state = 0;
    stepper.pos.xbusy = stepper.pos.zbusy = false;

    stepper.xop = STEPPER_OP_NA;
    stepper.zop = STEPPER_OP_NA;
  }
}

static void els_stepper_timer_xz_line(void) {
  static volatile bool z_pul_state = 0;
  static volatile bool x_pul_state = 0;
  static volatile bool pul_pending = 0;

  if (stepper.xsteps > 0) {
    if (x_pul_state)
      els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    else
      els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);

    if (x_pul_state) {
      stepper.pos.xpos += (stepper.pos.xdir * stepper.xdelta);
      stepper.xsteps--;
      stepper.xz_line_err += stepper.xz_line_n;
    }

    x_pul_state = !x_pul_state;
  }

  if (stepper.zsteps > 0) {
    if (stepper.xz_line_err >= stepper.xz_line_d || pul_pending || !stepper.xsteps) {
      if (stepper.xz_line_err >= stepper.xz_line_d)
        stepper.xz_line_err -= stepper.xz_line_d;

      if (z_pul_state) {
        els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
        pul_pending = 0;
      }
      else {
        els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
        pul_pending = 1;
      }

      if (z_pul_state) {
        stepper.pos.zpos += (stepper.pos.zdir * stepper.zdelta);
        stepper.zsteps--;
      }

      z_pul_state = !z_pul_state;
    }
  }

  if (stepper.xsteps == 0 && stepper.zsteps == 0) {
    if (els_config->x_closed_loop)
      stepper.pos.xpos = els_dro.xpos_um * 1e-3;

    if (els_config->z_closed_loop)
      stepper.pos.zpos = els_dro.zpos_um * 1e-3;

    stepper.xtarget = els_dro.xpos_um;
    stepper.ztarget = els_dro.zpos_um;

    x_pul_state = z_pul_state = 0;
    stepper.pos.xbusy = stepper.pos.zbusy = false;

    stepper.xop = STEPPER_OP_NA;
    stepper.zop = STEPPER_OP_NA;
  }
}

static void els_stepper_timer_arc_q2_cw(void) {
  static volatile bool z_pul_state = 0;
  static volatile bool x_pul_state = 0;

  if (z_pul_state) {
    els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    stepper.pos.zpos += (stepper.pos.zdir * stepper.zdelta);
    z_pul_state = 0;
  }
  else if ((stepper.pos.zpos - stepper.cw_circle_zpos) >= stepper.zdelta) {
    els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    z_pul_state = 1;
  }
  else if (x_pul_state) {
    els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    x_pul_state = 0;

    stepper.pos.xpos += (stepper.pos.xdir * stepper.xdelta);
    stepper.xsteps--;

    stepper.cw_circle_zpos =
      sqrt(SQR(stepper.cw_circle_radius) - SQR(stepper.pos.xpos - stepper.cw_circle_center_x)) +
      stepper.cw_circle_center_z;
  }
  else if (stepper.xsteps > 0) {
    els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    x_pul_state = 1;
  }
  else {
    if (els_config->x_closed_loop)
      stepper.pos.xpos = els_dro.xpos_um * 1e-3;

    if (els_config->z_closed_loop)
      stepper.pos.zpos = els_dro.zpos_um * 1e-3;

    stepper.xtarget = els_dro.xpos_um;
    stepper.ztarget = els_dro.zpos_um;

    x_pul_state = z_pul_state = 0;
    stepper.pos.xbusy = stepper.pos.zbusy = false;

    stepper.xop = STEPPER_OP_NA;
    stepper.zop = STEPPER_OP_NA;
  }
}

static void els_stepper_timer_arc_q3_ccw(void) {
  static volatile bool z_pul_state = 0;
  static volatile bool x_pul_state = 0;

  if (z_pul_state) {
    els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    stepper.pos.zpos += (stepper.pos.zdir * stepper.zdelta);
    z_pul_state = 0;
  }
  else if ((stepper.ccw_circle_zpos - stepper.pos.zpos) >= stepper.zdelta) {
    els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    z_pul_state = 1;
  }
  else if (x_pul_state) {
    els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    x_pul_state = 0;

    stepper.pos.xpos += (stepper.pos.xdir * stepper.xdelta);
    stepper.xsteps--;

    stepper.ccw_circle_zpos =
      -sqrt(SQR(stepper.ccw_circle_radius) - SQR(stepper.pos.xpos - stepper.ccw_circle_center_x)) +
       stepper.ccw_circle_center_z;
  }
  else if (stepper.xsteps > 0) {
    els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    x_pul_state = 1;
  }
  else {
    if (els_config->x_closed_loop)
      stepper.pos.xpos = els_dro.xpos_um * 1e-3;

    if (els_config->z_closed_loop)
      stepper.pos.zpos = els_dro.zpos_um * 1e-3;

    stepper.xtarget = els_dro.xpos_um;
    stepper.ztarget = els_dro.zpos_um;

    x_pul_state = z_pul_state = 0;
    stepper.pos.xbusy = stepper.pos.zbusy = false;

    stepper.xop = STEPPER_OP_NA;
    stepper.zop = STEPPER_OP_NA;
  }
}

static void els_stepper_timer_arc_q4_ccw(void) {
  static volatile bool z_pul_state = 0;
  static volatile bool x_pul_state = 0;

  if (z_pul_state) {
    els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    stepper.pos.zpos += (stepper.pos.zdir * stepper.zdelta);
    z_pul_state = 0;
  }
  else if ((stepper.pos.zpos - stepper.ccw_circle_zpos) >= stepper.zdelta) {
    els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    z_pul_state = 1;
  }
  else if (x_pul_state) {
    els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    x_pul_state = 0;

    stepper.pos.xpos += (stepper.pos.xdir * stepper.xdelta);
    stepper.xsteps--;

    stepper.ccw_circle_zpos =
      -sqrt(SQR(stepper.ccw_circle_radius) - SQR(stepper.pos.xpos - stepper.ccw_circle_center_x)) +
       stepper.ccw_circle_center_z;
  }
  else if (stepper.xsteps > 0) {
    els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    x_pul_state = 1;
  }
  else {
    if (els_config->x_closed_loop)
      stepper.pos.xpos = els_dro.xpos_um * 1e-3;

    if (els_config->z_closed_loop)
      stepper.pos.zpos = els_dro.zpos_um * 1e-3;

    stepper.xtarget = els_dro.xpos_um;
    stepper.ztarget = els_dro.zpos_um;

    x_pul_state = z_pul_state = 0;
    stepper.pos.xbusy = stepper.pos.zbusy = false;

    stepper.xop = STEPPER_OP_NA;
    stepper.zop = STEPPER_OP_NA;
  }
}

//==============================================================================
// ISR
//==============================================================================
static void els_stepper_timer_isr(void) {
  TIM_SR(ELS_TIMER) &= ~TIM_SR_UIF;

  switch (stepper.zop) {
    case STEPPER_OP_LINE_Z:
      els_stepper_timer_z_line();
      break;
    case STEPPER_OP_LINE_ZX:
      els_stepper_timer_zx_line();
      break;
    default:
      break;
  }

  switch (stepper.xop) {
    case STEPPER_OP_LINE_X:
      els_stepper_timer_x_line();
      break;
    case STEPPER_OP_LINE_XZ:
      els_stepper_timer_xz_line();
      break;
    case STEPPER_OP_ARC_Q2_CW:
      els_stepper_timer_arc_q2_cw();
      break;
    case STEPPER_OP_ARC_Q3_CCW:
      els_stepper_timer_arc_q3_ccw();
      break;
    case STEPPER_OP_ARC_Q4_CCW:
      els_stepper_timer_arc_q4_ccw();
      break;
    default:
      break;
  }
}

// ----------------------------------------------------------------------------------
// GPIO: setup pins.
// ----------------------------------------------------------------------------------
static void els_stepper_configure_gpio(void) {
  els_gpio_mode_output(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  els_gpio_mode_output(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN);
  els_gpio_mode_output(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);

  els_gpio_mode_output(ELS_X_ENA_PORT, ELS_X_ENA_PIN);
  els_gpio_mode_output(ELS_X_DIR_PORT, ELS_X_DIR_PIN);
  els_gpio_mode_output(ELS_X_PUL_PORT, ELS_X_PUL_PIN);

  els_stepper_disable_x();
  els_stepper_disable_z();
}

static void els_stepper_configure_axes(void) {
  // Z-axis movement per pulse in mm.
  stepper.zdelta  = (1.0 / (double)els_config->z_pulses_per_mm);

  // X-axis movement per pulse in mm.
  stepper.xdelta  = (1.0 / (double)els_config->x_pulses_per_mm);
}

static void els_stepper_enable_z(void) {
  // active low.
  #if ELS_Z_ENA_ACTIVE_LOW
    els_gpio_clear(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  #else
    els_gpio_set(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  #endif
}

static void els_stepper_disable_z(void) {
  // active low.
  #if ELS_Z_ENA_ACTIVE_LOW
    els_gpio_set(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  #else
    els_gpio_clear(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  #endif
}

static void els_stepper_enable_x(void) {
  // active low.
  #if ELS_X_ENA_ACTIVE_LOW
    els_gpio_clear(ELS_X_ENA_PORT, ELS_X_ENA_PIN);
  #else
    els_gpio_set(ELS_X_ENA_PORT, ELS_X_ENA_PIN);
  #endif
}

static void els_stepper_disable_x(void) {
  // active low.
  #if ELS_X_ENA_ACTIVE_LOW
    els_gpio_set(ELS_X_ENA_PORT, ELS_X_ENA_PIN);
  #else
    els_gpio_clear(ELS_X_ENA_PORT, ELS_X_ENA_PIN);
  #endif
}

// ----------------------------------------------------------------------------------
// Timer functions.
// ----------------------------------------------------------------------------------
//
// Clock setup.
static void els_stepper_configure_timer(void) {
  rcc_periph_clock_enable(ELS_TIMER_RCC);
  rcc_periph_reset_pulse(ELS_TIMER_RST);

  // clock division 0, alignment edge, count up.
  timer_set_mode(ELS_TIMER, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  // 10us counter, ~100KHz
  timer_set_prescaler(ELS_TIMER, ((rcc_apb1_frequency * 2) / 100e3) - 1);

  // disable preload
  timer_disable_preload(ELS_TIMER);
  timer_continuous_mode(ELS_TIMER);

  nvic_set_priority(ELS_TIMER_IRQ, 5);
  nvic_enable_irq(ELS_TIMER_IRQ);
  timer_enable_update_event(ELS_TIMER);
}

static void els_stepper_timer_start(void) {
  timer_enable_counter(ELS_TIMER);
  timer_enable_irq(ELS_TIMER, TIM_DIER_UIE);
}

static void els_stepper_timer_stop(void) {
  timer_disable_irq(ELS_TIMER, TIM_DIER_UIE);
  timer_disable_counter(ELS_TIMER);
}

// Update timer frequency for required feed rate.
static void els_stepper_timer_z_update(uint32_t feed_um, bool accel) {
  uint32_t res;

  if (feed_um == stepper.zfeed_um && accel)
    return;

  stepper.zfeed_um = feed_um;

  // Figure out how many pulses per second is required for the feed speed.
  // Ignore rounding errors, we don't need to be super accurate here.
  //
  res = (feed_um * els_config->z_pulses_per_mm) / 1000;
  // We need to clock at twice the rate to toggle the gpio.
  res = res * 2;

  // Clock's at 100Khz, figure out the period
  // TODO: check if period is 0 - 65535
  res = 100e3 / res;

  timer_disable_irq(ELS_TIMER, TIM_DIER_UIE);
  timer_disable_counter(ELS_TIMER);

  stepper.zreload_target = res;

  if (accel)
    timer_set_period(ELS_TIMER, ELS_TIMER_RELOAD_MAX);
  else
    timer_set_period(ELS_TIMER, res);

  timer_enable_counter(ELS_TIMER);
  timer_enable_irq(ELS_TIMER, TIM_DIER_UIE);
}

// Update timer frequency for required feed rate.
static void els_stepper_timer_x_update(uint32_t feed_um, bool accel) {
  uint32_t res;

  if (feed_um == stepper.xfeed_um && accel)
    return;

  stepper.xfeed_um = feed_um;

  // Figure out how many pulses per second is required for the feed speed.
  // Ignore rounding errors, we don't need to be super accurate here.
  //
  res = (feed_um * els_config->x_pulses_per_mm) / 1000;
  // We need to clock at twice the rate to toggle the gpio.
  res = res * 2;

  // Clock's at 100Khz, figure out the period
  // TODO: check if period is 0 - 65535
  res = 100e3 / res;

  timer_disable_irq(ELS_TIMER, TIM_DIER_UIE);
  timer_disable_counter(ELS_TIMER);

  stepper.xreload_target = res;

  if (accel)
    timer_set_period(ELS_TIMER, ELS_TIMER_RELOAD_MAX);
  else
    timer_set_period(ELS_TIMER, res);

  timer_enable_counter(ELS_TIMER);
  timer_enable_irq(ELS_TIMER, TIM_DIER_UIE);
}
