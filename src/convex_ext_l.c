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

#include "convex_ext_l.h"
#include "utils.h"

#define PRECISION 1e-2
//==============================================================================
// Externs
//==============================================================================
extern tft_device_t tft;

extern const tft_font_t noto_sans_mono_bold_arrows_24;
extern const tft_font_t noto_sans_mono_bold_26;
extern const tft_font_t noto_sans_mono_bold_20;
extern const tft_font_t noto_sans_mono_bold_14;
extern const tft_font_t gears_regular_32;
extern const tft_font_t gears_regular_50;
extern const tft_font_t inconsolata_lgc_bold_14;

//==============================================================================
// Config
//==============================================================================
typedef enum {
  ELS_CONVEX_EXT_IDLE       = 1,
  ELS_CONVEX_EXT_PAUSED     = 2,
  ELS_CONVEX_EXT_ACTIVE     = 4,
  ELS_CONVEX_EXT_SET_ZAXES  = 8,
  ELS_CONVEX_EXT_ZJOG       = 16,
  ELS_CONVEX_EXT_SET_XAXES  = 32,
  ELS_CONVEX_EXT_XJOG       = 64,
  ELS_CONVEX_EXT_SET_FEED   = 128,
  ELS_CONVEX_EXT_SET_DOC    = 256,
  ELS_CONVEX_EXT_SET_RADIUS = 512,
  ELS_CONVEX_EXT_SET_DEPTH  = 1024,
  ELS_CONVEX_EXT_SET_LENGTH = 2048
} els_convex_ext_l_state_t;

typedef enum {
  ELS_CONVEX_EXT_OP_NA      = -1,
  ELS_CONVEX_EXT_OP_IDLE    = 0,
  ELS_CONVEX_EXT_OP_READY   = 1,
  ELS_CONVEX_EXT_OP_MOVEZ0  = 2,
  ELS_CONVEX_EXT_OP_MOVEX0  = 3,
  ELS_CONVEX_EXT_OP_START   = 4,
  ELS_CONVEX_EXT_OP_FEED    = 5,
  ELS_CONVEX_EXT_OP_PLAN    = 6,
  ELS_CONVEX_EXT_OP_TURNING = 7,
  ELS_CONVEX_EXT_OP_RESETZ  = 8,
  ELS_CONVEX_EXT_OP_RESETX  = 9,
  ELS_CONVEX_EXT_OP_DONE    = 10
} els_convex_ext_l_op_state_t;

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
#define ELS_CONVEX_EXT_FEED_MIN  (100)
#define ELS_CONVEX_EXT_FEED_MAX  (6000)

#define ELS_CONVEX_EXT_DOC_MIN   (10)
#define ELS_CONVEX_EXT_DOC_MAX   (2000)

static struct {
  int32_t  feed_um;
  double   feed_mm_s;

  // not supported
  bool     feed_reverse;

  int32_t  depth_of_cut_um;

  bool     locked;

  uint32_t finish_pass_count;

  double   depth;
  double   length;
  double   radius;

  double   arc_center_z, arc_center_x;

  double   xcurr;

  // input read for jogging etc
  int32_t  encoder_pos;

  // encoder multiplier
  int16_t encoder_multiplier;

  // module state
  els_convex_ext_l_state_t state;

  // tracking variables for display refresh.
  els_convex_ext_l_state_t prev_state;
  els_spindle_direction_t prev_dir;

  // operation state
  els_convex_ext_l_op_state_t op_state;
  els_convex_ext_l_op_state_t prev_op_state;

  // dro
  bool show_dro;

  // only finish pass
  bool only_finish_pass;
} els_convex_ext_l = {
  .depth_of_cut_um = 200,
  .feed_um = 4000,
  .radius = 12.5,
  .length = 5,
  .depth = 5,
  .encoder_multiplier = 1
};

//==============================================================================
// Internal functions
//==============================================================================
static void els_convex_ext_l_run(void);
static void els_convex_ext_l_turn(void);

static void els_convex_ext_l_display_refresh(void);
static void els_convex_ext_l_display_setting(void);
static void els_convex_ext_l_display_axes(void);
static void els_convex_ext_l_display_header(void);
static void els_convex_ext_l_display_diagram(void);
static void els_convex_ext_l_display_encoder_pips(void);

