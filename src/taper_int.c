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

#include "taper_int.h"
#include "utils.h"

#define PRECISION 1e-2
//==============================================================================
// Externs
//==============================================================================
extern tft_device_t tft;

extern const tft_font_t noto_sans_mono_bold_arrows_24;
extern const tft_font_t noto_sans_mono_bold_26;
extern const tft_font_t noto_sans_mono_bold_14;
extern const tft_font_t gears_regular_32;
extern const tft_font_t gears_regular_50;
extern const tft_font_t inconsolata_lgc_bold_14;

//==============================================================================
// Config
//==============================================================================
typedef enum {
  ELS_TAPER_EXT_IDLE      = 1,
  ELS_TAPER_EXT_PAUSED    = 2,
  ELS_TAPER_EXT_ACTIVE    = 4,
  ELS_TAPER_EXT_SET_ZAXES = 8,
  ELS_TAPER_EXT_ZJOG      = 16,
  ELS_TAPER_EXT_SET_XAXES = 32,
  ELS_TAPER_EXT_XJOG      = 64,
  ELS_TAPER_EXT_SET_FEED  = 128,
  ELS_TAPER_EXT_SET_DOC   = 256,
  ELS_TAPER_EXT_SET_LEN   = 512,
  ELS_TAPER_EXT_SET_DEPTH = 1024
} els_taper_int_state_t;

typedef enum {
  ELS_TAPER_EXT_OP_NA      = -1,
  ELS_TAPER_EXT_OP_IDLE    = 0,
  ELS_TAPER_EXT_OP_READY   = 1,
  ELS_TAPER_EXT_OP_MOVEZ0  = 2,
  ELS_TAPER_EXT_OP_MOVEX0  = 3,
  ELS_TAPER_EXT_OP_START   = 4,
  ELS_TAPER_EXT_OP_FEED    = 5,
  ELS_TAPER_EXT_OP_PLAN    = 6,
  ELS_TAPER_EXT_OP_TURNING = 7,
  ELS_TAPER_EXT_OP_RESETZ  = 8,
  ELS_TAPER_EXT_OP_RESETX  = 9,
  ELS_TAPER_EXT_OP_DONE    = 10
} els_taper_int_op_state_t;

static const char *op_labels[] = {
  "CONFIG ",
  "READY  ",
  "MOVE Z0",
  "MOVE X0",
  "ORIGIN ",
  "FEED IN",
  "PLANNER",
  "TURNING",
  "MOVE Z0",
  "MOVE XC",
  "DONE   "
};


//==============================================================================
// Internal state
//==============================================================================
#define ELS_TAPER_EXT_FEED_MIN  (100)
#define ELS_TAPER_EXT_FEED_MAX  (6000)

#define ELS_TAPER_EXT_DOC_MIN   (10)
#define ELS_TAPER_EXT_DOC_MAX   (2000)

static struct {
  int32_t  feed_um;
  double   feed_mm_s;

  // not supported
  bool     feed_reverse;

  int32_t  depth_of_cut_um;

  bool     locked;

  uint32_t spring_pass_count;

  double   length;
  double   depth;
  double   slope;

  double   xcurr;

  // input read for jogging
  int32_t  encoder_pos;

  // encoder multiplier
  int16_t encoder_multiplier;

  // module state
  els_taper_int_state_t state;

  // tracking variables for display refresh.
  els_taper_int_state_t prev_state;
  els_spindle_direction_t prev_dir;

  // operation state
  els_taper_int_op_state_t op_state;
  els_taper_int_op_state_t prev_op_state;

  // dro
  bool show_dro;

  // only spring pass
  bool only_spring_pass;
} els_taper_int = {
  .depth_of_cut_um = 200,
  .feed_um = 4000,
  .length = 10,
  .depth = 2,
  .encoder_multiplier = 1
};

//==============================================================================
// Internal functions
//==============================================================================
static void els_taper_int_run(void);
static void els_taper_int_turn(void);

