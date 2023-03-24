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

#include "tft/tft.h"
#include "tft/fonts/fonts.h"

#include "constants.h"

#include "config.h"
#include "dro.h"
#include "encoder.h"
#include "spindle.h"
#include "stepper.h"
#include "keypad.h"
#include "turning.h"
#include "utils.h"

#define ELS_TURNING_TIMER         TIM4
#define ELS_TURNING_TIMER_IRQ     NVIC_TIM4_IRQ
#define ELS_TURNING_TIMER_MAX     UINT16_MAX
#define ELS_TURNING_TIMER_RCC     RCC_TIM4
#define ELS_TURNING_TIMER_RST     RST_TIM4

#define PRECISION                 (1e-2)

//==============================================================================
// Externs
//==============================================================================
extern tft_device_t tft;

extern const tft_font_t noto_sans_mono_bold_26;
extern const tft_font_t noto_sans_mono_bold_arrows_24;
extern const tft_font_t gears_regular_32;
extern const tft_font_t gears_regular_50;

//==============================================================================
// Config
//==============================================================================
typedef enum {
  ELS_TURNING_IDLE       = 1,
  ELS_TURNING_PAUSED     = 2,
  ELS_TURNING_ACTIVE     = 4,
  ELS_TURNING_SET_FEED   = 8,
  ELS_TURNING_SET_ZAXES  = 16,
  ELS_TURNING_SET_XAXES  = 32,
  ELS_TURNING_ZLIM       = 64,
  ELS_TURNING_SET_LENGTH = 128

} els_turning_state_t;

#define ELS_TURNING_SET_ZDIR_LR          els_gpio_set(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)
#define ELS_TURNING_SET_ZDIR_RL          els_gpio_clear(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)

#define ELS_TURNING_TIMER_CHANGE(feed)   do {                                                              \
                                           if (els_turning.timer_feed_um != feed) {                        \
                                             els_turning_timer_update(feed);                               \
                                             els_turning_timer_start();                                    \
                                           }                                                               \
                                         } while (0)

//==============================================================================
// Internal state
//==============================================================================
#define ELS_TURNING_FEED_MIN  (100)
#define ELS_TURNING_FEED_MAX  (10000)

static struct {
  int32_t  timer_feed_um;
  bool     timer_running;

  int32_t  feed_um;
  bool     feed_reverse;

  bool     locked;

  // z-axis state & config
  double   length;
  double   zdelta;
  bool     zstop;

  // input read for jogging
  int32_t  encoder_pos;

  // encoder multiplier
  int16_t encoder_multiplier;

  // module state
  els_turning_state_t state;

  // tracking variables for display refresh.
  els_turning_state_t prev_state;
  els_spindle_direction_t prev_dir;

} els_turning = {
  .feed_um = 2000,
  .zstop = true,
  .length = 10,
  .encoder_multiplier = 1
};

//==============================================================================
// Internal functions
//==============================================================================
static void els_turning_run(void);
static void els_turning_display_refresh(void);
static void els_turning_display_feed(void);
static void els_turning_display_axes(void);
static void els_turning_display_header(void);
static void els_turning_display_encoder_pips(void);

static void els_turning_set_feed(void);

static void els_turning_set_zaxes(void);
static void els_turning_set_xaxes(void);

static void els_turning_handle_zlim(void);
static void els_turning_set_length(void);

static void els_turning_configure_timer(void);

static void els_turning_timer_update(int32_t feed_um);
static void els_turning_timer_start(void);
static void els_turning_timer_stop(void);
static void els_turning_timer_isr(void) __attribute__ ((interrupt ("IRQ")));

static void els_turning_keypad_process(void);

// jog
static void els_turning_zjog(void);

//==============================================================================
// API
//==============================================================================
void els_turning_setup(void) {
  els_turning_configure_timer();
}