static void els_convex_ext_l_calculate_arc(void);

static void els_convex_ext_l_set_feed(void);
static void els_convex_ext_l_set_depth_of_cut(void);
static void els_convex_ext_l_set_radius(void);
static void els_convex_ext_l_set_depth(void);
static void els_convex_ext_l_set_length(void);

static void els_convex_ext_l_set_zaxes(void);
static void els_convex_ext_l_set_xaxes(void);

static void els_convex_ext_l_keypad_process(void);

// manual jogging
static void els_convex_ext_l_zjog(void);
static void els_convex_ext_l_xjog(void);

//==============================================================================
// API
//==============================================================================
void els_convex_ext_l_setup(void) {
  // NO-OP
}

void els_convex_ext_l_start(void) {
  char text[32];

  tft_filled_rectangle(&tft, 0,   0, 480, 320, ILI9481_BLACK);

  tft_filled_rectangle(&tft, 0,  50, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0,  50, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 100, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 100, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 190, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 190, 480,   1, ILI9481_LITEGRAY);

  els_convex_ext_l_display_header();

  if (!els_convex_ext_l.show_dro)
    els_convex_ext_l_display_diagram();

  // angle symbol
  tft_font_write_bg(&tft, 165,  62, "J", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // pulse symbol
  tft_font_write_bg(&tft, 355,  58, "K", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // reset state
  els_convex_ext_l.prev_state = 0;
  els_convex_ext_l.prev_dir = 0;
  els_convex_ext_l.feed_mm_s = els_convex_ext_l.feed_um / 1000.0;
  els_convex_ext_l_calculate_arc();

  els_convex_ext_l.state = ELS_CONVEX_EXT_IDLE;
  els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_IDLE;
  els_convex_ext_l.prev_op_state = ELS_CONVEX_EXT_OP_NA;

  els_convex_ext_l_display_setting();
  els_convex_ext_l_display_axes();
  els_convex_ext_l_display_refresh();

  els_stepper_start();
}

void els_convex_ext_l_stop(void) {
  els_stepper_stop();
}

bool els_convex_ext_l_busy(void) {
  return els_convex_ext_l.state != ELS_CONVEX_EXT_IDLE;
}

void els_convex_ext_l_update(void) {
  static uint64_t last_refreshed_at = 0;
  uint64_t elapsed = els_timer_elapsed_microseconds();

  bool kp_locked = els_keypad_locked();
  if (els_convex_ext_l.locked != kp_locked) {
    els_convex_ext_l.locked = kp_locked;
    els_convex_ext_l_display_header();
  }

  if (els_convex_ext_l.state & (ELS_CONVEX_EXT_IDLE | ELS_CONVEX_EXT_PAUSED | ELS_CONVEX_EXT_ACTIVE))
    els_convex_ext_l_keypad_process();

  if (els_convex_ext_l.state &
     (ELS_CONVEX_EXT_PAUSED | ELS_CONVEX_EXT_ACTIVE | ELS_CONVEX_EXT_SET_XAXES | ELS_CONVEX_EXT_SET_ZAXES))
    els_stepper_enable();
  else
    els_stepper_disable();

  if (els_convex_ext_l.state == ELS_CONVEX_EXT_IDLE)
    els_stepper_disable();
  else
    els_stepper_enable();

  switch (els_convex_ext_l.state) {
    case ELS_CONVEX_EXT_PAUSED:
    case ELS_CONVEX_EXT_ACTIVE:
      els_convex_ext_l_run();
      break;
    case ELS_CONVEX_EXT_SET_FEED:
      els_convex_ext_l_set_feed();
      break;
    case ELS_CONVEX_EXT_SET_DOC:
      els_convex_ext_l_set_depth_of_cut();
      break;
    case ELS_CONVEX_EXT_SET_RADIUS:
      els_convex_ext_l_set_radius();
      break;
    case ELS_CONVEX_EXT_SET_DEPTH:
      els_convex_ext_l_set_depth();
      break;
    case ELS_CONVEX_EXT_SET_LENGTH:
      els_convex_ext_l_set_length();
      break;
    default:
      if (els_convex_ext_l.state & (ELS_CONVEX_EXT_SET_ZAXES | ELS_CONVEX_EXT_ZJOG))
        els_convex_ext_l_set_zaxes();
      else if (els_convex_ext_l.state & (ELS_CONVEX_EXT_SET_XAXES | ELS_CONVEX_EXT_XJOG))
        els_convex_ext_l_set_xaxes();
      break;
  }

  if (elapsed - last_refreshed_at > 1e5) {
    last_refreshed_at = elapsed;
    els_convex_ext_l_display_refresh();
  }

  int16_t em = els_encoder_get_multiplier();
  if (em != els_convex_ext_l.encoder_multiplier) {
    els_convex_ext_l.encoder_multiplier = em;
    els_convex_ext_l_display_encoder_pips();
  }
}

//==============================================================================
// Internal functions
//==============================================================================

// ----------------------------------------------------------------------------------
// Display functions
// ----------------------------------------------------------------------------------
static void els_convex_ext_l_display_setting(void) {
  char text[32];

  els_sprint_double2(text, sizeof(text), els_convex_ext_l.feed_um / 1000.0, "Zf");
  if (els_convex_ext_l.state == ELS_CONVEX_EXT_SET_FEED)
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double2(text, sizeof(text), els_convex_ext_l.depth_of_cut_um / 1000.0, "Xs");
  if (els_convex_ext_l.state == ELS_CONVEX_EXT_SET_DOC)
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_convex_ext_l.radius, "R");
  if (els_convex_ext_l.state == ELS_CONVEX_EXT_SET_RADIUS)
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_20, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_20, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_convex_ext_l.depth, "D");
  if (els_convex_ext_l.state == ELS_CONVEX_EXT_SET_DEPTH)
    tft_font_write_bg(&tft, 310, 252, text, &noto_sans_mono_bold_20, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 252, text, &noto_sans_mono_bold_20, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_convex_ext_l.length, "L");
  if (els_convex_ext_l.state == ELS_CONVEX_EXT_SET_LENGTH)
    tft_font_write_bg(&tft, 310, 276, text, &noto_sans_mono_bold_20, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 276, text, &noto_sans_mono_bold_20, ILI9481_WHITE, ILI9481_BLACK);

  if (els_convex_ext_l.only_finish_pass) {
    tft_filled_rectangle(&tft, 227, 149, 73, 25, ILI9481_WHITE);
    tft_font_write_bg(&tft, 230, 149, "FINISH", &noto_sans_mono_bold_14, ILI9481_BLACK, ILI9481_WHITE);
  }
  else {
    tft_filled_rectangle(&tft, 227, 149, 80, 30, ILI9481_BLACK);
  }
}

