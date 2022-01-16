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
#include "utils.h"

#define ELS_Z_TIMER                   TIM4
#define ELS_Z_TIMER_IRQ               NVIC_TIM4_IRQ
#define ELS_Z_TIMER_RCC               RCC_TIM4
#define ELS_Z_TIMER_RST               RST_TIM4

#define ELS_X_TIMER                   TIM7
#define ELS_X_TIMER_IRQ               NVIC_TIM7_IRQ
#define ELS_X_TIMER_RCC               RCC_TIM7
#define ELS_X_TIMER_RST               RST_TIM7

#define ELS_SET_ZDIR_LR               els_gpio_set(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)
#define ELS_SET_ZDIR_RL               els_gpio_clear(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)

#define ELS_Z_BACKLASH_FIX            do {                                                                 \
                                        if (stepper.zdir != 0 && els_config->z_backlash_pulses) {          \
                                          els_printf("z backlash compensation\n");                         \
                                          for (size_t _n = 0; _n < els_config->z_backlash_pulses; _n++) {  \
                                            els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);                   \
                                            els_delay_microseconds(ELS_BACKLASH_DELAY_US);                 \
                                            els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);                 \
                                            els_delay_microseconds(ELS_BACKLASH_DELAY_US);                 \
                                          }                                                                \
                                        }                                                                  \
                                      } while (0)


#define ELS_SET_XDIR_BT               els_gpio_set(ELS_X_DIR_PORT, ELS_X_DIR_PIN)
#define ELS_SET_XDIR_TB               els_gpio_clear(ELS_X_DIR_PORT, ELS_X_DIR_PIN)

#define ELS_X_BACKLASH_FIX            do {                                                                 \
                                        if (stepper.xdir != 0 && els_config->x_backlash_pulses) {          \
                                          els_printf("x backlash compensation\n");                         \
                                          for (size_t _n = 0; _n < els_config->x_backlash_pulses; _n++) {  \
                                            els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);                   \
                                            els_delay_microseconds(ELS_BACKLASH_DELAY_US);                 \
                                            els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);                 \
                                            els_delay_microseconds(ELS_BACKLASH_DELAY_US);                 \
                                          }                                                                \
                                        }                                                                  \
                                      } while (0)

#define ELS_TIMER_RELOAD_MAX          600
#define ELS_TIMER_ACCEL_STEPS         40

#define ELS_TIMER_ACCEL_X             1.05
#define ELS_TIMER_DECEL_X             1.5

#define ELS_TIMER_ACCEL_Z             1.05
#define ELS_TIMER_DECEL_Z             1.5

#define ELS_TIMER_ACCEL_DISABLED      0

//==============================================================================
// Internal State
//==============================================================================

typedef enum {
  STEPPER_OP_NA,
  STEPPER_OP_LINE_X,
  STEPPER_OP_LINE_Z,
  STEPPER_OP_LINE_XZ,
  STEPPER_OP_LINE_ZX,
  STEPPER_OP_ARC_CW,
  STEPPER_OP_ARC_CCW
} stepper_op_t;

static struct {
  els_stepper_t pos;

  int8_t xdir;
  volatile int32_t xsteps;
  double xdelta;
  uint32_t xfeed_um;
  stepper_op_t xop;

  // linear xz movement
  int32_t xz_line_n;
  int32_t xz_line_d;
  int32_t xz_line_err;

  int8_t zdir;
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
  double cw_circle_length;
  double cw_circle_zpos;

  // ccw arc movement
  double ccw_circle_radius;
  double ccw_circle_length;
  double ccw_circle_zpos;

  uint32_t xreload_target;
  uint32_t zreload_target;
  uint32_t xreload_updated_at;
  uint32_t zreload_updated_at;
} stepper;

//==============================================================================
// Exported constant
//==============================================================================
const els_stepper_t *els_stepper = &stepper.pos;

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
static void els_stepper_timer_z_isr(void);
static void els_stepper_timer_x_isr(void);
static void els_stepper_timer_x_update(uint32_t feed_um, bool accel);
static void els_stepper_timer_z_update(uint32_t feed_um, bool accel);

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
  els_nvic_irq_set_handler(ELS_X_TIMER_IRQ, els_stepper_timer_x_isr);
  els_nvic_irq_set_handler(ELS_Z_TIMER_IRQ, els_stepper_timer_z_isr);

  els_stepper_enable_z();
  els_stepper_enable_x();

  // default
  timer_set_period(ELS_Z_TIMER, ELS_TIMER_RELOAD_MAX);
  timer_set_period(ELS_X_TIMER, ELS_TIMER_RELOAD_MAX);

  els_stepper_timer_start();
}

