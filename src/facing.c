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
#include "spindle.h"
#include "stepper.h"

#include "facing.h"
#include "utils.h"

#define PRECISION       1e-2
#define BACKOFF_DEPTH   5
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
  ELS_FACING_IDLE        = 1,
  ELS_FACING_PAUSED      = 2,
  ELS_FACING_ACTIVE      = 4,
  ELS_FACING_SET_ZAXES   = 8,
  ELS_FACING_ZJOG        = 16,
  ELS_FACING_SET_XAXES   = 32,
  ELS_FACING_XJOG        = 64,
  ELS_FACING_SET_FEED    = 128,
  ELS_FACING_SET_DOC     = 256,
  ELS_FACING_SET_LENGTH   = 512,
  ELS_FACING_SET_DEPTH   = 1024
} els_facing_state_t;

typedef enum {
  ELS_FACING_OP_NA      = -1,
  ELS_FACING_OP_IDLE    = 0,
  ELS_FACING_OP_READY   = 1,
  ELS_FACING_OP_MOVEZ0  = 2,
  ELS_FACING_OP_MOVEX0  = 3,
  ELS_FACING_OP_START   = 4,
  ELS_FACING_OP_ATX0    = 5,
  ELS_FACING_OP_ATXL    = 6,
  ELS_FACING_OP_ATXLZM  = 7,
  ELS_FACING_OP_ATX0ZM  = 8,
  ELS_FACING_OP_FEED_IN = 9,
  ELS_FACING_OP_RETURN  = 10,
  ELS_FACING_OP_DONE    = 11
} els_facing_op_state_t;

static const char *op_labels[] = {
  "CONFIG ",
  "READY  ",
  "MOVE Z0",
  "MOVE X0",
  "ORIGIN ",
  "FACING ",
  "LIMIT  ",
  "BACKOFF",
  "RETURN ",
  "FEED IN",
  "RETURN ",
  "DONE   "
};

//==============================================================================
// Internal state
//==============================================================================
#define ELS_FACING_FEED_MIN  (100)
#define ELS_FACING_FEED_MAX  (5000)

#define ELS_FACING_DOC_MIN   (50)
#define ELS_FACING_DOC_MAX   (2000)

static struct {
  uint32_t feed_um;
  double   feed_mm_s;

  // not supported yet.
  bool     feed_reverse;

  int32_t  depth_of_cut_um;

  bool     locked;

  double   length;
  double   depth;

  // input read for jogging
  int32_t  encoder_pos;

  // encoder multiplier
  int16_t encoder_multiplier;

  // module state
  els_facing_state_t state;

  // tracking variables for display refresh.
  els_facing_state_t prev_state;
  els_spindle_direction_t prev_dir;

  // operation state
  els_facing_op_state_t op_state;

  // saved z location
  double zpos_prev;

  // dro
  bool show_dro;

} els_facing = {
  .depth_of_cut_um = 250,
  .feed_um = 1000,
  .length = 1.0,
  .depth = 5.0,
  .encoder_multiplier = 1
};

//==============================================================================
// Internal functions
//==============================================================================
static void els_facing_run(void);
static void els_facing_turn(void);

static void els_facing_display_refresh(void);
static void els_facing_display_setting(void);
static void els_facing_display_axes(void);
static void els_facing_display_header(void);
static void els_facing_display_diagram(void);
static void els_facing_display_encoder_pips(void);

static void els_facing_set_feed(void);
static void els_facing_set_length_of_cut(void);
static void els_facing_set_length(void);
static void els_facing_set_depth(void);

static void els_facing_set_zaxes(void);
static void els_facing_set_xaxes(void);

static void els_facing_keypad_process(void);

// manual jogging
static void els_facing_zjog(void);
static void els_facing_xjog(void);

//==============================================================================
// API
//==============================================================================
void els_facing_setup(void) {
  // NO-OP
}