static void els_taper_int_display_refresh(void);
static void els_taper_int_display_setting(void);
static void els_taper_int_display_axes(void);
static void els_taper_int_display_header(void);
static void els_taper_int_display_diagram(void);
static void els_taper_int_display_encoder_pips(void);

static void els_taper_int_set_feed(void);
static void els_taper_int_set_depth_of_cut(void);
static void els_taper_int_set_length(void);
static void els_taper_int_set_depth(void);

static void els_taper_int_set_zaxes(void);
static void els_taper_int_set_xaxes(void);

static void els_taper_int_keypad_process(void);

// manual jogging
static void els_taper_int_zjog(void);
static void els_taper_int_xjog(void);

//==============================================================================
// API
//==============================================================================
void els_taper_int_setup(void) {
  // NO-OP
}

void els_taper_int_start(void) {
  char text[32];

  tft_filled_rectangle(&tft, 0,   0, 480, 320, ILI9481_BLACK);

  tft_filled_rectangle(&tft, 0,  50, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0,  50, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 100, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 100, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 190, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 190, 480,   1, ILI9481_LITEGRAY);

  els_taper_int_display_header();

  if (!els_taper_int.show_dro)
    els_taper_int_display_diagram();

  // angle symbol
  tft_font_write_bg(&tft, 165,  62, "J", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // pulse symbol
  tft_font_write_bg(&tft, 355,  58, "K", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // reset state
  els_taper_int.prev_state = 0;
  els_taper_int.prev_dir = 0;
  els_taper_int.feed_mm_s = els_taper_int.feed_um / 1000.0;

  els_taper_int.state = ELS_TAPER_EXT_IDLE;
  els_taper_int.op_state = ELS_TAPER_EXT_OP_IDLE;
  els_taper_int.prev_op_state = ELS_TAPER_EXT_OP_NA;

  els_taper_int_display_setting();
  els_taper_int_display_axes();
  els_taper_int_display_refresh();

  els_stepper_start();
}

void els_taper_int_stop(void) {
  els_stepper_stop();
}

bool els_taper_int_busy(void) {
  return els_taper_int.state != ELS_TAPER_EXT_IDLE;
}

void els_taper_int_update(void) {
  static uint64_t last_refreshed_at = 0;

  uint64_t elapsed = els_timer_elapsed_microseconds();

  bool kp_locked = els_keypad_locked();
  if (els_taper_int.locked != kp_locked) {
    els_taper_int.locked = kp_locked;
    els_taper_int_display_header();
  }

  if (els_taper_int.state & (ELS_TAPER_EXT_IDLE | ELS_TAPER_EXT_PAUSED | ELS_TAPER_EXT_ACTIVE))
    els_taper_int_keypad_process();

  switch (els_taper_int.state) {
    case ELS_TAPER_EXT_PAUSED:
    case ELS_TAPER_EXT_ACTIVE:
      els_taper_int_run();
      break;
    case ELS_TAPER_EXT_SET_FEED:
      els_taper_int_set_feed();
      break;
    case ELS_TAPER_EXT_SET_DOC:
      els_taper_int_set_depth_of_cut();
      break;
    case ELS_TAPER_EXT_SET_LEN:
      els_taper_int_set_length();
      break;
    case ELS_TAPER_EXT_SET_DEPTH:
      els_taper_int_set_depth();
      break;
    default:
      if (els_taper_int.state & (ELS_TAPER_EXT_SET_ZAXES | ELS_TAPER_EXT_ZJOG))
        els_taper_int_set_zaxes();
      else if (els_taper_int.state & (ELS_TAPER_EXT_SET_XAXES | ELS_TAPER_EXT_XJOG))
        els_taper_int_set_xaxes();
      break;
  }

  if (elapsed - last_refreshed_at > 1e5) {
    last_refreshed_at = elapsed;
    els_taper_int_display_refresh();
  }

  int16_t em = els_encoder_get_multiplier();
  if (em != els_taper_int.encoder_multiplier) {
    els_taper_int.encoder_multiplier = em;
    els_taper_int_display_encoder_pips();
  }
}

//==============================================================================
// Internal functions
//==============================================================================

// ----------------------------------------------------------------------------------
// Display functions
// ----------------------------------------------------------------------------------
static void els_taper_int_display_setting(void) {
  char text[32];

  els_sprint_double2(text, sizeof(text), els_taper_int.feed_um / 1000.0, "Zf");
  if (els_taper_int.state == ELS_TAPER_EXT_SET_FEED)
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double2(text, sizeof(text), els_taper_int.depth_of_cut_um / 1000.0, "Xs");
  if (els_taper_int.state == ELS_TAPER_EXT_SET_DOC)
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_taper_int.length, "L");
  if (els_taper_int.state == ELS_TAPER_EXT_SET_LEN)
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_taper_int.depth, "D");
  if (els_taper_int.state == ELS_TAPER_EXT_SET_DEPTH)
    tft_font_write_bg(&tft, 310, 262, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 262, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  if (els_taper_int.only_spring_pass) {
    tft_filled_rectangle(&tft, 227, 149, 73, 25, ILI9481_WHITE);
    tft_font_write_bg(&tft, 230, 149, "SPRING", &noto_sans_mono_bold_14, ILI9481_BLACK, ILI9481_WHITE);
  }
  else {
    tft_filled_rectangle(&tft, 227, 149, 80, 30, ILI9481_BLACK);
  }
}

static void els_taper_int_display_axes(void) {
  char text[32];

  els_sprint_double33(text, sizeof(text), els_stepper->zpos, "Z");
  if (els_taper_int.state & ELS_TAPER_EXT_SET_ZAXES)
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), els_stepper->xpos, "X");
  if (els_taper_int.state & ELS_TAPER_EXT_SET_XAXES)
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  if (els_taper_int.show_dro) {
    tft_font_write_bg(&tft, 8, 190, "DRO", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.zpos_um / 1000.0), "Z");
    tft_font_write_bg(&tft, 8, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.xpos_um / 1000.0), "X");
    tft_font_write_bg(&tft, 8, 262, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double24(text, sizeof(text), atan(els_taper_int.depth / els_taper_int.length) * 180 / M_PI, NULL);
    tft_font_write_bg(&tft, 226, 240, "J", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);
    tft_font_write_bg(&tft, 226, 280, text, &noto_sans_mono_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  }
}