void els_turning_start(void) {
  char text[32];

  tft_filled_rectangle(&tft, 0,   0, 480, 320, ILI9481_BLACK);

  tft_filled_rectangle(&tft, 0,  50, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0,  50, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 100, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 100, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 190, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 190, 480,   1, ILI9481_LITEGRAY);

  els_turning_display_header();

  // angle symbol
  tft_font_write_bg(&tft, 165,  62, "J", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // pulse symbol
  tft_font_write_bg(&tft, 355,  58, "K", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // reset isr
  els_nvic_irq_set_handler(ELS_TURNING_TIMER_IRQ, els_turning_timer_isr);

  // reset state
  els_turning.prev_state = 0;
  els_turning.prev_dir = 0;

  els_turning.state = ELS_TURNING_IDLE;
  els_turning_timer_update(els_turning.feed_um);
  els_turning_timer_start();

  els_turning_display_feed();

  tft_font_write_bg(&tft, 8, 190, "DRO", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
  els_turning_display_axes();

  els_turning_display_refresh();

  els_stepper_start();
  els_stepper_disable_x();
}

void els_turning_stop(void) {
  els_turning_timer_stop();
  els_stepper_stop();
}

void els_turning_update(void) {
  static uint64_t last_refreshed_at = 0;

  uint64_t elapsed = els_timer_elapsed_microseconds();

  bool kp_locked = els_keypad_locked();
  if (els_turning.locked != kp_locked) {
    els_turning.locked = kp_locked;
    els_turning_display_header();
  }

  if (els_turning.state & (ELS_TURNING_IDLE | ELS_TURNING_PAUSED | ELS_TURNING_ACTIVE))
    els_turning_keypad_process();

  switch (els_turning.state) {
    case ELS_TURNING_PAUSED:
    case ELS_TURNING_ACTIVE:
      els_turning_run();
      break;
    case ELS_TURNING_SET_FEED:
      els_turning_set_feed();
      break;
    case ELS_TURNING_SET_LENGTH:
      els_turning_set_length();
      break;
    case ELS_TURNING_ZLIM:
      els_turning_handle_zlim();
      break;
    case ELS_TURNING_SET_ZAXES:
      els_turning_set_zaxes();
      break;
    case ELS_TURNING_SET_XAXES:
      els_turning_set_xaxes();
      break;
    default:
      break;
  }

  if (elapsed - last_refreshed_at > 1e5) {
    last_refreshed_at = elapsed;
    els_turning_display_refresh();
  }

  int16_t em = els_encoder_get_multiplier();
  if (em != els_turning.encoder_multiplier) {
    els_turning.encoder_multiplier = em;
    els_turning_display_encoder_pips();
  }
}

//==============================================================================
// ISR
//==============================================================================
static volatile bool z_pul_state = 0;
static void els_turning_timer_isr(void) {
  TIM_SR(ELS_TURNING_TIMER) &= ~TIM_SR_UIF;

  if (els_turning.state == ELS_TURNING_ACTIVE) {
    if (z_pul_state)
      els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    else
      els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);

    if (els_turning.zstop) {
      if (els_config->z_closed_loop)
        els_stepper->zpos = (double)els_dro.zpos_um * 1e-3;
      else if (z_pul_state)
        els_stepper->zpos += (els_stepper->zdir * els_turning.zdelta);

      if (els_stepper->zdir * els_stepper->zpos >= els_turning.length)
        els_turning.state = ELS_TURNING_ZLIM;
    }
    else if (els_config->z_closed_loop)  {
      els_stepper->zpos = (double)els_dro.zpos_um * 1e-3;
    }

    z_pul_state = !z_pul_state;
  }
}

//==============================================================================
// Internal functions
//==============================================================================

// ----------------------------------------------------------------------------------
// Display functions
// ----------------------------------------------------------------------------------
static void els_turning_display_feed(void) {
  char text[32];
  tft_rgb_t color = (els_turning.state == ELS_TURNING_SET_FEED ? ILI9481_YELLOW : ILI9481_WHITE);

  tft_filled_rectangle(&tft, 270, 228, 84, 25, ILI9481_BLACK);
  els_sprint_double2(text, sizeof(text), els_turning.feed_um / 1000.0, "FEED");
  tft_font_write_bg(&tft, 270, 228, text, &noto_sans_mono_bold_26, color, ILI9481_BLACK);
  if (els_turning.feed_reverse) {
    tft_font_write_bg(&tft, 270, 190, "LR", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
    tft_font_write_bg(&tft, 335, 185, "C", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  }
  else {
    tft_font_write_bg(&tft, 270, 190, "RL", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
    tft_font_write_bg(&tft, 335, 185, "A", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  }
}

static void els_turning_display_axes(void) {
  char text[32];
  tft_rgb_t color;

  els_sprint_double3(text, sizeof(text), els_stepper->zpos, "Z");
  color = (els_turning.state == ELS_TURNING_SET_ZAXES ? ILI9481_YELLOW : ILI9481_WHITE);
  tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, color, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_turning.length, "LEN");
  color = (els_turning.state == ELS_TURNING_SET_LENGTH ? ILI9481_YELLOW : ILI9481_WHITE);
  tft_font_write_bg(&tft, 270, 102, text, &noto_sans_mono_bold_26, color, ILI9481_BLACK);

  if (els_turning.zstop)
    tft_font_write_bg(&tft, 8, 142, "LIMITS ON ", &noto_sans_mono_bold_26, ILI9481_CERULEAN, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 142, "LIMITS OFF", &noto_sans_mono_bold_26, ILI9481_GRAY, ILI9481_BLACK);
}

static void els_turning_display_header(void) {
  tft_rgb_t color = els_turning.locked ? ILI9481_RED : ILI9481_CERULEAN;

  tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
  tft_font_write_bg(&tft, 8, 0, "MANUAL TURNING", &noto_sans_mono_bold_26, ILI9481_WHITE, color);
  els_turning_display_encoder_pips();
}

static void els_turning_display_encoder_pips(void) {
  // multiplier is 100, 10 or 1
  size_t pips = els_turning.encoder_multiplier;
  pips = pips > 10 ? 3 : pips > 1 ? 2 : 1;

  for (size_t n = 0, spacing = 0; n < 3; n++, spacing++)
    tft_filled_rectangle(&tft,
      440, 32 - (n * 10 + spacing * 2),
      15 + n * 10, 10,
      (n < pips ? ILI9481_WHITE : els_turning.locked ? ILI9481_LITEGRAY : ILI9481_BGCOLOR1));

}

static void els_turning_display_refresh(void) {
  els_spindle_direction_t dir = els_spindle_get_direction();
  if (dir != els_turning.prev_dir) {
    els_turning.prev_dir = dir;
    tft_filled_rectangle(&tft, 0, 55, 60, 45, ILI9481_BLACK);
    switch (dir) {
      case ELS_S_DIRECTION_CW:
        // cw symbol
        tft_font_write_bg(&tft, 5, 62, "A", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);
        break;
      case ELS_S_DIRECTION_CCW:
        // ccw symbol
        tft_font_write_bg(&tft, 5, 62, "B", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);
        break;
      default:
        // spindle symbol
        tft_font_write_bg(&tft, 3, 52, "N", &gears_regular_50, ILI9481_WHITE, ILI9481_BLACK);
        break;
    }
  }

  if (els_turning.prev_state != els_turning.state) {
    els_turning.prev_state = els_turning.state;
    tft_filled_rectangle(&tft, 270, 265, 170, 35, ILI9481_BLACK);
    const uint16_t x = 270, y = 262;
    switch (els_turning.state) {
      case ELS_TURNING_IDLE:
        tft_font_write_bg(&tft, x, y, "IDLE", &noto_sans_mono_bold_26, ILI9481_DIANNE, ILI9481_BLACK);
        break;
      case ELS_TURNING_PAUSED:
        tft_font_write_bg(&tft, x, y, "READY", &noto_sans_mono_bold_26, ILI9481_ORANGE, ILI9481_BLACK);
        break;
      case ELS_TURNING_ACTIVE:
        tft_font_write_bg(&tft, x, y, "ACTIVE", &noto_sans_mono_bold_26, ILI9481_CERULEAN, ILI9481_BLACK);
        break;
      case ELS_TURNING_SET_FEED:
        tft_font_write_bg(&tft, x, y, "SET FEED", &noto_sans_mono_bold_26, ILI9481_DIANNE, ILI9481_BLACK);
        break;
      case ELS_TURNING_SET_ZAXES:
        tft_font_write_bg(&tft, x, y, "Z AXIS", &noto_sans_mono_bold_26, ILI9481_DIANNE, ILI9481_BLACK);
        break;
      case ELS_TURNING_SET_XAXES:
        tft_font_write_bg(&tft, x, y, "X AXIS", &noto_sans_mono_bold_26, ILI9481_DIANNE, ILI9481_BLACK);
        break;
      case ELS_TURNING_ZLIM:
        tft_font_write_bg(&tft, x, y, "Z LIMIT", &noto_sans_mono_bold_26, ILI9481_RED, ILI9481_BLACK);
        break;
      case ELS_TURNING_SET_LENGTH:
        tft_font_write_bg(&tft, x, y, "SET LEN", &noto_sans_mono_bold_26, ILI9481_DIANNE, ILI9481_BLACK);
        break;
      default:
        break;
    }
  }

  char text[32];
  snprintf(text, sizeof(text), "%04d", els_spindle_get_rpm());
  tft_font_write_bg(&tft, 62, 52, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  double angle = els_spindle_get_angle();
  snprintf(text, sizeof(text), "%03d.%02d", (int)angle, (int)((angle - (int)angle) * 100));
  tft_font_write_bg(&tft, 200, 52, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  snprintf(text, sizeof(text), "%04d", els_spindle_get_counter());
  tft_font_write_bg(&tft, 396, 52, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), els_stepper->zpos, "Z");
  tft_rgb_t color = (els_turning.state == ELS_TURNING_SET_ZAXES ? ILI9481_YELLOW : ILI9481_WHITE);
  tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, color, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), (els_dro.zpos_um / 1000.0), "Z");
  tft_font_write_bg(&tft, 8, 228, text, &noto_sans_mono_bold_26,
    (els_turning.state == ELS_TURNING_SET_ZAXES) ? ILI9481_YELLOW : ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), (els_dro.xpos_um / 1000.0), "X");
  tft_font_write_bg(&tft, 8, 262, text, &noto_sans_mono_bold_26,
    (els_turning.state == ELS_TURNING_SET_XAXES) ? ILI9481_YELLOW : ILI9481_WHITE, ILI9481_BLACK);
}

// ----------------------------------------------------------------------------------
// Top module level keypad handler.
// ----------------------------------------------------------------------------------
static void els_turning_keypad_process(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
      if (els_turning.state == ELS_TURNING_IDLE) {
        els_turning.state = ELS_TURNING_PAUSED;
        els_turning_timer_update(els_turning.timer_feed_um);
        els_turning_timer_start();
      }
      break;
    case ELS_KEY_EXIT:
      if (els_turning.state & (ELS_TURNING_PAUSED | ELS_TURNING_ACTIVE)) {
        els_turning.state = ELS_TURNING_IDLE;
        els_turning_timer_stop();
      }
      break;
    case ELS_KEY_SET_FEED:
      els_turning.state = ELS_TURNING_SET_FEED;
      els_turning.encoder_pos = 0;
      els_turning_display_feed();
      els_encoder_reset();
      break;
    case ELS_KEY_SET_ZX:
      if (els_turning.state & (ELS_TURNING_IDLE | ELS_TURNING_PAUSED)) {
        els_turning.state = ELS_TURNING_SET_ZAXES;
        els_turning.encoder_pos = 0;
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F1:
      els_encoder_reset();
      els_turning.state = ELS_TURNING_SET_LENGTH;
      els_turning_display_axes();
      break;
    case ELS_KEY_FUN_F2:
      tft_filled_rectangle(&tft, 74, 142, 168, 40, ILI9481_BLACK);
      els_turning.zstop = !els_turning.zstop;
      els_turning_display_axes();
      break;
    default:
      break;
  }
}

// ---------------------------------------------------------------------------------------
// Function 1: primary turning handler, detects direction change and toggles the DIR pin.
// ---------------------------------------------------------------------------------------
static void els_turning_run(void) {
  int zdir = (els_turning.feed_reverse ? 1 : -1);

  switch (els_spindle_get_direction()) {
    case ELS_S_DIRECTION_CCW:
    case ELS_S_DIRECTION_CW:
      if (!els_stepper->zbusy) {
        if (els_stepper->zdir != zdir) {
          els_stepper->zdir = zdir;
          if (zdir == 1)
            ELS_TURNING_SET_ZDIR_LR;
          else
            ELS_TURNING_SET_ZDIR_RL;
          els_stepper_z_backlash_fix();
        }
      }
      if (els_turning.state == ELS_TURNING_PAUSED && els_spindle_get_counter() == 0)
        els_turning.state = ELS_TURNING_ACTIVE;
      break;
    default:
      if (els_turning.state == ELS_TURNING_ACTIVE)
        els_turning.state = ELS_TURNING_PAUSED;
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2: feed settings.
// ----------------------------------------------------------------------------------
static void els_turning_set_feed(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      ELS_TURNING_TIMER_CHANGE(els_turning.feed_um);
      els_turning.state = ELS_TURNING_IDLE;
      els_turning_display_feed();
      break;
    case ELS_KEY_REV_FEED:
      els_turning.feed_reverse = !els_turning.feed_reverse;
      els_turning_display_feed();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_turning.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_turning.encoder_pos) * 10 * els_turning.encoder_multiplier;
        if (els_turning.feed_um + delta <= ELS_TURNING_FEED_MIN)
          els_turning.feed_um = ELS_TURNING_FEED_MIN;
        else if (els_turning.feed_um + delta >= ELS_TURNING_FEED_MAX)
          els_turning.feed_um = ELS_TURNING_FEED_MAX;
        else
          els_turning.feed_um += delta;
        els_turning.encoder_pos = encoder_curr;
        els_turning_display_feed();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 3: Z Axis - limits & jogging
// ----------------------------------------------------------------------------------
static void els_turning_set_zaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_EXIT:
      els_turning.state = ELS_TURNING_IDLE;
      ELS_TURNING_TIMER_CHANGE(els_turning.feed_um);
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_stepper->zbusy)
        break;
      els_dro_zero_z();
      els_stepper_zero_z();
      els_turning_display_axes();
      break;
    case ELS_KEY_JOG_ZX_ORI:
      if (!els_stepper->zbusy) {
        if (els_config->z_closed_loop)
          els_stepper->zpos = (double)els_dro.zpos_um * 1e-3;
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);
      }
      break;
    case ELS_KEY_SET_ZX:
      els_turning.state = ELS_TURNING_SET_XAXES;
      els_turning_display_axes();
      break;
    default:
      els_turning_zjog();
      break;
  }
}

static void els_turning_set_xaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_turning.state = ELS_TURNING_IDLE;
      els_turning_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      els_dro_zero_x();
      els_turning_display_axes();
      break;
    case ELS_KEY_SET_ZX:
      els_encoder_reset();
      els_turning.state = ELS_TURNING_SET_ZAXES;
      els_turning_display_axes();
      break;
    default:
      break;
  }
}