void els_facing_start(void) {
  char text[32];

  tft_filled_rectangle(&tft, 0,   0, 480, 320, ILI9481_BLACK);

  tft_filled_rectangle(&tft, 0,  50, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0,  50, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 100, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 100, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 190, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 190, 480,   1, ILI9481_LITEGRAY);

  els_facing_display_header();

  // angle symbol
  tft_font_write_bg(&tft, 165,  62, "J", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // pulse symbol
  tft_font_write_bg(&tft, 355,  58, "K", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // reset state
  els_facing.prev_state = 0;
  els_facing.prev_dir = 0;
  els_facing.feed_mm_s = (double)(els_facing.feed_um / 1000.0);

  if (!els_facing.show_dro)
    els_facing_display_diagram();

  els_facing.state = ELS_FACING_IDLE;

  els_facing_display_setting();
  els_facing_display_axes();
  els_facing_display_refresh();

  els_stepper_start();;
}

void els_facing_stop(void) {
  els_stepper_stop();;
}

bool els_facing_busy(void) {
  return els_facing.state != ELS_FACING_IDLE;
}

void els_facing_update(void) {
  static uint64_t last_refreshed_at = 0;

  uint64_t elapsed = els_timer_elapsed_microseconds();

  bool kp_locked = els_keypad_locked();
  if (els_facing.locked != kp_locked) {
    els_facing.locked = kp_locked;
    els_facing_display_header();
  }

  if (els_facing.state & (ELS_FACING_IDLE | ELS_FACING_PAUSED | ELS_FACING_ACTIVE))
    els_facing_keypad_process();

  switch (els_facing.state) {
    case ELS_FACING_PAUSED:
    case ELS_FACING_ACTIVE:
      els_facing_run();
      break;
    case ELS_FACING_SET_FEED:
      els_facing_set_feed();
      break;
    case ELS_FACING_SET_DOC:
      els_facing_set_length_of_cut();
      break;
    case ELS_FACING_SET_LENGTH:
      els_facing_set_length();
      break;
    case ELS_FACING_SET_DEPTH:
      els_facing_set_depth();
      break;
    default:
      if (els_facing.state & (ELS_FACING_SET_ZAXES | ELS_FACING_ZJOG))
        els_facing_set_zaxes();
      else if (els_facing.state & (ELS_FACING_SET_XAXES | ELS_FACING_XJOG))
        els_facing_set_xaxes();
      break;
  }

  if (elapsed - last_refreshed_at > 1e5) {
    last_refreshed_at = elapsed;
    els_facing_display_refresh();
  }

  int16_t em = els_encoder_get_multiplier();
  if (em != els_facing.encoder_multiplier) {
    els_facing.encoder_multiplier = em;
    els_facing_display_encoder_pips();
  }
}

//==============================================================================
// Internal functions
//==============================================================================

// ----------------------------------------------------------------------------------
// Display functions
// ----------------------------------------------------------------------------------
static void els_facing_display_setting(void) {
  char text[32];

  els_sprint_double2(text, sizeof(text), els_facing.feed_mm_s, "Xf");
  if (els_facing.state == ELS_FACING_SET_FEED)
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double2(text, sizeof(text), els_facing.depth_of_cut_um / 1000.0, "Zs");
  if (els_facing.state == ELS_FACING_SET_DOC)
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_facing.length, "L");
  if (els_facing.state == ELS_FACING_SET_LENGTH)
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_facing.depth, "D");
  if (els_facing.state == ELS_FACING_SET_DEPTH)
    tft_font_write_bg(&tft, 310, 262, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 262, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
}

static void els_facing_display_axes(void) {
  char text[32];

  els_sprint_double33(text, sizeof(text), els_stepper->zpos, "Z");

  if (els_facing.state & ELS_FACING_SET_ZAXES)
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), els_stepper->xpos, "X");
  if (els_facing.state & ELS_FACING_SET_XAXES)
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  if (els_facing.show_dro) {
    tft_font_write_bg(&tft, 8, 190, "DRO", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.zpos_um / 1000.0), "Z");
    tft_font_write_bg(&tft, 8, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.xpos_um / 1000.0), "X");
    tft_font_write_bg(&tft, 8, 262, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
  }
}