static void els_convex_ext_l_display_axes(void) {
  char text[32];

  els_sprint_double33(text, sizeof(text), els_stepper->zpos, "Z");
  if (els_convex_ext_l.state & ELS_CONVEX_EXT_SET_ZAXES)
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), els_stepper->xpos, "X");
  if (els_convex_ext_l.state & ELS_CONVEX_EXT_SET_XAXES)
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  if (els_convex_ext_l.show_dro) {
    tft_font_write_bg(&tft, 8, 190, "DRO", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.zpos_um / 1000.0), "Z");
    tft_font_write_bg(&tft, 8, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.xpos_um / 1000.0), "X");
    tft_font_write_bg(&tft, 8, 262, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
  }
}

static void els_convex_ext_l_display_diagram(void) {
  // ----------------------------------------------------------------------------------------------
  // diagram
  // ----------------------------------------------------------------------------------------------
  tft_filled_rectangle(&tft, 100, 220, 160, 80, ILI9481_WHITE);
  tft_filled_rectangle(&tft, 160, 220, 40, 80, ILI9481_ORANGE);
  tft_filled_circle(&tft, 195, 260, 40, ILI9481_WHITE);
  tft_filled_rectangle(&tft, 140, 220, 20, 22, ILI9481_BLACK);
  tft_filled_rectangle(&tft, 140, 278, 20, 22, ILI9481_BLACK);

  tft_font_write_bg(&tft, 156, 176, "A", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 176, 192, "L", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 184, 176, "C", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);

  tft_font_write_bg(&tft, 186, 238, "B", &noto_sans_mono_bold_arrows_24, ILI9481_BLUE, ILI9481_WHITE);
  tft_font_write_bg(&tft, 202, 262, "R", &inconsolata_lgc_bold_14, ILI9481_BLUE, ILI9481_WHITE);
  tft_font_write_bg(&tft, 186, 262, "D", &noto_sans_mono_bold_arrows_24, ILI9481_BLUE, ILI9481_WHITE);

  tft_font_write_bg(&tft, 140, 184, "D", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 144, 218, "D", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 140, 226, "B", &noto_sans_mono_bold_arrows_24, ILI9481_BLACK, ILI9481_WHITE);

  // origin
  tft_font_write_bg(&tft, 150, 265, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);

  // legend
  tft_font_write_bg(&tft, 8, 200, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  tft_font_write_bg(&tft, 26, 220, "(0,0)", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  // ----------------------------------------------------------------------------------------------
}

static void els_convex_ext_l_display_header(void) {
  tft_rgb_t color = (els_convex_ext_l.locked ? ILI9481_RED : ILI9481_CERULEAN);
  tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
  tft_font_write_bg(&tft, 8, 0, "CONVEX EXTERNAL - L", &noto_sans_mono_bold_26, ILI9481_WHITE, color);
  els_convex_ext_l_display_encoder_pips();
}

static void els_convex_ext_l_display_encoder_pips(void) {
  // multiplier is 100, 10 or 1
  size_t pips = els_convex_ext_l.encoder_multiplier;
  pips = pips > 10 ? 3 : pips > 1 ? 2 : 1;

  for (size_t n = 0, spacing = 0; n < 3; n++, spacing++)
    tft_filled_rectangle(&tft,
      440, 32 - (n * 10 + spacing * 2),
      15 + n * 10, 10,
      (n < pips ? ILI9481_WHITE : els_convex_ext_l.locked ? ILI9481_LITEGRAY : ILI9481_BGCOLOR1));
}

static void els_convex_ext_l_display_refresh(void) {
  els_spindle_direction_t dir = els_spindle_get_direction();
  if (dir != els_convex_ext_l.prev_dir) {
    els_convex_ext_l.prev_dir = dir;
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

  els_convex_ext_l_display_axes();

  if (els_convex_ext_l.op_state != els_convex_ext_l.prev_op_state) {
    els_convex_ext_l.prev_op_state = els_convex_ext_l.op_state;
    tft_filled_rectangle(&tft, 310, 195, 169, 35, ILI9481_BLACK);
    if (els_convex_ext_l.op_state == ELS_CONVEX_EXT_OP_TURNING && els_convex_ext_l.finish_pass_count > 0) {
      tft_font_write_bg(&tft, 310, 190, "FINISH", &noto_sans_mono_bold_26, ILI9481_CERULEAN, ILI9481_BLACK);
    }
    else {
      const char *label = op_labels[els_convex_ext_l.op_state];
      tft_font_write_bg(&tft, 310, 190, label, &noto_sans_mono_bold_26, ILI9481_CERULEAN, ILI9481_BLACK);
    }
  }
}

// ----------------------------------------------------------------------------------
// Top module level keypad handler.
// ----------------------------------------------------------------------------------
static void els_convex_ext_l_keypad_process(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
      if (els_convex_ext_l.state == ELS_CONVEX_EXT_IDLE) {
        if (els_convex_ext_l.length > 0 && els_convex_ext_l.depth > 0) {
          els_convex_ext_l.state = ELS_CONVEX_EXT_PAUSED;
          els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_READY;
        }
        else {
          els_convex_ext_l.state = ELS_CONVEX_EXT_SET_RADIUS;
          els_convex_ext_l_display_setting();
        }
      }
      break;
    case ELS_KEY_EXIT:
      if (els_convex_ext_l.state & (ELS_CONVEX_EXT_PAUSED | ELS_CONVEX_EXT_ACTIVE)) {
        els_convex_ext_l.state = ELS_CONVEX_EXT_IDLE;
        els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_IDLE;
      }
      break;
    case ELS_KEY_SET_FEED:
      if (els_convex_ext_l.state & (ELS_CONVEX_EXT_IDLE | ELS_CONVEX_EXT_PAUSED)) {
        els_convex_ext_l.state = ELS_CONVEX_EXT_SET_FEED;
        els_convex_ext_l.encoder_pos = 0;
        els_convex_ext_l_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F1:
      if (els_convex_ext_l.state & (ELS_CONVEX_EXT_IDLE | ELS_CONVEX_EXT_PAUSED)) {
        els_convex_ext_l.state = ELS_CONVEX_EXT_SET_RADIUS;
        els_convex_ext_l.encoder_pos = 0;
        els_convex_ext_l_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F2:
      els_convex_ext_l.show_dro = !els_convex_ext_l.show_dro;
      tft_filled_rectangle(&tft, 0, 195, 300, 125, ILI9481_BLACK);
      if (els_convex_ext_l.show_dro)
        els_convex_ext_l_display_axes();
      else
        els_convex_ext_l_display_diagram();
      break;
    case ELS_KEY_REV_FEED:
      if (els_convex_ext_l.state & (ELS_CONVEX_EXT_IDLE | ELS_CONVEX_EXT_PAUSED)) {
        els_convex_ext_l.only_finish_pass = !els_convex_ext_l.only_finish_pass;
        els_convex_ext_l_display_setting();
      }
      break;
    case ELS_KEY_SET_ZX:
      if (els_convex_ext_l.state & (ELS_CONVEX_EXT_IDLE | ELS_CONVEX_EXT_PAUSED)) {
        els_convex_ext_l.state = ELS_CONVEX_EXT_SET_ZAXES;
        els_convex_ext_l.encoder_pos = 0;
        els_convex_ext_l_display_axes();
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
static void els_convex_ext_l_run(void) {
  switch (els_spindle_get_direction()) {
    case ELS_S_DIRECTION_CW:
    case ELS_S_DIRECTION_CCW:
      if (els_convex_ext_l.state == ELS_CONVEX_EXT_PAUSED && els_spindle_get_counter() == 0)
        els_convex_ext_l.state = ELS_CONVEX_EXT_ACTIVE;
      break;
    default:
      if (els_convex_ext_l.state == ELS_CONVEX_EXT_ACTIVE)
        els_convex_ext_l.state = ELS_CONVEX_EXT_PAUSED;
      break;
  }

  if (els_convex_ext_l.state == ELS_CONVEX_EXT_ACTIVE)
    els_convex_ext_l_turn();
}

static void els_convex_ext_l_turn(void) {
  double xd, remaining, ztarget;

  switch (els_convex_ext_l.op_state) {
    case ELS_CONVEX_EXT_OP_NA:
    case ELS_CONVEX_EXT_OP_IDLE:
      break;
    case ELS_CONVEX_EXT_OP_READY:
      els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_MOVEZ0;
      els_stepper_sync();
      break;
    case ELS_CONVEX_EXT_OP_MOVEZ0:
      if (els_stepper->zbusy)
        break;

      if (fabs(els_stepper->zpos) > PRECISION)
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_retract_jog_mm_s);
      else
        els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_MOVEX0;
      break;
    case ELS_CONVEX_EXT_OP_MOVEX0:
      if (els_stepper->xbusy)
        break;

      if (fabs(els_stepper->xpos) > PRECISION)
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_retract_jog_mm_s);
      else
        els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_START;
      break;
    case ELS_CONVEX_EXT_OP_START:
      if (els_stepper->xbusy)
        break;

      els_convex_ext_l.finish_pass_count = 0;
      if (els_convex_ext_l.only_finish_pass) {
        els_stepper_move_x(-els_convex_ext_l.depth - els_stepper->xpos + 0.1, els_config->x_retract_jog_mm_s);
        els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_FEED;
      }
      else {
        els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_PLAN;
      }
      break;
    case ELS_CONVEX_EXT_OP_FEED:
      if (els_stepper->xbusy || els_stepper->zbusy)
        break;

      els_convex_ext_l.xcurr = els_stepper->xpos;
      remaining = fabs(els_convex_ext_l.xcurr + els_convex_ext_l.depth);
      xd = MIN(els_convex_ext_l.depth_of_cut_um / 1000.0, remaining);
      if (remaining <= (els_convex_ext_l.depth_of_cut_um / 1000.0))
        els_convex_ext_l.finish_pass_count++;

      els_stepper_move_x(-xd, els_config->x_retract_jog_mm_s);
      els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_PLAN;
      break;
    case ELS_CONVEX_EXT_OP_PLAN:
      if (els_stepper->xbusy)
        break;

      els_convex_ext_l.xcurr = els_stepper->xpos;
      els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_TURNING;
      if (els_convex_ext_l.finish_pass_count > 0) {
        els_stepper_move_arc_q3_ccw(
          els_convex_ext_l.arc_center_z,
          els_convex_ext_l.arc_center_x,
          els_convex_ext_l.radius,
          els_convex_ext_l.depth,
          els_convex_ext_l.feed_mm_s / 4.0
        );
      }
      else {
        ztarget = -sqrt(SQR(els_convex_ext_l.radius) - SQR(els_stepper->xpos - els_convex_ext_l.arc_center_x)) +
                   els_convex_ext_l.arc_center_z;
        els_stepper_move_z(ztarget - els_stepper->zpos - 0.25, els_convex_ext_l.feed_mm_s);
      }
      break;
    case ELS_CONVEX_EXT_OP_TURNING:
      if (els_stepper->xbusy || els_stepper->zbusy)
        break;

      els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_RESETZ;
      els_stepper_move_z(0 - els_stepper->zpos, els_config->z_retract_jog_mm_s);
      break;
    case ELS_CONVEX_EXT_OP_RESETZ:
      if (els_stepper->zbusy)
        break;

      if (els_convex_ext_l.finish_pass_count > 0) {
        els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_DONE;
      }
      else {
        els_stepper_move_x(els_convex_ext_l.xcurr - els_stepper->xpos, els_config->x_retract_jog_mm_s);
        els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_RESETX;
      }
      break;
    case ELS_CONVEX_EXT_OP_RESETX:
      if (els_stepper->xbusy)
        break;
      els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_FEED;
      break;
    case ELS_CONVEX_EXT_OP_DONE:
      // beer time
      if (els_stepper->xbusy || els_stepper->zbusy)
        break;

      els_convex_ext_l.op_state = ELS_CONVEX_EXT_OP_IDLE;
      els_convex_ext_l.state = ELS_CONVEX_EXT_IDLE;
      els_convex_ext_l.finish_pass_count = 0;
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.1: feed settings.
// ----------------------------------------------------------------------------------
static void els_convex_ext_l_set_feed(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_convex_ext_l.state = ELS_CONVEX_EXT_IDLE;
      els_convex_ext_l_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_convex_ext_l.state = ELS_CONVEX_EXT_SET_DOC;
      els_convex_ext_l_display_setting();
      break;
    case ELS_KEY_REV_FEED:
      // TODO
      els_convex_ext_l_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_convex_ext_l.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_convex_ext_l.encoder_pos) * 10 * els_convex_ext_l.encoder_multiplier;
        if (els_convex_ext_l.feed_um + delta <= ELS_CONVEX_EXT_FEED_MIN)
          els_convex_ext_l.feed_um = ELS_CONVEX_EXT_FEED_MIN;
        else if (els_convex_ext_l.feed_um + delta >= ELS_CONVEX_EXT_FEED_MAX)
          els_convex_ext_l.feed_um = ELS_CONVEX_EXT_FEED_MAX;
        else
          els_convex_ext_l.feed_um += delta;
        els_convex_ext_l.encoder_pos = encoder_curr;
        els_convex_ext_l_display_setting();
        els_convex_ext_l.feed_mm_s = els_convex_ext_l.feed_um / 1000.0;
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.2: depth of cut settings.
// ----------------------------------------------------------------------------------
static void els_convex_ext_l_set_depth_of_cut(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_convex_ext_l.state = ELS_CONVEX_EXT_IDLE;
      els_convex_ext_l_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_convex_ext_l.state = ELS_CONVEX_EXT_SET_FEED;
      els_convex_ext_l_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_convex_ext_l.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_convex_ext_l.encoder_pos) * 10 * els_convex_ext_l.encoder_multiplier;
        if (els_convex_ext_l.depth_of_cut_um + delta <= ELS_CONVEX_EXT_DOC_MIN)
          els_convex_ext_l.depth_of_cut_um = ELS_CONVEX_EXT_DOC_MIN;
        else if (els_convex_ext_l.depth_of_cut_um + delta >= ELS_CONVEX_EXT_DOC_MAX)
          els_convex_ext_l.depth_of_cut_um = ELS_CONVEX_EXT_DOC_MAX;
        else
          els_convex_ext_l.depth_of_cut_um += delta;
        els_convex_ext_l.encoder_pos = encoder_curr;
        els_convex_ext_l_display_setting();
      }
      break;
  }
}

void els_convex_ext_l_set_radius(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_convex_ext_l.state = ELS_CONVEX_EXT_IDLE;
      els_convex_ext_l_display_setting();
      els_convex_ext_l_calculate_arc();
      break;
    case ELS_KEY_FUN_F1:
      els_convex_ext_l.state = ELS_CONVEX_EXT_SET_DEPTH;
      els_convex_ext_l_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_convex_ext_l.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_convex_ext_l.encoder_pos) * (0.01 * els_convex_ext_l.encoder_multiplier);
        if (els_convex_ext_l.radius + delta <= 0)
          els_convex_ext_l.radius = 0;
        else if (els_convex_ext_l.radius + delta >= ELS_X_MAX_MM)
          els_convex_ext_l.radius = ELS_X_MAX_MM;
        else
          els_convex_ext_l.radius += delta;
        els_convex_ext_l.encoder_pos = encoder_curr;
        if (els_convex_ext_l.depth > els_convex_ext_l.radius)
          els_convex_ext_l.depth = els_convex_ext_l.radius;
        if (els_convex_ext_l.length > els_convex_ext_l.radius)
          els_convex_ext_l.length = els_convex_ext_l.radius;

        // re-adjust length.
        els_convex_ext_l_calculate_arc();
        if (els_convex_ext_l.length > fabs(els_convex_ext_l.arc_center_z)) {
          els_convex_ext_l.length = fabs(els_convex_ext_l.arc_center_z);
          els_convex_ext_l_calculate_arc();
        }

        els_convex_ext_l_display_setting();
      }
      break;
  }
}

void els_convex_ext_l_set_depth(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_convex_ext_l.state = ELS_CONVEX_EXT_IDLE;
      els_convex_ext_l_display_setting();
      els_convex_ext_l_calculate_arc();
      break;
    case ELS_KEY_FUN_F1:
      els_convex_ext_l.state = ELS_CONVEX_EXT_SET_LENGTH;
      els_convex_ext_l_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_convex_ext_l.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_convex_ext_l.encoder_pos) * (0.01 * els_convex_ext_l.encoder_multiplier);
        if (els_convex_ext_l.depth + delta <= 0)
          els_convex_ext_l.depth = 0;
        else if (els_convex_ext_l.depth + delta >= els_convex_ext_l.radius)
          els_convex_ext_l.depth = els_convex_ext_l.radius;
        else
          els_convex_ext_l.depth += delta;
        els_convex_ext_l.encoder_pos = encoder_curr;
        els_convex_ext_l_display_setting();
      }
      break;
  }
}