void els_stepper_stop(void) {
  els_stepper_timer_stop();
  els_stepper_disable_z();
  els_stepper_disable_x();
}

void els_stepper_move_x(double mm, double speed_mm_s) {
  if (stepper.pos.xbusy) {
    if (stepper.xop != STEPPER_OP_LINE_X)
      return;

    int8_t dir = (mm > 0 ? 1 : -1);
    if (speed_mm_s * 1000 != stepper.xfeed_um || dir != stepper.xdir)
      return;
  }

  stepper.xop = STEPPER_OP_LINE_X;
  els_stepper_timer_x_update(speed_mm_s * 1000, true);
  if (mm < 0) {
    ELS_SET_XDIR_BT;
    if (stepper.xdir != -1) {
      stepper.xdir = -1;
      ELS_X_BACKLASH_FIX;
    }
    stepper.pos.xbusy = true;
    stepper.xsteps += (int32_t)(-mm * els_config->z_pulses_per_mm);
  }
  else if (mm > 0) {
    ELS_SET_XDIR_TB;
    if (stepper.xdir != 1) {
      stepper.xdir = 1;
      ELS_X_BACKLASH_FIX;
    }
    stepper.pos.xbusy = true;
    stepper.xsteps += (int32_t)(mm * els_config->x_pulses_per_mm);
  }
}

void els_stepper_move_z(double mm, double speed_mm_s) {
  if (stepper.pos.zbusy) {
    if (stepper.zop != STEPPER_OP_LINE_Z)
      return;

    int8_t dir = (mm > 0 ? 1 : -1);
    if (speed_mm_s * 1000 != stepper.zfeed_um || dir != stepper.zdir)
      return;
  }

  stepper.zop = STEPPER_OP_LINE_Z;
  els_stepper_timer_z_update(speed_mm_s * 1000, true);
  if (mm < 0) {
    ELS_SET_ZDIR_RL;
    if (stepper.zdir != -1) {
      stepper.zdir = -1;
      ELS_Z_BACKLASH_FIX;
    }
    stepper.pos.zbusy = true;
    stepper.zsteps += (int32_t)(-mm * els_config->z_pulses_per_mm);
  }
  else if (mm > 0) {
    ELS_SET_ZDIR_LR;
    if (stepper.zdir != 1) {
      stepper.zdir = 1;
      ELS_Z_BACKLASH_FIX;
    }
    stepper.pos.zbusy = true;
    stepper.zsteps += (int32_t)(mm * els_config->z_pulses_per_mm);
  }
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
    if (stepper.xdir != -1) {
      stepper.xdir = -1;
      ELS_X_BACKLASH_FIX;
    }
  }
  else {
    stepper.xsteps = (x_mm * els_config->x_pulses_per_mm);
    ELS_SET_XDIR_TB;
    if (stepper.xdir != 1) {
      stepper.xdir = 1;
      ELS_X_BACKLASH_FIX;
    }
  }

  if (z_mm < 0) {
    stepper.zsteps = (-z_mm * els_config->z_pulses_per_mm);
    ELS_SET_ZDIR_RL;
    if (stepper.zdir != -1) {
      stepper.zdir = -1;
      ELS_Z_BACKLASH_FIX;
    }
  }
  else {
    stepper.zsteps = (z_mm * els_config->z_pulses_per_mm);
    ELS_SET_ZDIR_LR;
    if (stepper.zdir != 1) {
      stepper.zdir = 1;
      ELS_Z_BACKLASH_FIX;
    }
  }

  uint32_t gcd = els_gcd(stepper.xsteps, stepper.zsteps);
  if (stepper.zsteps > stepper.xsteps) {
    stepper.zx_line_n = stepper.xsteps / gcd;
    stepper.zx_line_d = stepper.zsteps / gcd;
    stepper.zx_line_err = 0;
    stepper.zop = STEPPER_OP_LINE_ZX;
    els_stepper_timer_z_update(speed_mm_s * 1000, false);
  }
  else {
    stepper.xz_line_n = stepper.zsteps / gcd;
    stepper.xz_line_d = stepper.xsteps / gcd;
    stepper.xz_line_err = 0;
    stepper.xop = STEPPER_OP_LINE_XZ;
    els_stepper_timer_x_update(speed_mm_s * 1000, false);
  }
}