static void els_turning_handle_zlim(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      if (!els_stepper->zbusy) {
        els_turning.state = ELS_TURNING_IDLE;
        els_turning_display_axes();
      }
      break;
    case ELS_KEY_JOG_ZX_ORI:
      if (!els_stepper->zbusy) {
        if (els_config->z_closed_loop)
          els_stepper->zpos = (double)els_dro.zpos_um * 1e-3;
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);
      }
      break;
    default:
      if (!els_stepper->zbusy && fabs(els_stepper->zpos) <= PRECISION)
        els_turning.state = ELS_TURNING_IDLE;
      break;
  }
}

static void els_turning_set_length(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_turning.state = ELS_TURNING_IDLE;
      els_turning_display_axes();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_turning.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_turning.encoder_pos);
        els_turning.length += (delta * 0.01 * els_turning.encoder_multiplier);
        els_turning.encoder_pos = encoder_curr;
        els_turning_display_axes();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Manual Jog
// ----------------------------------------------------------------------------------
static void els_turning_zjog(void) {
  double  delta, step;
  int32_t  encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_turning.encoder_pos != encoder_curr) {
    step = els_turning.encoder_multiplier == 1 ? 0.005 : 0.01 * els_turning.encoder_multiplier;
    delta = (encoder_curr - els_turning.encoder_pos) * step;
    els_turning.encoder_pos = encoder_curr;
    els_stepper_move_z(delta, els_config->z_jog_mm_s);
  }
}