static void els_facing_display_diagram(void) {
  // ----------------------------------------------------------------------------------------------
  // diagram
  // ----------------------------------------------------------------------------------------------
  tft_filled_rectangle(&tft, 100, 220, 156, 80, ILI9481_WHITE);
  tft_filled_rectangle(&tft, 235, 220,  21, 80, ILI9481_ORANGE);
  tft_filled_rectangle(&tft, 80,  258, 200,  4, ILI9481_LITEGRAY);

  tft_font_write_bg(&tft, 235, 178, "A", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 220, 194, "L", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 240, 178, "C", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);

  tft_font_write_bg(&tft, 260, 184, "D", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 265, 226, "D", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 260, 250, "B", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);

  // origin
  tft_font_write(&tft, 250, 265, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW);

  // legend
  tft_font_write_bg(&tft, 8, 200, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  tft_font_write_bg(&tft, 26, 220, "(0,0)", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
}

static void els_facing_display_header(void) {
  tft_rgb_t color = (els_facing.locked ? ILI9481_RED : ILI9481_DIANNE);

  tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
  tft_font_write_bg(&tft, 8, 0, "FACING", &noto_sans_mono_bold_26, ILI9481_WHITE, color);
  els_facing_display_encoder_pips();
}

static void els_facing_display_encoder_pips(void) {
  // multiplier is 100, 10 or 1
  size_t pips = els_facing.encoder_multiplier;
  pips = pips > 10 ? 3 : pips > 1 ? 2 : 1;

  for (size_t n = 0, spacing = 0; n < 3; n++, spacing++)
    tft_filled_rectangle(&tft,
      440, 32 - (n * 10 + spacing * 2),
      15 + n * 10, 10,
      (n < pips ? ILI9481_WHITE : els_facing.locked ? ILI9481_LITEGRAY : ILI9481_BGCOLOR1));
}

static void els_facing_display_refresh(void) {
  els_spindle_direction_t dir = els_spindle_get_direction();
  if (dir != els_facing.prev_dir) {
    els_facing.prev_dir = dir;
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

  els_facing_display_axes();

  static els_facing_op_state_t prev_op_state = ELS_FACING_OP_NA;
  if (els_facing.op_state != prev_op_state) {
    prev_op_state = els_facing.op_state;
    const char *label = op_labels[els_facing.op_state];
    tft_filled_rectangle(&tft, 310, 195, 169, 30, ILI9481_BLACK);
    tft_font_write_bg(&tft, 310, 190, label, &noto_sans_mono_bold_26, ILI9481_CERULEAN, ILI9481_BLACK);
  }
}

// ----------------------------------------------------------------------------------
// Top module level keypad handler.
// ----------------------------------------------------------------------------------
static void els_facing_keypad_process(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
      if (els_facing.state == ELS_FACING_IDLE) {
        if (els_facing.length > 0 && els_facing.depth > 0) {
          els_facing.state = ELS_FACING_PAUSED;
          els_facing.op_state = ELS_FACING_OP_READY;
        }
        else {
          els_facing.state = ELS_FACING_SET_LENGTH;
          els_facing_display_setting();
        }
      }
      break;
    case ELS_KEY_EXIT:
      if (els_facing.state & (ELS_FACING_PAUSED | ELS_FACING_ACTIVE)) {
        els_facing.state = ELS_FACING_IDLE;
        els_facing.op_state = ELS_FACING_OP_IDLE;
      }
      break;
    case ELS_KEY_SET_FEED:
      if (els_facing.state & (ELS_FACING_IDLE | ELS_FACING_PAUSED)) {
        els_facing.state = ELS_FACING_SET_FEED;
        els_facing.encoder_pos = 0;
        els_facing_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F1:
      if (els_facing.state & (ELS_FACING_IDLE | ELS_FACING_PAUSED)) {
        els_facing.state = ELS_FACING_SET_LENGTH;
        els_facing.encoder_pos = 0;
        els_facing_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F2:
      els_facing.show_dro = !els_facing.show_dro;
      tft_filled_rectangle(&tft, 0, 200, 300, 120, ILI9481_BLACK);
      if (els_facing.show_dro)
        els_facing_display_axes();
      else
        els_facing_display_diagram();
      break;
    case ELS_KEY_SET_ZX:
      if (els_facing.state & (ELS_FACING_IDLE | ELS_FACING_PAUSED)) {
        els_facing.state = ELS_FACING_SET_ZAXES;
        els_facing.encoder_pos = 0;
        els_facing_display_axes();
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
static void els_facing_run(void) {
  switch (els_spindle_get_direction()) {
    case ELS_S_DIRECTION_CW:
    case ELS_S_DIRECTION_CCW:
      if (els_facing.state == ELS_FACING_PAUSED) {
        if (!els_stepper->zbusy && !els_stepper->xbusy && els_spindle_get_counter() == 0)
          els_facing.state = ELS_FACING_ACTIVE;
      }
      break;
    default:
      if (els_facing.state == ELS_FACING_ACTIVE)
        els_facing.state = ELS_FACING_PAUSED;
      break;
  }

  if (els_facing.state == ELS_FACING_ACTIVE)
    els_facing_turn();
}

static void els_facing_turn(void) {
  double zd;

  switch (els_facing.op_state) {
    case ELS_FACING_OP_NA:
      break;
    case ELS_FACING_OP_IDLE:
      break;
    case ELS_FACING_OP_READY:
      els_facing.op_state = ELS_FACING_OP_MOVEZ0;
      els_stepper_sync();
      break;
    case ELS_FACING_OP_MOVEZ0:
      if (els_stepper->zbusy)
        break;

      // move to Z=0
      if (fabs(els_stepper->zpos) > PRECISION)
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_retract_jog_mm_s);
      else
        els_facing.op_state = ELS_FACING_OP_MOVEX0;
      break;
    case ELS_FACING_OP_MOVEX0:
      if (els_stepper->xbusy)
        break;
      // move to X=0
      if (fabs(els_stepper->xpos) > PRECISION)
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_retract_jog_mm_s);
      else
        els_facing.op_state = ELS_FACING_OP_START;
      break;
    case ELS_FACING_OP_START:
      // initial move
      els_stepper_move_x_no_accel(-els_facing.depth, els_facing.feed_mm_s);
      els_facing.op_state = ELS_FACING_OP_ATX0;
      break;
    case ELS_FACING_OP_ATX0:
      if (els_stepper->xbusy)
        break;

      // backoff
      els_facing.zpos_prev = els_stepper->zpos;
      els_stepper_move_z(BACKOFF_DEPTH, els_config->z_retract_jog_mm_s);
      els_facing.op_state = ELS_FACING_OP_ATXL;
      break;
    case ELS_FACING_OP_ATXL:
      if (els_stepper->zbusy)
        break;

      // back to X=0
      els_stepper_move_x(0 - els_stepper->xpos, els_config->x_retract_jog_mm_s);
      els_facing.op_state = ELS_FACING_OP_ATXLZM;
      break;
    case ELS_FACING_OP_ATXLZM:
      if (els_stepper->xbusy)
        break;

      // move to previous z postion before backoff
      els_stepper_move_z(els_facing.zpos_prev - els_stepper->zpos, els_config->z_retract_jog_mm_s);
      els_facing.op_state = ELS_FACING_OP_ATX0ZM;
      break;
    case ELS_FACING_OP_ATX0ZM:
      if (els_stepper->zbusy)
        break;

      // feed-in
      zd = MIN(
        els_facing.length + els_stepper->zpos,
        els_facing.depth_of_cut_um / 1000.0
      );

      if (zd > PRECISION) {
        els_stepper_move_z(-zd, els_facing.feed_mm_s);
        els_facing.op_state = ELS_FACING_OP_FEED_IN;
      }
      else {
        els_facing.op_state = ELS_FACING_OP_RETURN;
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_retract_jog_mm_s);
      }
      break;
    case ELS_FACING_OP_FEED_IN:
      if (els_stepper->zbusy)
        break;

      els_facing.op_state = ELS_FACING_OP_ATX0;
      els_stepper_move_x_no_accel(-els_facing.depth, els_facing.feed_mm_s);
      break;
    case ELS_FACING_OP_RETURN:
      if (els_stepper->zbusy)
        break;

      els_facing.op_state = ELS_FACING_OP_DONE;
      els_stepper_move_x(0 - els_stepper->xpos, els_config->x_retract_jog_mm_s);
      break;
    case ELS_FACING_OP_DONE:
      // beer time
      els_facing.state = ELS_FACING_IDLE;
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.1: feed settings.
// ----------------------------------------------------------------------------------
static void els_facing_set_feed(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_facing.state = ELS_FACING_IDLE;
      els_facing_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_facing.state = ELS_FACING_SET_DOC;
      els_facing_display_setting();
      break;
    case ELS_KEY_REV_FEED:
      // TODO
      // els_facing.feed_reverse = !els_facing.feed_reverse;
      els_facing_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_facing.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_facing.encoder_pos) * 10 * els_facing.encoder_multiplier;
        if (els_facing.feed_um + delta <= ELS_FACING_FEED_MIN)
          els_facing.feed_um = ELS_FACING_FEED_MIN;
        else if (els_facing.feed_um + delta >= ELS_FACING_FEED_MAX)
          els_facing.feed_um = ELS_FACING_FEED_MAX;
        else
          els_facing.feed_um += delta;
        els_facing.encoder_pos = encoder_curr;
        els_facing.feed_mm_s = els_facing.feed_um / 1000.0;
        els_facing_display_setting();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.2: depth of cut settings.
// ----------------------------------------------------------------------------------
static void els_facing_set_length_of_cut(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_facing.state = ELS_FACING_IDLE;
      els_facing_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_facing.state = ELS_FACING_SET_FEED;
      els_facing_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_facing.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_facing.encoder_pos) * 10 * els_facing.encoder_multiplier;
        if (els_facing.depth_of_cut_um + delta <= ELS_FACING_DOC_MIN)
          els_facing.depth_of_cut_um = ELS_FACING_DOC_MIN;
        else if (els_facing.depth_of_cut_um + delta >= ELS_FACING_DOC_MAX)
          els_facing.depth_of_cut_um = ELS_FACING_DOC_MAX;
        else
          els_facing.depth_of_cut_um += delta;
        els_facing.encoder_pos = encoder_curr;
        els_facing_display_setting();
      }
      break;
  }
}

void els_facing_set_length(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_facing.state = ELS_FACING_IDLE;
      els_facing_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_facing.state = ELS_FACING_SET_DEPTH;
      els_facing_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_facing.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_facing.encoder_pos) * 0.01 * els_facing.encoder_multiplier;
        if (els_facing.length + delta <= 0)
          els_facing.length = 0;
        else if (els_facing.length + delta >= ELS_Z_MAX_MM)
          els_facing.length = ELS_Z_MAX_MM;
        else
          els_facing.length += delta;
        els_facing.encoder_pos = encoder_curr;
        els_facing_display_setting();
      }
      break;
  }
}