// Assumptions:
//
// Circular Arc Origin
//
// (z, x) = (0, -r)
//
// Circle Equation
//
// z**2 + (x + r)**2 = r**2
//
// Movement
//
// Arc starts at x = xd and finishes at x = 0 and the motion travel is clockwise.
//
void els_stepper_move_cw_arc(double radius, double xd, double speed_mm_s) {
  if (stepper.pos.xbusy || stepper.pos.zbusy || xd <= 0)
    return;

  els_stepper_timer_x_update(speed_mm_s * 1000, false);
  ELS_SET_XDIR_TB;
  if (stepper.xdir != 1) {
    stepper.xdir = 1;
    ELS_X_BACKLASH_FIX;
  }

  ELS_SET_ZDIR_RL;
  if (stepper.zdir != -1) {
    stepper.zdir = -1;
    ELS_Z_BACKLASH_FIX;
  }

  stepper.pos.xbusy = true;
  stepper.pos.zbusy = true;
  stepper.cw_circle_radius = radius;
  stepper.cw_circle_length = sqrt(SQR(radius) - SQR(radius - xd));
  stepper.cw_circle_zpos = stepper.pos.zpos;
  stepper.xsteps = xd * els_config->x_pulses_per_mm;
  stepper.xop = STEPPER_OP_ARC_CW;
}

// Assumptions:
//
// Circular Arc Origin
//
// (z, x) = (0, -r)
//
// Circle Equation
//
// z**2 + (x + r)**2 = r**2
//
// Movement
//
// Arc starts at x = xd and finishes at x = 0 and the motion travel is anti-clockwise.
//
void els_stepper_move_ccw_arc(double radius, double xd, double speed_mm_s) {
  if (stepper.pos.xbusy || stepper.pos.zbusy || xd <= 0)
    return;

  els_stepper_timer_x_update(speed_mm_s * 1000, false);
  ELS_SET_XDIR_TB;
  if (stepper.xdir != 1) {
    stepper.xdir = 1;
    ELS_X_BACKLASH_FIX;
  }

  ELS_SET_ZDIR_LR;
  if (stepper.zdir != 1) {
    stepper.zdir = 1;
    ELS_Z_BACKLASH_FIX;
  }

  stepper.pos.xbusy = true;
  stepper.pos.zbusy = true;
  stepper.ccw_circle_radius = radius;
  stepper.ccw_circle_length = sqrt(SQR(radius) - SQR(radius - xd));
  stepper.ccw_circle_zpos = stepper.pos.zpos;
  stepper.xsteps = xd * els_config->x_pulses_per_mm;
  stepper.xop = STEPPER_OP_ARC_CCW;
}

void els_stepper_zero_x(void) {
  stepper.pos.xpos = 0;
}

void els_stepper_zero_z(void) {
  stepper.pos.zpos = 0;
}


//==============================================================================
// Stepper path control
//==============================================================================
static void els_stepper_timer_z_line(void) {
  static volatile bool z_pul_state = 0;

  if (stepper.zsteps > 0) {
    if (z_pul_state)
      els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    else
      els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);

    if (z_pul_state) {
      stepper.pos.zpos += (stepper.zdir * stepper.zdelta);
      stepper.zsteps--;

      uint32_t now = els_timer_elapsed_milliseconds();
      uint32_t elapsed = MAX((now - stepper.zreload_updated_at) / 10, 5);
      if (stepper.zsteps > ELS_TIMER_ACCEL_STEPS && TIM_ARR(ELS_Z_TIMER) != stepper.zreload_target) {
        stepper.zreload_updated_at = now;
        if (TIM_ARR(ELS_Z_TIMER) > stepper.zreload_target)
          TIM_ARR(ELS_Z_TIMER) = MAX(TIM_ARR(ELS_Z_TIMER) - ELS_TIMER_ACCEL_Z * elapsed, stepper.zreload_target);
        else
          TIM_ARR(ELS_Z_TIMER) = MIN(TIM_ARR(ELS_Z_TIMER) + ELS_TIMER_ACCEL_Z * elapsed, stepper.zreload_target);
      }
      else if (stepper.zsteps <= ELS_TIMER_ACCEL_STEPS) {
        stepper.zreload_updated_at = now;
        TIM_ARR(ELS_Z_TIMER) = MIN(TIM_ARR(ELS_Z_TIMER) + ELS_TIMER_DECEL_Z * elapsed, ELS_TIMER_RELOAD_MAX);
      }
    }

    z_pul_state = !z_pul_state;
  }
  else {
    stepper.pos.zbusy = false;
    TIM_ARR(ELS_Z_TIMER) = ELS_TIMER_RELOAD_MAX;
  }
}