void els_convex_ext_l_set_length(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_convex_ext_l.state = ELS_CONVEX_EXT_IDLE;
      els_convex_ext_l_display_setting();
      els_convex_ext_l_calculate_arc();
      break;
    case ELS_KEY_FUN_F1:
      els_convex_ext_l.state = ELS_CONVEX_EXT_SET_RADIUS;
      els_convex_ext_l_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_convex_ext_l.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_convex_ext_l.encoder_pos) * (0.01 * els_convex_ext_l.encoder_multiplier);
        if (els_convex_ext_l.length + delta <= 0)
          els_convex_ext_l.length = 0;
        else if (els_convex_ext_l.length + delta >= els_convex_ext_l.radius)
          els_convex_ext_l.length = els_convex_ext_l.radius;
        else
          els_convex_ext_l.length += delta;
        els_convex_ext_l.encoder_pos = encoder_curr;

        // re-adjust length.
        els_convex_ext_l_calculate_arc();
        if (els_convex_ext_l.length > fabs(els_convex_ext_l.arc_center_z)) {
          els_convex_ext_l.length = fabs(els_convex_ext_l.arc_center_z);
          els_convex_ext_l_calculate_arc();
        }

        els_convex_ext_l_display_setting();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 3: Axis - position, origin & jogging