void els_facing_set_depth(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_facing.state = ELS_FACING_IDLE;
      els_facing_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_facing.state = ELS_FACING_SET_LENGTH;
      els_facing_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_facing.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_facing.encoder_pos) * 0.01 * els_facing.encoder_multiplier;
        if (els_facing.depth + delta <= 0)
          els_facing.depth = 0;
        else if (els_facing.depth + delta >= ELS_X_MAX_MM)
          els_facing.depth = ELS_X_MAX_MM;
        else
          els_facing.depth += delta;
        els_facing.encoder_pos = encoder_curr;
        els_facing_display_setting();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 3: Axis - position, origin & jogging
// ----------------------------------------------------------------------------------
static void els_facing_set_zaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_facing.state = (els_facing.state & ELS_FACING_ZJOG) ? ELS_FACING_SET_ZAXES : ELS_FACING_IDLE;
      els_facing_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_facing.state == ELS_FACING_SET_ZAXES) {
        els_stepper_zero_z();
        els_dro_zero_z();
        els_facing_display_axes();
      }
      break;
    case ELS_KEY_SET_ZX:
      els_facing.state = ELS_FACING_SET_XAXES;
      els_facing_display_axes();
      break;
    default:
      els_facing_zjog();
      break;
  }
}

static void els_facing_set_xaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_facing.state = ELS_FACING_IDLE;
      els_facing_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_facing.state == ELS_FACING_SET_XAXES) {
        els_stepper_zero_x();
        els_dro_zero_x();
        els_facing_display_axes();
      }
      break;
    case ELS_KEY_SET_ZX:
      els_facing.state = ELS_FACING_SET_ZAXES;
      els_facing_display_axes();
      break;
    default:
      els_facing_xjog();
      break;
  }
}

// ----------------------------------------------------------------------------------
// Manual Jog
// ----------------------------------------------------------------------------------

static void els_facing_zjog(void) {
  double delta;
  int32_t encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_facing.encoder_pos != encoder_curr) {
    delta = (encoder_curr - els_facing.encoder_pos) * (0.01 * els_facing.encoder_multiplier);
    els_facing.encoder_pos = encoder_curr;
    els_stepper_move_z(delta, els_config->z_jog_mm_s);
  }
}

static void els_facing_xjog(void) {
  double delta;
  int32_t encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_facing.encoder_pos != encoder_curr) {
    delta = (encoder_curr - els_facing.encoder_pos) * (0.01 * els_facing.encoder_multiplier);
    els_facing.encoder_pos = encoder_curr;
    els_stepper_move_x(delta, els_config->x_jog_mm_s);
  }
}
