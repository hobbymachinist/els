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
#include "keypad.h"
#include "stepper.h"
#include "spindle.h"
#include "parting.h"
#include "utils.h"

#define PRECISION       (1e-2)

//==============================================================================
// Externs
//==============================================================================
extern tft_device_t tft;

extern const tft_font_t noto_sans_mono_bold_arrows_24;
extern const tft_font_t noto_sans_mono_bold_26;
extern const tft_font_t gears_regular_32;
extern const tft_font_t gears_regular_50;
extern const tft_font_t inconsolata_lgc_bold_14;

//==============================================================================
// Config
//==============================================================================
typedef enum {
  ELS_PARTING_IDLE      = 1,
  ELS_PARTING_PAUSED    = 2,
  ELS_PARTING_ACTIVE    = 4,
  ELS_PARTING_SET_ZAXES = 8,
  ELS_PARTING_ZJOG      = 16,
  ELS_PARTING_SET_XAXES = 32,
  ELS_PARTING_XJOG      = 64,
  ELS_PARTING_SET_FEED  = 128,
  ELS_PARTING_SET_RETR  = 256,
  ELS_PARTING_SET_DEPTH = 512
} els_parting_state_t;

typedef enum {
  ELS_PARTING_OP_IDLE    = 1,
  ELS_PARTING_OP_READY   = 2,
  ELS_PARTING_OP_MOVEZ0  = 3,
  ELS_PARTING_OP_MOVEX0  = 4,
  ELS_PARTING_OP_START   = 5,
  ELS_PARTING_OP_FEED_IN = 6,
  ELS_PARTING_OP_RETRACT = 7,
  ELS_PARTING_OP_AT_XR   = 8,
  ELS_PARTING_OP_AT_XP   = 9,
  ELS_PARTING_OP_DONE    = 10
} els_parting_op_state_t;

static const char *op_labels[] = {
  "N/A    ",
  "CONFIG ",
  "READY  ",
  "MOVE Z0",
  "MOVE X0",
  "ORIGIN ",
  "FEED IN",
  "RETRACT",
  "RETRACT",
  "RETURN ",
  "DONE   "
};

//==============================================================================
// Internal state
//==============================================================================
#define ELS_PARTING_FEED_MIN         (100)
#define ELS_PARTING_FEED_MAX         (2000)

#define ELS_PARTING_RETRACT_MM       (5)
#define ELS_PARTING_RETRACT_MM_MAX   (5)

static struct {
  int32_t  feed_um;
  double   feed_mm_s;

  // not supported yet.
  bool     feed_reverse;

  bool     locked;

  double   depth;
  double   retract_per_mm;

  // jogging state
  int32_t  encoder_pos;

  // encoder multiplier
  int16_t encoder_multiplier;

  // module state
  els_parting_state_t state;

  // tracking variables for display refresh.
  els_parting_op_state_t prev_op_state;
  els_parting_state_t prev_state;
  els_spindle_direction_t prev_dir;

  // operation state
  els_parting_op_state_t op_state;

  // saved xpos
  double xpos_prev;

  // dro
  bool show_dro;
} els_parting = {
  .xpos_prev = 0,
  .feed_um = 1000,
  .depth = 5.0,
  .retract_per_mm = 0,
  .encoder_multiplier = 1
};

//==============================================================================
// Internal functions
//==============================================================================
static void els_parting_run(void);
static void els_parting_turn(void);

static void els_parting_display_refresh(void);
static void els_parting_display_setting(void);
static void els_parting_display_axes(void);
static void els_parting_display_header(void);
static void els_parting_display_diagram(void);
static void els_parting_display_encoder_pips(void);

static void els_parting_set_feed(void);
static void els_parting_set_depth(void);
static void els_parting_set_retract(void);

static void els_parting_set_zaxes(void);
static void els_parting_set_xaxes(void);

static void els_parting_configure_gpio(void);
static void els_parting_configure_timer(void);

static void els_parting_keypad_process(void);

// manual jogging
static void els_parting_zjog(void);
static void els_parting_xjog(void);

//==============================================================================
// API
//==============================================================================
void els_parting_setup(void) {
  // NO-OP
}