static void els_taper_int_display_diagram(void) {
  // ----------------------------------------------------------------------------------------------
  // diagram
  // ----------------------------------------------------------------------------------------------
  tft_filled_rectangle(&tft, 100, 220, 156, 80, ILI9481_WHITE);
  tft_filled_triangle(&tft,  254, 220, 156, 240, 254, 240, ILI9481_ORANGE);
  tft_filled_triangle(&tft,  254, 280, 156, 280, 254, 300, ILI9481_ORANGE);
  tft_filled_rectangle(&tft, 156, 240, 100, 40, ILI9481_BLACK);

  tft_font_write_bg(&tft, 156, 178, "A", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 205, 194, "L", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 240, 178, "C", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);

  tft_font_write_bg(&tft, 260, 184, "D", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 280, 216, "D", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 260, 224, "B", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);

  // origin
  tft_font_write_bg(&tft, 250, 245, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);

  // legend
  tft_font_write_bg(&tft, 8, 200, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  tft_font_write_bg(&tft, 26, 220, "(0,0)", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  // ----------------------------------------------------------------------------------------------
}

static void els_taper_int_display_header(void) {
  tft_rgb_t color = (els_taper_int.locked ? ILI9481_RED : ILI9481_DIANNE);

  tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
  tft_font_write_bg(&tft, 8, 0, "TAPER INTERNAL", &noto_sans_mono_bold_26, ILI9481_WHITE, color);
  els_taper_int_display_encoder_pips();
}

static void els_taper_int_display_encoder_pips(void) {
  // multiplier is 100, 10 or 1
  size_t pips = els_taper_int.encoder_multiplier;
  pips = pips > 10 ? 3 : pips > 1 ? 2 : 1;

  for (size_t n = 0, spacing = 0; n < 3; n++, spacing++)
    tft_filled_rectangle(&tft,
      440, 32 - (n * 10 + spacing * 2),
      15 + n * 10, 10,
      (n < pips ? ILI9481_WHITE : els_taper_int.locked ? ILI9481_LITEGRAY : ILI9481_BGCOLOR1));
}

static void els_taper_int_display_refresh(void) {
  els_spindle_direction_t dir = els_spindle_get_direction();
  if (dir != els_taper_int.prev_dir) {
    els_taper_int.prev_dir = dir;
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

  els_taper_int_display_axes();

  if (els_taper_int.op_state != els_taper_int.prev_op_state) {
    els_taper_int.prev_op_state = els_taper_int.op_state;
    tft_filled_rectangle(&tft, 310, 195, 169, 35, ILI9481_BLACK);
    if (els_taper_int.op_state == ELS_TAPER_EXT_OP_TURNING && els_taper_int.spring_pass_count > 0) {
      tft_font_write_bg(&tft, 310, 190, "SPRING", &noto_sans_mono_bold_26, ILI9481_CERULEAN, ILI9481_BLACK);
    }
    else {
      const char *label = op_labels[els_taper_int.op_state];
      tft_font_write_bg(&tft, 310, 190, label, &noto_sans_mono_bold_26, ILI9481_CERULEAN, ILI9481_BLACK);
    }
  }
}

// ----------------------------------------------------------------------------------
// Top module level keypad handler.
// ----------------------------------------------------------------------------------
static void els_taper_int_keypad_process(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
      if (els_taper_int.state == ELS_TAPER_EXT_IDLE) {
        if (els_taper_int.length > 0 && els_taper_int.depth > 0) {
          els_taper_int.state = ELS_TAPER_EXT_PAUSED;
          els_taper_int.op_state = ELS_TAPER_EXT_OP_READY;
        }
        else {
          els_taper_int.state = ELS_TAPER_EXT_SET_LEN;
          els_taper_int_display_setting();
        }
      }
      break;
    case ELS_KEY_EXIT:
      if (els_taper_int.state & (ELS_TAPER_EXT_PAUSED | ELS_TAPER_EXT_ACTIVE)) {
        els_taper_int.state = ELS_TAPER_EXT_IDLE;
        els_taper_int.op_state = ELS_TAPER_EXT_OP_IDLE;
      }
      break;
    case ELS_KEY_SET_FEED:
      if (els_taper_int.state & (ELS_TAPER_EXT_IDLE | ELS_TAPER_EXT_PAUSED)) {
        els_taper_int.state = ELS_TAPER_EXT_SET_FEED;
        els_taper_int.encoder_pos = 0;
        els_taper_int_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F1:
      if (els_taper_int.state & (ELS_TAPER_EXT_IDLE | ELS_TAPER_EXT_PAUSED)) {
        els_taper_int.state = ELS_TAPER_EXT_SET_LEN;
        els_taper_int.encoder_pos = 0;
        els_taper_int_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F2:
      els_taper_int.show_dro = !els_taper_int.show_dro;
      tft_filled_rectangle(&tft, 0, 200, 300, 120, ILI9481_BLACK);
      if (els_taper_int.show_dro)
        els_taper_int_display_axes();
      else
        els_taper_int_display_diagram();
      break;
    case ELS_KEY_REV_FEED:
      if (els_taper_int.state & (ELS_TAPER_EXT_IDLE | ELS_TAPER_EXT_PAUSED)) {
        els_taper_int.only_spring_pass = !els_taper_int.only_spring_pass;
        els_taper_int_display_setting();
      }
      break;
    case ELS_KEY_SET_ZX:
      if (els_taper_int.state & (ELS_TAPER_EXT_IDLE | ELS_TAPER_EXT_PAUSED)) {
        els_taper_int.state = ELS_TAPER_EXT_SET_ZAXES;
        els_taper_int.encoder_pos = 0;
        els_taper_int_display_axes();
        els_encoder_reset();
      }
      break;
    default:
      break;
  }
}

// ---------------------------------------------------------------------------------------
// Function 1: primary turning handler
// ---------------------------------------------------------------------------------------
static void els_taper_int_run(void) {
  switch (els_spindle_get_direction()) {
    case ELS_S_DIRECTION_CW:
    case ELS_S_DIRECTION_CCW:
      if (els_taper_int.state == ELS_TAPER_EXT_PAUSED && els_spindle_get_counter() == 0)
        els_taper_int.state = ELS_TAPER_EXT_ACTIVE;
      break;
    default:
      if (els_taper_int.state == ELS_TAPER_EXT_ACTIVE)
        els_taper_int.state = ELS_TAPER_EXT_PAUSED;
      break;
  }

  if (els_taper_int.state == ELS_TAPER_EXT_ACTIVE)
    els_taper_int_turn();
}

static void els_taper_int_turn(void) {
  double xd, remaining;

  switch (els_taper_int.op_state) {
    case ELS_TAPER_EXT_OP_NA:
    case ELS_TAPER_EXT_OP_IDLE:
      break;
    case ELS_TAPER_EXT_OP_READY:
      els_taper_int.op_state = ELS_TAPER_EXT_OP_MOVEZ0;
      els_stepper_sync();
      break;
    case ELS_TAPER_EXT_OP_MOVEZ0:
      if (els_stepper->zbusy)
        break;

      if (fabs(els_stepper->zpos) > PRECISION)
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_retract_jog_mm_s);
      else
        els_taper_int.op_state = ELS_TAPER_EXT_OP_MOVEX0;
      break;
    case ELS_TAPER_EXT_OP_MOVEX0:
      if (els_taper_int.only_spring_pass) {
        els_taper_int.op_state = ELS_TAPER_EXT_OP_START;
        break;
      }

      if (els_stepper->xbusy)
        break;

      if (fabs(els_stepper->xpos) > PRECISION)
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_retract_jog_mm_s);
      else
        els_taper_int.op_state = ELS_TAPER_EXT_OP_START;
      break;
    case ELS_TAPER_EXT_OP_START:
      if (els_stepper->xbusy)
        break;

      els_taper_int.slope = (els_taper_int.length / els_taper_int.depth);
      els_taper_int.op_state = ELS_TAPER_EXT_OP_FEED;
      els_taper_int.spring_pass_count = 0;

      if (els_taper_int.only_spring_pass)
        els_stepper_move_x(els_taper_int.depth - els_stepper->xpos - 0.1, els_config->x_retract_jog_mm_s);
      break;
    case ELS_TAPER_EXT_OP_FEED:
      if (els_stepper->xbusy)
        break;

      els_taper_int.xcurr = els_stepper->xpos;
      remaining = fabs(-els_taper_int.xcurr + els_taper_int.depth);
      if (remaining <= PRECISION) {
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_retract_jog_mm_s);
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_retract_jog_mm_s);
        els_taper_int.op_state = ELS_TAPER_EXT_OP_DONE;
      }
      else {
        if (remaining >= (2 * els_taper_int.depth_of_cut_um) / 1000.0)
          xd = els_taper_int.depth_of_cut_um / 1000.0;
        else if (remaining > (els_taper_int.depth_of_cut_um / 1000.0))
          xd = remaining / 2.0;
        else {
          xd = remaining;
          els_taper_int.spring_pass_count++;
          els_delay_reset();
        }

        els_stepper_move_x(xd, els_config->x_retract_jog_mm_s);
        els_taper_int.op_state = ELS_TAPER_EXT_OP_PLAN;
      }
      break;
    case ELS_TAPER_EXT_OP_PLAN:
      if (els_stepper->xbusy)
        break;

      els_taper_int.xcurr = els_stepper->xpos;
      els_taper_int.op_state = ELS_TAPER_EXT_OP_TURNING;
      els_stepper_move_xz(
        0 - els_stepper->xpos,
        -els_stepper->xpos * els_taper_int.slope,
        (els_taper_int.spring_pass_count > 0 ? els_taper_int.feed_mm_s / 4 : els_taper_int.feed_mm_s)
      );
      break;
    case ELS_TAPER_EXT_OP_TURNING:
      if (els_stepper->xbusy || els_stepper->zbusy)
        break;

      els_taper_int.op_state = ELS_TAPER_EXT_OP_RESETZ;
      els_stepper_move_z(0 - els_stepper->zpos, els_config->z_retract_jog_mm_s);
      break;
    case ELS_TAPER_EXT_OP_RESETZ:
      if (els_stepper->zbusy)
        break;

      if (fabs(-els_taper_int.xcurr + els_taper_int.depth) > PRECISION) {
        els_stepper_move_x(els_taper_int.xcurr - els_stepper->xpos, els_config->x_retract_jog_mm_s);
        els_taper_int.op_state = ELS_TAPER_EXT_OP_RESETX;
      }
      else {
        els_taper_int.op_state = ELS_TAPER_EXT_OP_DONE;
      }
      break;
    case ELS_TAPER_EXT_OP_RESETX:
      if (els_stepper->xbusy)
        break;
      els_taper_int.op_state = ELS_TAPER_EXT_OP_FEED;
      break;
    case ELS_TAPER_EXT_OP_DONE:
      // beer time
      if (els_stepper->xbusy || els_stepper->zbusy)
        break;

      els_taper_int.op_state = ELS_TAPER_EXT_OP_IDLE;
      els_taper_int.state = ELS_TAPER_EXT_IDLE;
      els_taper_int.spring_pass_count = 0;
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.1: feed settings.
// ----------------------------------------------------------------------------------
static void els_taper_int_set_feed(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_taper_int.state = ELS_TAPER_EXT_IDLE;
      els_taper_int_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_taper_int.state = ELS_TAPER_EXT_SET_DOC;
      els_taper_int_display_setting();
      break;
    case ELS_KEY_REV_FEED:
      // TODO
      els_taper_int_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_taper_int.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_taper_int.encoder_pos) * 10 * els_taper_int.encoder_multiplier;
        if (els_taper_int.feed_um + delta <= ELS_TAPER_EXT_FEED_MIN)
          els_taper_int.feed_um = ELS_TAPER_EXT_FEED_MIN;
        else if (els_taper_int.feed_um + delta >= ELS_TAPER_EXT_FEED_MAX)
          els_taper_int.feed_um = ELS_TAPER_EXT_FEED_MAX;
        else
          els_taper_int.feed_um += delta;
        els_taper_int.encoder_pos = encoder_curr;
        els_taper_int_display_setting();
        els_taper_int.feed_mm_s = els_taper_int.feed_um / 1000.0;
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.2: depth of cut settings.
// ----------------------------------------------------------------------------------
static void els_taper_int_set_depth_of_cut(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_taper_int.state = ELS_TAPER_EXT_IDLE;
      els_taper_int_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_taper_int.state = ELS_TAPER_EXT_SET_FEED;
      els_taper_int_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_taper_int.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_taper_int.encoder_pos) * 10 * els_taper_int.encoder_multiplier;
        if (els_taper_int.depth_of_cut_um + delta <= ELS_TAPER_EXT_DOC_MIN)
          els_taper_int.depth_of_cut_um = ELS_TAPER_EXT_DOC_MIN;
        else if (els_taper_int.depth_of_cut_um + delta >= ELS_TAPER_EXT_DOC_MAX)
          els_taper_int.depth_of_cut_um = ELS_TAPER_EXT_DOC_MAX;
        else
          els_taper_int.depth_of_cut_um += delta;
        els_taper_int.encoder_pos = encoder_curr;
        els_taper_int_display_setting();
      }
      break;
  }
}