// ----------------------------------------------------------------------------------
static void els_convex_ext_l_set_zaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_convex_ext_l.state = (els_convex_ext_l.state & ELS_CONVEX_EXT_ZJOG) ? ELS_CONVEX_EXT_SET_ZAXES : ELS_CONVEX_EXT_IDLE;
      els_convex_ext_l_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_convex_ext_l.state == ELS_CONVEX_EXT_SET_ZAXES) {
        els_stepper_zero_z();
        els_dro_zero_z();
        els_convex_ext_l_display_axes();
      }
      break;
    case ELS_KEY_JOG_ZX_ORI:
      if (!els_stepper->zbusy)
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);
      break;
    case ELS_KEY_SET_ZX:
      els_convex_ext_l.state = ELS_CONVEX_EXT_SET_XAXES;
      els_convex_ext_l_display_axes();
      break;
    default:
      els_convex_ext_l_zjog();
      break;
  }
}

static void els_convex_ext_l_set_xaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_convex_ext_l.state = ELS_CONVEX_EXT_IDLE;
      els_convex_ext_l_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_convex_ext_l.state == ELS_CONVEX_EXT_SET_XAXES) {
        els_stepper_zero_x();
        els_dro_zero_x();
        els_convex_ext_l_display_axes();
      }
      break;
    case ELS_KEY_JOG_ZX_ORI:
      if (!els_stepper->xbusy)
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_jog_mm_s);
      break;
    case ELS_KEY_SET_ZX:
      els_convex_ext_l.state = ELS_CONVEX_EXT_SET_ZAXES;
      els_convex_ext_l_display_axes();
      break;
    default:
      els_convex_ext_l_xjog();
      break;
  }
}