void els_parting_start(void) {
  char text[32];

  tft_filled_rectangle(&tft, 0,   0, 480, 320, ILI9481_BLACK);

  tft_filled_rectangle(&tft, 0,  50, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0,  50, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 100, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 100, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 190, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 190, 480,   1, ILI9481_LITEGRAY);

  els_parting_display_header();

  // angle symbol
  tft_font_write_bg(&tft, 165,  62, "J", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // pulse symbol
  tft_font_write_bg(&tft, 355,  58, "K", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // reset state
  els_parting.feed_mm_s = els_parting.feed_um / 1000.0;
  els_parting.prev_op_state = 0;
  els_parting.prev_state = 0;
  els_parting.prev_dir = 0;
  els_parting.state = ELS_PARTING_IDLE;
  els_parting.op_state = ELS_PARTING_OP_IDLE;

  if (!els_parting.show_dro)
    els_parting_display_diagram();

  els_parting.state = ELS_PARTING_IDLE;

  els_parting_display_setting();
  els_parting_display_axes();
  els_parting_display_refresh();

  els_stepper_start();
}

void els_parting_stop(void) {
  els_stepper_stop();
}

bool els_parting_busy(void) {
  return els_parting.state != ELS_PARTING_IDLE;
}

void els_parting_update(void) {
  static uint64_t last_refreshed_at = 0;

  uint64_t elapsed = els_timer_elapsed_microseconds();

  bool kp_locked = els_keypad_locked();
  if (els_parting.locked != kp_locked) {
    els_parting.locked = kp_locked;
    els_parting_display_header();
  }

  if (els_parting.state & (ELS_PARTING_IDLE | ELS_PARTING_PAUSED | ELS_PARTING_ACTIVE))
    els_parting_keypad_process();

  switch (els_parting.state) {
    case ELS_PARTING_PAUSED:
    case ELS_PARTING_ACTIVE:
      els_parting_run();
      break;
    case ELS_PARTING_SET_FEED:
      els_parting_set_feed();
      break;
    case ELS_PARTING_SET_RETR:
      els_parting_set_retract();
      break;
    case ELS_PARTING_SET_DEPTH:
      els_parting_set_depth();
      break;
    default:
      if (els_parting.state & (ELS_PARTING_SET_ZAXES | ELS_PARTING_ZJOG))
        els_parting_set_zaxes();
      else if (els_parting.state & (ELS_PARTING_SET_XAXES | ELS_PARTING_XJOG))
        els_parting_set_xaxes();
      break;
  }

  if (elapsed - last_refreshed_at > 1e5) {
    last_refreshed_at = elapsed;
    els_parting_display_refresh();
  }

  int16_t em = els_encoder_get_multiplier();
  if (em != els_parting.encoder_multiplier) {
    els_parting.encoder_multiplier = em;
    els_parting_display_encoder_pips();
  }
}

//==============================================================================
// Internal functions
//==============================================================================

// ----------------------------------------------------------------------------------
// Display functions
// ----------------------------------------------------------------------------------
static void els_parting_display_setting(void) {
  char text[32];

  els_sprint_double2(text, sizeof(text), els_parting.feed_um / 1000.0, "Xf");
  if (els_parting.state == ELS_PARTING_SET_FEED)
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double2(text, sizeof(text), els_parting.retract_per_mm, "Rx");
  if (els_parting.state == ELS_PARTING_SET_RETR)
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_parting.depth, "D");
  if (els_parting.state == ELS_PARTING_SET_DEPTH)
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
}

static void els_parting_display_axes(void) {
  char text[32];

  els_sprint_double33(text, sizeof(text), els_stepper->zpos, "Z");

  if (els_parting.state & ELS_PARTING_SET_ZAXES)
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), els_stepper->xpos, "X");
  if (els_parting.state & ELS_PARTING_SET_XAXES)
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  if (els_parting.show_dro) {
    tft_font_write_bg(&tft, 8, 190, "DRO", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.zpos_um / 1000.0), "Z");
    tft_font_write_bg(&tft, 8, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.xpos_um / 1000.0), "X");
    tft_font_write_bg(&tft, 8, 262, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
  }
}