static void els_stepper_timer_x_line(void) {
  static volatile bool x_pul_state = 0;

  if (stepper.xsteps > 0) {
    if (x_pul_state)
      els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    else
      els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);

    if (x_pul_state) {
      stepper.pos.xpos += (stepper.xdir * stepper.xdelta);
      stepper.xsteps--;

      uint32_t now = els_timer_elapsed_milliseconds();
      uint32_t elapsed = MAX((now - stepper.xreload_updated_at) / 10, 5);
      if (stepper.xsteps > ELS_TIMER_ACCEL_STEPS && TIM_ARR(ELS_X_TIMER) != stepper.xreload_target) {
        stepper.xreload_updated_at = now;
        if (TIM_ARR(ELS_X_TIMER) > stepper.xreload_target)
          TIM_ARR(ELS_X_TIMER) = MAX(TIM_ARR(ELS_X_TIMER) - ELS_TIMER_ACCEL_X * elapsed, stepper.xreload_target);
        else
          TIM_ARR(ELS_X_TIMER) = MIN(TIM_ARR(ELS_X_TIMER) + ELS_TIMER_ACCEL_X * elapsed, stepper.xreload_target);
      }
      else if (stepper.xsteps <= ELS_TIMER_ACCEL_STEPS) {
        stepper.xreload_updated_at = now;
        TIM_ARR(ELS_X_TIMER) = MIN(TIM_ARR(ELS_X_TIMER) + ELS_TIMER_DECEL_X * elapsed, ELS_TIMER_RELOAD_MAX);
      }
    }

    x_pul_state = !x_pul_state;
  }
  else {
    stepper.pos.xbusy = false;
    TIM_ARR(ELS_X_TIMER) = ELS_TIMER_RELOAD_MAX;
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
      stepper.pos.zpos += (stepper.zdir * stepper.zdelta);
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
        stepper.pos.xpos += (stepper.xdir * stepper.xdelta);
        stepper.xsteps--;
      }

      x_pul_state = !x_pul_state;
    }
  }

  if (stepper.xsteps == 0 && stepper.zsteps == 0)
    stepper.pos.xbusy = stepper.pos.zbusy = false;
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
      stepper.pos.xpos += (stepper.xdir * stepper.xdelta);
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
        stepper.pos.zpos += (stepper.zdir * stepper.zdelta);
        stepper.zsteps--;
      }

      z_pul_state = !z_pul_state;
    }
  }

  if (stepper.xsteps == 0 && stepper.zsteps == 0) {
    stepper.pos.xbusy = stepper.pos.zbusy = false;
  }
}


static void els_stepper_timer_arc_cw(void) {
  static volatile bool z_pul_state = 0;
  static volatile bool x_pul_state = 0;

  if (z_pul_state) {
    els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    stepper.pos.zpos += (stepper.zdir * stepper.zdelta);
    z_pul_state = 0;
  }
  else if ((stepper.pos.zpos - stepper.cw_circle_zpos) >= stepper.zdelta) {
    els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    z_pul_state = 1;
  }
  else if (x_pul_state) {
    els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    x_pul_state = 0;

    stepper.pos.xpos += (stepper.xdir * stepper.xdelta);
    stepper.xsteps--;

    stepper.cw_circle_zpos = SQR(stepper.cw_circle_radius) - SQR(stepper.pos.xpos + stepper.cw_circle_radius);
    stepper.cw_circle_zpos = sqrt(fabs(stepper.cw_circle_zpos)) - stepper.cw_circle_length;
  }
  else if (stepper.xsteps > 0) {
    els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    x_pul_state = 1;
  }
  else {
    x_pul_state = z_pul_state = 0;
    stepper.pos.xbusy = stepper.pos.zbusy = false;
    stepper.xop = STEPPER_OP_NA;
  }
}