void els_taper_int_set_length(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_taper_int.state = ELS_TAPER_EXT_IDLE;
      els_taper_int_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_taper_int.state = ELS_TAPER_EXT_SET_DEPTH;
      els_taper_int_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_taper_int.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_taper_int.encoder_pos) * 0.01 * els_taper_int.encoder_multiplier;
        if (els_taper_int.length + delta <= 0)
          els_taper_int.length = 0;
        else if (els_taper_int.length + delta >= ELS_Z_MAX_MM)
          els_taper_int.length = ELS_Z_MAX_MM;
        else
          els_taper_int.length += delta;
        els_taper_int.encoder_pos = encoder_curr;
        els_taper_int_display_setting();
      }
      break;
  }
}

void els_taper_int_set_depth(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_taper_int.state = ELS_TAPER_EXT_IDLE;
      els_taper_int_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_taper_int.state = ELS_TAPER_EXT_SET_LEN;
      els_taper_int_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_taper_int.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_taper_int.encoder_pos) * 0.01 * els_taper_int.encoder_multiplier;
        if (els_taper_int.depth + delta <= 0)
          els_taper_int.depth = 0;
        else if (els_taper_int.depth + delta >= ELS_X_MAX_MM)
          els_taper_int.depth = ELS_X_MAX_MM;
        else
          els_taper_int.depth += delta;
        els_taper_int.encoder_pos = encoder_curr;
        els_taper_int_display_setting();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 3: Axis - position, origin & jogging