static void els_parting_display_diagram(void) {
  // ----------------------------------------------------------------------------------------------
  // diagram
  // ----------------------------------------------------------------------------------------------
  tft_filled_rectangle(&tft, 100, 220, 156, 80, ILI9481_WHITE);
  tft_filled_rectangle(&tft, 240, 220,  16, 80, ILI9481_ORANGE);
  tft_filled_rectangle(&tft, 80,  258, 200,  4, ILI9481_LITEGRAY);

  tft_font_write_bg(&tft, 260, 184, "D", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 265, 226, "D", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 260, 250, "B", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);

  // origin
  tft_font_write(&tft, 230, 265, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW);

  // legend
  tft_font_write_bg(&tft, 8, 200, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  tft_font_write_bg(&tft, 26, 220, "(0,0)", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  // ----------------------------------------------------------------------------------------------
}

static void els_parting_display_header(void) {
  tft_rgb_t color = (els_parting.locked ? ILI9481_RED : ILI9481_CERULEAN);

  tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
  tft_font_write_bg(&tft, 8, 0, "PARTING", &noto_sans_mono_bold_26, ILI9481_WHITE, color);
  els_parting_display_encoder_pips();
}

static void els_parting_display_encoder_pips(void) {
  // multiplier is 100, 10 or 1
  size_t pips = els_parting.encoder_multiplier;
  pips = pips > 10 ? 3 : pips > 1 ? 2 : 1;

  for (size_t n = 0, spacing = 0; n < 3; n++, spacing++)
    tft_filled_rectangle(&tft,
      440, 32 - (n * 10 + spacing * 2),
      15 + n * 10, 10,
      (n < pips ? ILI9481_WHITE : els_parting.locked ? ILI9481_LITEGRAY : ILI9481_BGCOLOR1));
}

static void els_parting_display_refresh(void) {
  els_spindle_direction_t dir = els_spindle_get_direction();
  if (dir != els_parting.prev_dir) {
    els_parting.prev_dir = dir;
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

  char text[32];
  snprintf(text, sizeof(text), "%04d", els_spindle_get_rpm());
  tft_font_write_bg(&tft, 62, 52, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  double angle = els_spindle_get_angle();
  snprintf(text, sizeof(text), "%03d.%02d", (int)angle, (int)((angle - (int)angle) * 100));
  tft_font_write_bg(&tft, 200, 52, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  snprintf(text, sizeof(text), "%04d", els_spindle_get_counter());
  tft_font_write_bg(&tft, 396, 52, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_parting_display_axes();

  if (els_parting.op_state != els_parting.prev_op_state) {
    els_parting.prev_op_state = els_parting.op_state;

    const char *label = op_labels[els_parting.op_state];
    tft_filled_rectangle(&tft, 310, 195, 169, 35, ILI9481_BLACK);
    tft_font_write_bg(&tft, 310, 190, label, &noto_sans_mono_bold_26, ILI9481_CERULEAN, ILI9481_BLACK);
  }
}

// ----------------------------------------------------------------------------------
// Top module level keypad handler.
// ----------------------------------------------------------------------------------
static void els_parting_keypad_process(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
      if (els_parting.state == ELS_PARTING_IDLE) {
        if (els_parting.depth > 0) {
          els_parting.state = ELS_PARTING_PAUSED;
          els_parting.op_state = ELS_PARTING_OP_READY;
        }
        else {
          els_parting.state = ELS_PARTING_SET_DEPTH;
          els_parting_display_setting();
        }
      }
      break;
    case ELS_KEY_EXIT:
      if (els_parting.state & (ELS_PARTING_PAUSED | ELS_PARTING_ACTIVE)) {
        els_parting.state = ELS_PARTING_IDLE;
        els_parting.op_state = ELS_PARTING_OP_IDLE;
      }
      break;
    case ELS_KEY_SET_FEED:
      if (els_parting.state & (ELS_PARTING_IDLE | ELS_PARTING_PAUSED)) {
        els_parting.state = ELS_PARTING_SET_FEED;
        els_parting.encoder_pos = 0;
        els_parting_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F1:
      if (els_parting.state & (ELS_PARTING_IDLE | ELS_PARTING_PAUSED)) {
        els_parting.state = ELS_PARTING_SET_DEPTH;
        els_parting.encoder_pos = 0;
        els_parting_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F2:
      els_parting.show_dro = !els_parting.show_dro;
      tft_filled_rectangle(&tft, 0, 200, 300, 120, ILI9481_BLACK);
      if (els_parting.show_dro)
        els_parting_display_axes();
      else
        els_parting_display_diagram();
      break;
    case ELS_KEY_SET_ZX:
      if (els_parting.state & (ELS_PARTING_IDLE | ELS_PARTING_PAUSED)) {
        els_parting.state = ELS_PARTING_SET_ZAXES;
        els_parting.encoder_pos = 0;
        els_parting_display_axes();
        els_encoder_reset();
      }
      break;
    default:
      break;
  }
}

// ---------------------------------------------------------------------------------------
// Function 1: primary turning handler, detects direction change and toggles the DIR pin.
// ---------------------------------------------------------------------------------------
static void els_parting_run(void) {
  switch (els_spindle_get_direction()) {
    case ELS_S_DIRECTION_CW:
    case ELS_S_DIRECTION_CCW:
      if (els_parting.state == ELS_PARTING_PAUSED) {
        if (!els_stepper->zbusy && !els_stepper->xbusy && els_spindle_get_counter() == 0)
          els_parting.state = ELS_PARTING_ACTIVE;
      }
      break;
    default:
      if (els_parting.state == ELS_PARTING_ACTIVE)
        els_parting.state = ELS_PARTING_PAUSED;
      break;
  }

  if (els_parting.state == ELS_PARTING_ACTIVE)
    els_parting_turn();
}

static void els_parting_turn(void) {
  double xd;

  switch (els_parting.op_state) {
    case ELS_PARTING_OP_IDLE:
      break;
    case ELS_PARTING_OP_READY:
      els_parting.op_state = ELS_PARTING_OP_MOVEZ0;
      els_stepper_sync();
      break;
    case ELS_PARTING_OP_MOVEZ0:
      if (els_stepper->zbusy)
        break;
      if (fabs(els_stepper->zpos) > PRECISION) {
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_retract_jog_mm_s);
      }
      else
        els_parting.op_state = ELS_PARTING_OP_MOVEX0;
      break;
    case ELS_PARTING_OP_MOVEX0:
      if (els_stepper->xbusy)
        break;
      if (fabs(els_stepper->xpos) > PRECISION)
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_retract_jog_mm_s);
      else
        els_parting.op_state = ELS_PARTING_OP_START;
      break;
    case ELS_PARTING_OP_START:
      // initial move.
      els_parting.op_state = ELS_PARTING_OP_FEED_IN;
      els_parting.xpos_prev = els_stepper->xpos;
      break;
    case ELS_PARTING_OP_FEED_IN:
      if (els_stepper->xbusy)
        break;

      xd = els_parting.depth + els_stepper->xpos;
      if (xd >= PRECISION) {
        if (els_parting.retract_per_mm > 0) {
          if ((els_parting.xpos_prev - els_stepper->xpos) >= (els_parting.retract_per_mm - PRECISION)) {
            els_parting.xpos_prev = els_stepper->xpos;
            els_stepper_move_x(ELS_PARTING_RETRACT_MM, els_config->x_retract_jog_mm_s);
            els_parting.op_state = ELS_PARTING_OP_RETRACT;
          }
          else {
            xd = MIN(els_parting.retract_per_mm, els_parting.depth + els_stepper->xpos);
            if (xd >= PRECISION) {
              els_stepper_move_x(-xd, els_parting.feed_mm_s);
            }
            else {
              els_stepper_move_x(0 - els_stepper->xpos, els_config->x_retract_jog_mm_s);
              els_parting.op_state = ELS_PARTING_OP_DONE;
            }
          }
        }
        else {
          els_stepper_move_x(-xd, els_parting.feed_mm_s);
        }
      }
      else {
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_retract_jog_mm_s);
        els_parting.op_state = ELS_PARTING_OP_DONE;
      }
      break;
    case ELS_PARTING_OP_RETRACT:
      if (!els_stepper->xbusy)
        els_parting.op_state = ELS_PARTING_OP_AT_XR;
      break;
    case ELS_PARTING_OP_AT_XR:
      els_stepper_move_x(els_parting.xpos_prev - els_stepper->xpos, els_config->x_retract_jog_mm_s);
      els_parting.op_state = ELS_PARTING_OP_AT_XP;
      break;
    case ELS_PARTING_OP_AT_XP:
      if (!els_stepper->xbusy) {
        els_parting.xpos_prev = els_stepper->xpos;
        els_parting.op_state = ELS_PARTING_OP_FEED_IN;
      }
      break;
    case ELS_PARTING_OP_DONE:
      if (els_stepper->xbusy)
        break;

      // beer time
      els_parting.op_state = ELS_PARTING_OP_IDLE;
      els_parting.state = ELS_PARTING_IDLE;
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.1: feed settings.
// ----------------------------------------------------------------------------------
static void els_parting_set_feed(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_parting.state = ELS_PARTING_IDLE;
      els_parting_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_parting.state = ELS_PARTING_SET_RETR;
      els_parting_display_setting();
      break;
    case ELS_KEY_REV_FEED:
      els_parting_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_parting.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_parting.encoder_pos) * 10 * els_parting.encoder_multiplier;
        if (els_parting.feed_um + delta <= ELS_PARTING_FEED_MIN)
          els_parting.feed_um = ELS_PARTING_FEED_MIN;
        else if (els_parting.feed_um + delta >= ELS_PARTING_FEED_MAX)
          els_parting.feed_um = ELS_PARTING_FEED_MAX;
        else
          els_parting.feed_um += delta;
        els_parting.encoder_pos = encoder_curr;
        els_parting_display_setting();

        els_parting.feed_mm_s = els_parting.feed_um / 1000.0;
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.2: retract settings
// ----------------------------------------------------------------------------------
static void els_parting_set_retract(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_parting.state = ELS_PARTING_IDLE;
      els_parting_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_parting.state = ELS_PARTING_SET_FEED;
      els_parting_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_parting.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_parting.encoder_pos) * 0.01 * els_parting.encoder_multiplier;
        if (els_parting.retract_per_mm + delta <= 0)
          els_parting.retract_per_mm = 0;
        else if ((els_parting.retract_per_mm + delta) >= ELS_PARTING_RETRACT_MM_MAX)
          els_parting.retract_per_mm = ELS_PARTING_RETRACT_MM_MAX;
        else
          els_parting.retract_per_mm += delta;

        els_parting.encoder_pos = encoder_curr;
        els_parting_display_setting();
      }
      break;
  }
}

void els_parting_set_depth(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_parting.state = ELS_PARTING_IDLE;
      els_parting_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_parting.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_parting.encoder_pos) * 0.01 * els_parting.encoder_multiplier;
        if (els_parting.depth + delta <= 0)
          els_parting.depth = 0;
        else if (els_parting.depth + delta >= ELS_X_MAX_MM)
          els_parting.depth = ELS_X_MAX_MM;
        else
          els_parting.depth += delta;
        els_parting.encoder_pos = encoder_curr;
        els_parting_display_setting();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 3: Axis - position, origin & jogging
// ----------------------------------------------------------------------------------
static void els_parting_set_zaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_parting.state = (els_parting.state & ELS_PARTING_ZJOG) ? ELS_PARTING_SET_ZAXES : ELS_PARTING_IDLE;
      els_parting_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_parting.state == ELS_PARTING_SET_ZAXES) {
        els_stepper_zero_z();
        els_dro_zero_z();
        els_parting_display_axes();
      }
      break;
    case ELS_KEY_JOG_ZX_ORI:
      if (!els_stepper->zbusy)
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);
      break;
    case ELS_KEY_SET_ZX:
      els_parting.state = ELS_PARTING_SET_XAXES;
      els_parting_display_axes();
      break;
    default:
      els_parting_zjog();
      break;
  }
}