// ----------------------------------------------------------------------------------
// Timer functions.
// ----------------------------------------------------------------------------------
//
// Clock setup.
static void els_turning_configure_timer(void) {
  rcc_periph_clock_enable(ELS_TURNING_TIMER_RCC);
  rcc_periph_reset_pulse(ELS_TURNING_TIMER_RST);

  // clock division 0, alignment edge, count up.
  timer_set_mode(ELS_TURNING_TIMER, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  // 10us counter, ~100KHz
  timer_set_prescaler(ELS_TURNING_TIMER, ((rcc_apb1_frequency * 2) / 100e3) - 1);

  // disable preload
  timer_disable_preload(ELS_TURNING_TIMER);
  timer_continuous_mode(ELS_TURNING_TIMER);

  nvic_set_priority(ELS_TURNING_TIMER_IRQ, 4);
  nvic_enable_irq(ELS_TURNING_TIMER_IRQ);
  timer_enable_update_event(ELS_TURNING_TIMER);
}

static void els_turning_timer_start(void) {
  if (els_turning.timer_running)
    return;

  els_turning.timer_running = true;
  timer_enable_counter(ELS_TURNING_TIMER);
  timer_enable_irq(ELS_TURNING_TIMER, TIM_DIER_UIE);
}

static void els_turning_timer_stop(void) {
  if (!els_turning.timer_running)
    return;

  els_turning.timer_running = false;
  timer_disable_irq(ELS_TURNING_TIMER, TIM_DIER_UIE);
  timer_disable_counter(ELS_TURNING_TIMER);
}

bool els_turning_busy(void) {
  return els_turning.state != ELS_TURNING_IDLE;
}

// Update timer frequency for required feed rate.
static void els_turning_timer_update(int32_t feed_um) {
  uint32_t res;

  // if (feed_um == els_turning.timer_feed_um)
  //   return;

  els_turning.timer_feed_um = feed_um;

  // Z-axis movement per pulse in mm.
  els_turning.zdelta = (1.0 / (double)els_config->z_pulses_per_mm);

  // Figure out how many pulses per second is required for the feed speed.
  // Ignore rounding errors, we don't need to be super accurate here.
  //
  res = (feed_um * els_config->z_pulses_per_mm) / 1000;
  // We need to clock at twice the rate to toggle the gpio.
  res = res * 2;

  // Clock's at 100Khz, figure out the period
  // TODO: check if period is 0 - 65535
  res = 100e3 / res;

  els_turning_timer_stop();
  timer_set_period(ELS_TURNING_TIMER, res);
}