// ----------------------------------------------------------------------------------
static void els_taper_int_set_zaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_taper_int.state = (els_taper_int.state & ELS_TAPER_EXT_ZJOG) ? ELS_TAPER_EXT_SET_ZAXES : ELS_TAPER_EXT_IDLE;
      els_taper_int_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_taper_int.state == ELS_TAPER_EXT_SET_ZAXES) {
        els_stepper_zero_z();
        els_dro_zero_z();
        els_taper_int_display_axes();
      }
      break;
    case ELS_KEY_SET_ZX:
      els_taper_int.state = ELS_TAPER_EXT_SET_XAXES;
      els_taper_int_display_axes();
      break;
    default:
      els_taper_int_zjog();
      break;
  }
}

static void els_taper_int_set_xaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_taper_int.state = ELS_TAPER_EXT_IDLE;
      els_taper_int_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_taper_int.state == ELS_TAPER_EXT_SET_XAXES) {
        els_stepper_zero_x();
        els_dro_zero_x();
        els_taper_int_display_axes();
      }
      break;
    case ELS_KEY_SET_ZX:
      els_taper_int.state = ELS_TAPER_EXT_SET_ZAXES;
      els_taper_int_display_axes();
      break;
    default:
      els_taper_int_xjog();
      break;
  }
}

// ----------------------------------------------------------------------------------
// Manual Jog
// ----------------------------------------------------------------------------------
static void els_taper_int_zjog(void) {
  double delta;
  int32_t encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_taper_int.encoder_pos != encoder_curr) {
    delta = (encoder_curr - els_taper_int.encoder_pos) * 0.01 * els_taper_int.encoder_multiplier;
    els_taper_int.encoder_pos = encoder_curr;
    els_stepper_move_z(delta, els_config->z_jog_mm_s);
  }
}

static void els_taper_int_xjog(void) {
  double delta;
  int32_t encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_taper_int.encoder_pos != encoder_curr) {
    // ----------------------------------------------------------------------------------
    // Jog pulse calculation
    // ----------------------------------------------------------------------------------
    delta = (encoder_curr - els_taper_int.encoder_pos) * 0.01 * els_taper_int.encoder_multiplier;
    els_taper_int.encoder_pos = encoder_curr;
    els_stepper_move_x(delta, els_config->x_jog_mm_s);
  }
}