static void els_parting_set_xaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_parting.state = ELS_PARTING_IDLE;
      els_parting_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_parting.state == ELS_PARTING_SET_XAXES) {
        els_stepper_zero_x();
        els_dro_zero_x();
        els_parting_display_axes();
      }
      break;
    case ELS_KEY_JOG_ZX_ORI:
      if (!els_stepper->xbusy)
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_jog_mm_s);
      break;
    case ELS_KEY_SET_ZX:
      els_parting.state = ELS_PARTING_SET_ZAXES;
      els_parting_display_axes();
      break;
    default:
      els_parting_xjog();
      break;
  }
}

// ----------------------------------------------------------------------------------
// Manual Jog
// ----------------------------------------------------------------------------------
static void els_parting_zjog(void) {
  double  delta, step;
  int32_t  encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_parting.encoder_pos != encoder_curr) {
    step = els_parting.encoder_multiplier == 1 ? 0.005 : 0.01 * els_parting.encoder_multiplier;
    delta = (encoder_curr - els_parting.encoder_pos) * step;
    els_parting.encoder_pos = encoder_curr;
    els_stepper_move_z(delta, els_config->z_jog_mm_s);
  }
}

static void els_parting_xjog(void) {
  double delta, step;
  int32_t encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_parting.encoder_pos != encoder_curr) {
    step = els_parting.encoder_multiplier == 1 ? 0.005 : 0.01 * els_parting.encoder_multiplier;
    delta = (encoder_curr - els_parting.encoder_pos) * step;
    els_parting.encoder_pos = encoder_curr;
    els_stepper_move_x(delta, els_config->x_jog_mm_s);
  }
}