// ----------------------------------------------------------------------------------
// Manual Jog
// ----------------------------------------------------------------------------------
static void els_convex_ext_l_zjog(void) {
  double delta, step;
  int32_t encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_convex_ext_l.encoder_pos != encoder_curr) {
    step = els_convex_ext_l.encoder_multiplier == 1 ? 0.005 : 0.01 * els_convex_ext_l.encoder_multiplier;
    delta = (encoder_curr - els_convex_ext_l.encoder_pos) * step;
    els_convex_ext_l.encoder_pos = encoder_curr;
    els_stepper_move_z(delta, els_config->z_jog_mm_s);
  }
}

static void els_convex_ext_l_xjog(void) {
  double delta, step;
  int32_t encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_convex_ext_l.encoder_pos != encoder_curr) {
    step = els_convex_ext_l.encoder_multiplier == 1 ? 0.005 : 0.01 * els_convex_ext_l.encoder_multiplier;
    delta = (encoder_curr - els_convex_ext_l.encoder_pos) * step;
    els_convex_ext_l.encoder_pos = encoder_curr;
    els_stepper_move_x(delta, els_config->x_jog_mm_s);
  }
}

// ----------------------------------------------------------------------------------
// Calculate arc center (z, x)
// ----------------------------------------------------------------------------------
void els_convex_ext_l_calculate_arc(void) {
  // midpoint distance.
  double dz = els_convex_ext_l.length / 2.0;
  double dx = els_convex_ext_l.depth / 2.0;

  // midpoint z,x coordinates.
  double mz = +dz;
  double mx = -dx;

  // rhombus diagonal 1
  double d1 = sqrt(SQR(dz) + SQR(dx));
  // rhombus diagonal 2
  double d2 = sqrt(SQR(els_convex_ext_l.radius) - SQR(d1));

  // circle origin
  els_convex_ext_l.arc_center_z = mz + (d2 * dx) / d1;
  els_convex_ext_l.arc_center_x = mx - (d2 * dz) / d1;

  printf("Cz = %.2f Cx = %.2f\n", els_convex_ext_l.arc_center_z, els_convex_ext_l.arc_center_x);
}