static void els_stepper_timer_arc_ccw(void) {
  static volatile bool z_pul_state = 0;
  static volatile bool x_pul_state = 0;

  if (z_pul_state) {
    els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    stepper.pos.zpos += (stepper.zdir * stepper.zdelta);
    z_pul_state = 0;
  }
  else if ((stepper.ccw_circle_zpos - stepper.pos.zpos) >= stepper.zdelta) {
    els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    z_pul_state = 1;
  }
  else if (x_pul_state) {
    els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    x_pul_state = 0;

    stepper.pos.xpos += (stepper.xdir * stepper.xdelta);
    stepper.xsteps--;

    stepper.ccw_circle_zpos = SQR(stepper.ccw_circle_radius) - SQR(stepper.pos.xpos + stepper.ccw_circle_radius);
    stepper.ccw_circle_zpos = stepper.ccw_circle_length - sqrt(fabs(stepper.ccw_circle_zpos));
  }
  else if (stepper.xsteps > 0) {
    els_gpio_set(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
    x_pul_state = 1;
  }
  else {
    x_pul_state = z_pul_state = 0;
    stepper.pos.xbusy = stepper.pos.zbusy = false;
    stepper.xop = STEPPER_OP_NA;
  }
}

//==============================================================================
// ISR
//==============================================================================
static void els_stepper_timer_z_isr(void) {
  TIM_SR(ELS_Z_TIMER) &= ~TIM_SR_UIF;

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
}

static void els_stepper_timer_x_isr(void) {
  TIM_SR(ELS_X_TIMER) &= ~TIM_SR_UIF;

  switch (stepper.xop) {
    case STEPPER_OP_LINE_X:
      els_stepper_timer_x_line();
      break;
    case STEPPER_OP_LINE_XZ:
      els_stepper_timer_xz_line();
      break;
    case STEPPER_OP_ARC_CW:
      els_stepper_timer_arc_cw();
      break;
    case STEPPER_OP_ARC_CCW:
      els_stepper_timer_arc_ccw();
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
  //----------------- Z Axis --------------------------------------------------------
  rcc_periph_clock_enable(ELS_Z_TIMER_RCC);
  rcc_periph_reset_pulse(ELS_Z_TIMER_RST);

  // clock division 0, alignment edge, count up.
  timer_set_mode(ELS_Z_TIMER, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  // 10us counter, ~100KHz
  timer_set_prescaler(ELS_Z_TIMER, ((rcc_apb1_frequency * 2) / 100e3) - 1);

  // disable preload
  timer_disable_preload(ELS_Z_TIMER);
  timer_continuous_mode(ELS_Z_TIMER);

  nvic_set_priority(ELS_Z_TIMER_IRQ, 3);
  nvic_enable_irq(ELS_Z_TIMER_IRQ);
  timer_enable_update_event(ELS_Z_TIMER);

  //----------------- X Axis ---------------------------------------------------------
  rcc_periph_clock_enable(ELS_X_TIMER_RCC);
  rcc_periph_reset_pulse(ELS_X_TIMER_RST);

  // clock division 0, alignment edge, count up.
  timer_set_mode(ELS_X_TIMER, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  // 10us counter, ~100KHz
  timer_set_prescaler(ELS_X_TIMER, ((rcc_apb1_frequency * 2) / 100e3) - 1);

  // disable preload
  timer_disable_preload(ELS_X_TIMER);
  timer_continuous_mode(ELS_X_TIMER);

  nvic_set_priority(ELS_X_TIMER_IRQ, 4);
  nvic_enable_irq(ELS_X_TIMER_IRQ);
  timer_enable_update_event(ELS_X_TIMER);
}

static void els_stepper_timer_start(void) {
  timer_enable_counter(ELS_Z_TIMER);
  timer_enable_irq(ELS_Z_TIMER, TIM_DIER_UIE);

  timer_enable_counter(ELS_X_TIMER);
  timer_enable_irq(ELS_X_TIMER, TIM_DIER_UIE);
}

static void els_stepper_timer_stop(void) {
  timer_disable_irq(ELS_Z_TIMER, TIM_DIER_UIE);
  timer_disable_counter(ELS_Z_TIMER);

  timer_disable_irq(ELS_X_TIMER, TIM_DIER_UIE);
  timer_disable_counter(ELS_X_TIMER);
}

// Update timer frequency for required feed rate.
static void els_stepper_timer_z_update(uint32_t feed_um, bool accel) {
  uint32_t res;

  #if ELS_TIMER_ACCEL_DISABLED
    accel = false;
  #endif

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

  timer_disable_irq(ELS_Z_TIMER, TIM_DIER_UIE);
  timer_disable_counter(ELS_Z_TIMER);

  stepper.zreload_target = res;
  stepper.zreload_updated_at = els_timer_elapsed_milliseconds();

  if (!accel)
    timer_set_period(ELS_Z_TIMER, res);

  timer_enable_counter(ELS_Z_TIMER);
  timer_enable_irq(ELS_Z_TIMER, TIM_DIER_UIE);
}

// Update timer frequency for required feed rate.
static void els_stepper_timer_x_update(uint32_t feed_um, bool accel) {
  uint32_t res;

  #if ELS_TIMER_ACCEL_DISABLED
    accel = false;
  #endif

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

  timer_disable_irq(ELS_X_TIMER, TIM_DIER_UIE);
  timer_disable_counter(ELS_X_TIMER);

  stepper.xreload_target = res;
  stepper.xreload_updated_at = els_timer_elapsed_milliseconds();

  if (!accel)
    timer_set_period(ELS_X_TIMER, res);

  timer_enable_counter(ELS_X_TIMER);
  timer_enable_irq(ELS_X_TIMER, TIM_DIER_UIE);
}
