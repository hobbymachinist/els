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
#include "turn_dimension.h"
#include "utils.h"

#define PRECISION       (1e-2)
#define BACKOFF_DEPTH   1

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
  ELS_TURN_DIM_IDLE      = 1,
  ELS_TURN_DIM_PAUSED    = 2,
  ELS_TURN_DIM_ACTIVE    = 4,
  ELS_TURN_DIM_SET_ZAXES = 8,
  ELS_TURN_DIM_ZJOG      = 16,
  ELS_TURN_DIM_SET_XAXES = 32,
  ELS_TURN_DIM_XJOG      = 64,
  ELS_TURN_DIM_SET_FEED  = 128,
  ELS_TURN_DIM_SET_FFEED = 256,
  ELS_TURN_DIM_SET_DOC   = 512,
  ELS_TURN_DIM_SET_FDOC  = 1024,
  ELS_TURN_DIM_SET_LEN   = 2048,
  ELS_TURN_DIM_SET_DEPTH = 4096
} els_turn_dimension_state_t;

typedef enum {
  ELS_TURN_DIM_OP_IDLE    = 1,
  ELS_TURN_DIM_OP_READY   = 2,
  ELS_TURN_DIM_OP_MOVEZ0  = 3,
  ELS_TURN_DIM_OP_MOVEX0  = 4,
  ELS_TURN_DIM_OP_START   = 5,
  ELS_TURN_DIM_OP_ATZ0    = 6,
  ELS_TURN_DIM_OP_ATZL    = 7,
  ELS_TURN_DIM_OP_ATZLB   = 8,
  ELS_TURN_DIM_OP_ATZ0B   = 9,
  ELS_TURN_DIM_OP_FEED_IN = 10,
  ELS_TURN_DIM_OP_SPRING  = 11,
  ELS_TURN_DIM_OP_DONE    = 12
} els_turn_dimension_op_state_t;

static const char *op_labels[] = {
  "N/A    ",
  "CONFIG ",
  "READY  ",
  "MOVE Z0",
  "MOVE X0",
  "ORIGIN ",
  "FEED IN",
  "TURNING",
  "BACKOFF",
  "RETURN ",
  "FEED IN",
  "SPRING ",
  "DONE   "
};

//==============================================================================
// Internal state
//==============================================================================
#define ELS_TURN_DIM_FEED_MIN  (100)
#define ELS_TURN_DIM_FEED_MAX  (6000)

#define ELS_TURN_DIM_DOC_MIN   (50)
#define ELS_TURN_DIM_DOC_MAX   (2000)

static struct {
  int32_t  feed_um;
  double   feed_mm_s;

  int32_t  finish_feed_um;
  double   finish_feed_mm_s;

  // not supported yet.
  bool     feed_reverse;

  int32_t  depth_of_cut_um;
  int32_t  finish_depth_um;

  bool     locked;

  double   length;
  double   depth;

  uint8_t  spring_pass_count;
  uint8_t  spring_passes;

  // jogging state
  int32_t  encoder_pos;

  // encoder multiplier
  int16_t encoder_multiplier;

  // module state
  els_turn_dimension_state_t state;

  // tracking variables for display refresh.
  els_turn_dimension_op_state_t prev_op_state;
  els_turn_dimension_state_t prev_state;
  els_spindle_direction_t prev_dir;

  // operation state
  els_turn_dimension_op_state_t op_state;

  // saved xpos
  double xpos_prev;

  // dro
  bool show_dro;
} els_turn_dimension = {
  .depth_of_cut_um = 250,
  .finish_depth_um = 250,
  .feed_um = 4000,
  .finish_feed_um = 2000,
  .length = 20,
  .depth = 1.00,
  .spring_pass_count = 0,
  .spring_passes = 1,
  .encoder_multiplier = 1
};

//==============================================================================
// Internal functions
//==============================================================================
static void els_turn_dimension_run(void);
static void els_turn_dimension_turn(void);

static void els_turn_dimension_display_refresh(void);
static void els_turn_dimension_display_setting(void);
static void els_turn_dimension_display_axes(void);
static void els_turn_dimension_display_header(void);
static void els_turn_dimension_display_diagram(void);
static void els_turn_dimension_display_encoder_pips(void);

static void els_turn_dimension_set_feed(void);
static void els_turn_dimension_set_finish_feed(void);

static void els_turn_dimension_set_depth_of_cut(void);
static void els_turn_dimension_set_finish_depth(void);

static void els_turn_dimension_set_length(void);
static void els_turn_dimension_set_depth(void);

static void els_turn_dimension_set_zaxes(void);
static void els_turn_dimension_set_xaxes(void);

static void els_turn_dimension_configure_gpio(void);
static void els_turn_dimension_configure_timer(void);

static void els_turn_dimension_keypad_process(void);

// manual jogging
static void els_turn_dimension_zjog(void);
static void els_turn_dimension_xjog(void);

//==============================================================================
// API
//==============================================================================
void els_turn_dimension_setup(void) {
  // NO-OP
}

void els_turn_dimension_start(void) {
  char text[32];

  tft_filled_rectangle(&tft, 0,   0, 480, 320, ILI9481_BLACK);

  tft_filled_rectangle(&tft, 0,  50, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0,  50, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 100, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 100, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 190, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 190, 480,   1, ILI9481_LITEGRAY);

  els_turn_dimension_display_header();

  // angle symbol
  tft_font_write_bg(&tft, 165,  62, "J", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // pulse symbol
  tft_font_write_bg(&tft, 355,  58, "K", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // reset state
  els_turn_dimension.feed_mm_s = els_turn_dimension.feed_um / 1000.0;
  els_turn_dimension.finish_feed_mm_s = els_turn_dimension.finish_feed_um / 1000.0;
  els_turn_dimension.spring_pass_count = 0;
  els_turn_dimension.prev_op_state = 0;
  els_turn_dimension.prev_state = 0;
  els_turn_dimension.prev_dir = 0;
  els_turn_dimension.state = ELS_TURN_DIM_IDLE;
  els_turn_dimension.op_state = ELS_TURN_DIM_OP_IDLE;

  if (!els_turn_dimension.show_dro)
    els_turn_dimension_display_diagram();

  els_turn_dimension.state = ELS_TURN_DIM_IDLE;

  els_turn_dimension_display_setting();
  els_turn_dimension_display_axes();
  els_turn_dimension_display_refresh();

  els_stepper_start();
}

void els_turn_dimension_stop(void) {
  els_stepper_stop();
}

bool els_turn_dimension_busy(void) {
  return els_turn_dimension.state != ELS_TURN_DIM_IDLE;
}

void els_turn_dimension_update(void) {
  static uint64_t last_refreshed_at = 0;

  uint64_t elapsed = els_timer_elapsed_microseconds();

  bool kp_locked = els_keypad_locked();
  if (els_turn_dimension.locked != kp_locked) {
    els_turn_dimension.locked = kp_locked;
    els_turn_dimension_display_header();
  }

  if (els_turn_dimension.state & (ELS_TURN_DIM_IDLE | ELS_TURN_DIM_PAUSED | ELS_TURN_DIM_ACTIVE))
    els_turn_dimension_keypad_process();

  switch (els_turn_dimension.state) {
    case ELS_TURN_DIM_PAUSED:
    case ELS_TURN_DIM_ACTIVE:
      els_turn_dimension_run();
      break;
    case ELS_TURN_DIM_SET_FEED:
      els_turn_dimension_set_feed();
      break;
    case ELS_TURN_DIM_SET_FFEED:
      els_turn_dimension_set_finish_feed();
      break;
    case ELS_TURN_DIM_SET_DOC:
      els_turn_dimension_set_depth_of_cut();
      break;
    case ELS_TURN_DIM_SET_FDOC:
      els_turn_dimension_set_finish_depth();
      break;
    case ELS_TURN_DIM_SET_LEN:
      els_turn_dimension_set_length();
      break;
    case ELS_TURN_DIM_SET_DEPTH:
      els_turn_dimension_set_depth();
      break;
    default:
      if (els_turn_dimension.state & (ELS_TURN_DIM_SET_ZAXES | ELS_TURN_DIM_ZJOG))
        els_turn_dimension_set_zaxes();
      else if (els_turn_dimension.state & (ELS_TURN_DIM_SET_XAXES | ELS_TURN_DIM_XJOG))
        els_turn_dimension_set_xaxes();
      break;
  }

  if (elapsed - last_refreshed_at > 1e5) {
    last_refreshed_at = elapsed;
    els_turn_dimension_display_refresh();
  }

  int16_t em = els_encoder_get_multiplier();
  if (em != els_turn_dimension.encoder_multiplier) {
    els_turn_dimension.encoder_multiplier = em;
    els_turn_dimension_display_encoder_pips();
  }
}

//==============================================================================
// Internal functions
//==============================================================================

// ----------------------------------------------------------------------------------
// Display functions
// ----------------------------------------------------------------------------------
static void els_turn_dimension_display_setting(void) {
  char text[32];

  els_sprint_double1(text, sizeof(text), els_turn_dimension.feed_mm_s, "Zf");
  if (els_turn_dimension.state == ELS_TURN_DIM_SET_FEED)
    tft_font_write_bg(&tft, 228, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 228, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  tft_font_write_bg(&tft, 375, 102, "/", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
  els_sprint_double1(text, sizeof(text), els_turn_dimension.finish_feed_mm_s, NULL);
  if (els_turn_dimension.state == ELS_TURN_DIM_SET_FFEED)
    tft_font_write_bg(&tft, 395, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 395, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double1(text, sizeof(text), els_turn_dimension.depth_of_cut_um / 1000.0, "Xs");
  if (els_turn_dimension.state == ELS_TURN_DIM_SET_DOC)
    tft_font_write_bg(&tft, 228, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 228, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  tft_font_write_bg(&tft, 375, 135, "/", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
  els_sprint_double1(text, sizeof(text), els_turn_dimension.finish_depth_um / 1000.0, NULL);
  if (els_turn_dimension.state == ELS_TURN_DIM_SET_FDOC)
    tft_font_write_bg(&tft, 395, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 395, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_turn_dimension.length, "L");
  if (els_turn_dimension.state == ELS_TURN_DIM_SET_LEN)
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_turn_dimension.depth, "D");
  if (els_turn_dimension.state == ELS_TURN_DIM_SET_DEPTH)
    tft_font_write_bg(&tft, 310, 262, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 262, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
}

static void els_turn_dimension_display_axes(void) {
  char text[32];

  els_sprint_double33(text, sizeof(text), els_stepper->zpos, "Z");

  if (els_turn_dimension.state & ELS_TURN_DIM_SET_ZAXES)
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), els_stepper->xpos, "X");
  if (els_turn_dimension.state & ELS_TURN_DIM_SET_XAXES)
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  if (els_turn_dimension.show_dro) {
    tft_font_write_bg(&tft, 8, 190, "DRO", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.zpos_um / 1000.0), "Z");
    tft_font_write_bg(&tft, 8, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.xpos_um / 1000.0), "X");
    tft_font_write_bg(&tft, 8, 262, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
  }
}

static void els_turn_dimension_display_diagram(void) {
  // ----------------------------------------------------------------------------------------------
  // diagram
  // ----------------------------------------------------------------------------------------------
  tft_filled_rectangle(&tft, 100, 220, 156, 80, ILI9481_WHITE);
  tft_filled_rectangle(&tft, 156, 220, 100, 20, ILI9481_ORANGE);
  tft_filled_rectangle(&tft, 156, 280, 100, 20, ILI9481_ORANGE);

  tft_font_write_bg(&tft, 156, 178, "A", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 205, 194, "L", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 240, 178, "C", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);

  tft_font_write_bg(&tft, 260, 184, "D", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 280, 216, "D", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 260, 224, "B", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);

  // origin
  tft_font_write_bg(&tft, 250, 265, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);

  // legend
  tft_font_write_bg(&tft, 8, 200, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  tft_font_write_bg(&tft, 26, 220, "(0,0)", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  // ----------------------------------------------------------------------------------------------
}

static void els_turn_dimension_display_header(void) {
  tft_rgb_t color = (els_turn_dimension.locked ? ILI9481_RED : ILI9481_CERULEAN);

  tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
  tft_font_write_bg(&tft, 8, 0, "TURNING", &noto_sans_mono_bold_26, ILI9481_WHITE, color);
  els_turn_dimension_display_encoder_pips();
}

static void els_turn_dimension_display_encoder_pips(void) {
  // multiplier is 100, 10 or 1
  size_t pips = els_turn_dimension.encoder_multiplier;
  pips = pips > 10 ? 3 : pips > 1 ? 2 : 1;

  for (size_t n = 0, spacing = 0; n < 3; n++, spacing++)
    tft_filled_rectangle(&tft,
      440, 32 - (n * 10 + spacing * 2),
      15 + n * 10, 10,
      (n < pips ? ILI9481_WHITE : els_turn_dimension.locked ? ILI9481_LITEGRAY : ILI9481_BGCOLOR1));
}

static void els_turn_dimension_display_refresh(void) {
  els_spindle_direction_t dir = els_spindle_get_direction();
  if (dir != els_turn_dimension.prev_dir) {
    els_turn_dimension.prev_dir = dir;
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

  els_turn_dimension_display_axes();

  if (els_turn_dimension.op_state != els_turn_dimension.prev_op_state) {
    els_turn_dimension.prev_op_state = els_turn_dimension.op_state;

    static bool spring_pass_shown = false;
    if (els_turn_dimension.spring_pass_count > 0) {
      if (!spring_pass_shown) {
        spring_pass_shown = true;
        tft_filled_rectangle(&tft, 310, 195, 169, 35, ILI9481_BLACK);
        tft_font_write_bg(&tft, 310, 190, "SPRING", &noto_sans_mono_bold_26, ILI9481_CERULEAN, ILI9481_BLACK);
      }
    }
    else {
      const char *label = op_labels[els_turn_dimension.op_state];
      tft_filled_rectangle(&tft, 310, 195, 169, 35, ILI9481_BLACK);
      tft_font_write_bg(&tft, 310, 190, label, &noto_sans_mono_bold_26, ILI9481_CERULEAN, ILI9481_BLACK);
      spring_pass_shown = false;
    }
  }
}

// ----------------------------------------------------------------------------------
// Top module level keypad handler.
// ----------------------------------------------------------------------------------
static void els_turn_dimension_keypad_process(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
      if (els_turn_dimension.state == ELS_TURN_DIM_IDLE) {
        if (els_turn_dimension.length > 0 && els_turn_dimension.depth > 0) {
          els_turn_dimension.state = ELS_TURN_DIM_PAUSED;
          els_turn_dimension.op_state = ELS_TURN_DIM_OP_READY;
        }
        else {
          els_turn_dimension.state = ELS_TURN_DIM_SET_LEN;
          els_turn_dimension_display_setting();
        }
      }
      break;
    case ELS_KEY_EXIT:
      if (els_turn_dimension.state & (ELS_TURN_DIM_PAUSED | ELS_TURN_DIM_ACTIVE)) {
        els_turn_dimension.state = ELS_TURN_DIM_IDLE;
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_IDLE;
      }
      break;
    case ELS_KEY_SET_FEED:
      if (els_turn_dimension.state & (ELS_TURN_DIM_IDLE | ELS_TURN_DIM_PAUSED)) {
        els_turn_dimension.state = ELS_TURN_DIM_SET_FEED;
        els_turn_dimension.encoder_pos = 0;
        els_turn_dimension_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F1:
      if (els_turn_dimension.state & (ELS_TURN_DIM_IDLE | ELS_TURN_DIM_PAUSED)) {
        els_turn_dimension.state = ELS_TURN_DIM_SET_LEN;
        els_turn_dimension.encoder_pos = 0;
        els_turn_dimension_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F2:
      els_turn_dimension.show_dro = !els_turn_dimension.show_dro;
      tft_filled_rectangle(&tft, 0, 200, 300, 120, ILI9481_BLACK);
      if (els_turn_dimension.show_dro)
        els_turn_dimension_display_axes();
      else
        els_turn_dimension_display_diagram();
      break;
    case ELS_KEY_SET_ZX:
      if (els_turn_dimension.state & (ELS_TURN_DIM_IDLE | ELS_TURN_DIM_PAUSED)) {
        els_turn_dimension.state = ELS_TURN_DIM_SET_ZAXES;
        els_turn_dimension.encoder_pos = 0;
        els_turn_dimension_display_axes();
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
static void els_turn_dimension_run(void) {
  switch (els_spindle_get_direction()) {
    case ELS_S_DIRECTION_CW:
    case ELS_S_DIRECTION_CCW:
      if (els_turn_dimension.state == ELS_TURN_DIM_PAUSED) {
        if (!els_stepper->zbusy && !els_stepper->xbusy && els_spindle_get_counter() == 0)
          els_turn_dimension.state = ELS_TURN_DIM_ACTIVE;
      }
      break;
    default:
      if (els_turn_dimension.state == ELS_TURN_DIM_ACTIVE)
        els_turn_dimension.state = ELS_TURN_DIM_PAUSED;
      break;
  }

  if (els_turn_dimension.state == ELS_TURN_DIM_ACTIVE)
    els_turn_dimension_turn();
}

static void els_turn_dimension_turn(void) {
  double xd, remaining;

  switch (els_turn_dimension.op_state) {
    case ELS_TURN_DIM_OP_IDLE:
      break;
    case ELS_TURN_DIM_OP_READY:
      els_turn_dimension.op_state = ELS_TURN_DIM_OP_MOVEZ0;
      els_turn_dimension.spring_pass_count = 0;
      els_stepper_sync();
      break;
    case ELS_TURN_DIM_OP_MOVEZ0:
      if (els_stepper->zbusy)
        break;
      if (fabs(els_stepper->zpos) > PRECISION) {
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_retract_jog_mm_s);
      }
      else
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_MOVEX0;
      break;
    case ELS_TURN_DIM_OP_MOVEX0:
      if (els_stepper->xbusy)
        break;
      if (fabs(els_stepper->xpos) > PRECISION)
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_retract_jog_mm_s);
      else
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_START;
      break;
    case ELS_TURN_DIM_OP_START:
      // initial move - scratch pass.
      // els_stepper_move_z_no_accel(-els_turn_dimension.length, els_turn_dimension.feed_mm_s);
      // els_turn_dimension.op_state = ELS_TURN_DIM_OP_ATZL;
      els_turn_dimension.op_state = ELS_TURN_DIM_OP_ATZ0;
      break;
    case ELS_TURN_DIM_OP_ATZL:
      if (els_stepper->zbusy)
        break;

      // backoff
      els_turn_dimension.xpos_prev = els_stepper->xpos;
      els_stepper_move_x_no_accel(BACKOFF_DEPTH, els_turn_dimension.feed_mm_s);
      els_turn_dimension.op_state = ELS_TURN_DIM_OP_ATZLB;
      break;
    case ELS_TURN_DIM_OP_ATZLB:
      if (els_stepper->xbusy)
        break;
      // move to Z=0
      els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);

      if (els_turn_dimension.spring_pass_count >= els_turn_dimension.spring_passes) {
        els_stepper_move_x(0 - els_stepper->xpos, els_turn_dimension.feed_mm_s);
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_DONE;
      }
      else {
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_ATZ0B;
      }
      break;
    case ELS_TURN_DIM_OP_ATZ0B:
      if (els_stepper->zbusy || els_stepper->xbusy)
        break;

      if (fabs(els_stepper->xpos - els_turn_dimension.xpos_prev) >= PRECISION)
        els_stepper_move_x(els_turn_dimension.xpos_prev - els_stepper->xpos, els_turn_dimension.feed_mm_s);
      else
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_ATZ0;
      break;
    case ELS_TURN_DIM_OP_ATZ0:
      if (els_stepper->xbusy)
        break;

      remaining = fabs(els_turn_dimension.depth + els_stepper->xpos);
      if (remaining >= (els_turn_dimension.depth_of_cut_um + els_turn_dimension.finish_depth_um) / 1000.0) {
        xd = (els_turn_dimension.depth_of_cut_um / 1000.0);
        els_stepper_move_x(-xd, els_turn_dimension.feed_mm_s);
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_FEED_IN;
      }
      else if (remaining > (els_turn_dimension.finish_depth_um / 1000.0) + (PRECISION * 5)) {
        xd = (remaining - (els_turn_dimension.finish_depth_um / 1000.0));
        els_stepper_move_x(-xd, els_turn_dimension.feed_mm_s);
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_FEED_IN;
      }
      else if (remaining > PRECISION) {
        xd = remaining;
        els_stepper_move_x(-xd, els_turn_dimension.feed_mm_s);
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_SPRING;
      }
      else {
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_SPRING;
      }
      break;
    case ELS_TURN_DIM_OP_FEED_IN:
      if (els_stepper->xbusy)
        break;

      if (fabs(els_turn_dimension.depth + els_stepper->xpos) <= PRECISION) {
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_SPRING;
      }
      else {
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_ATZL;
        els_stepper_move_z_no_accel(-els_turn_dimension.length, els_turn_dimension.feed_mm_s);
      }
      break;
    case ELS_TURN_DIM_OP_SPRING:
      if (els_stepper->xbusy)
        break;

      if (els_turn_dimension.spring_pass_count >= els_turn_dimension.spring_passes) {
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_retract_jog_mm_s);
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_DONE;
      }
      else {
        els_turn_dimension.spring_pass_count++;
        els_turn_dimension.op_state = ELS_TURN_DIM_OP_ATZL;
        els_stepper_move_z_no_accel(-els_turn_dimension.length, els_turn_dimension.finish_feed_mm_s);
      }
      break;
    case ELS_TURN_DIM_OP_DONE:
      if (els_stepper->xbusy)
        break;

      // beer time
      els_turn_dimension.spring_pass_count = 0;
      els_turn_dimension.op_state = ELS_TURN_DIM_OP_IDLE;
      els_turn_dimension.state = ELS_TURN_DIM_IDLE;
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.1: feed settings.
// ----------------------------------------------------------------------------------
static void els_turn_dimension_set_feed(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_turn_dimension.state = ELS_TURN_DIM_IDLE;
      els_turn_dimension_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_turn_dimension.state = ELS_TURN_DIM_SET_FFEED;
      els_turn_dimension_display_setting();
      break;
    case ELS_KEY_REV_FEED:
      els_turn_dimension_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_turn_dimension.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_turn_dimension.encoder_pos) * 10 * els_turn_dimension.encoder_multiplier;
        if (els_turn_dimension.feed_um + delta <= ELS_TURN_DIM_FEED_MIN)
          els_turn_dimension.feed_um = ELS_TURN_DIM_FEED_MIN;
        else if (els_turn_dimension.feed_um + delta >= ELS_TURN_DIM_FEED_MAX)
          els_turn_dimension.feed_um = ELS_TURN_DIM_FEED_MAX;
        else
          els_turn_dimension.feed_um += delta;
        els_turn_dimension.encoder_pos = encoder_curr;
        els_turn_dimension.feed_mm_s = els_turn_dimension.feed_um / 1000.0;
        els_turn_dimension_display_setting();
      }
      break;
  }
}

static void els_turn_dimension_set_finish_feed(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_turn_dimension.state = ELS_TURN_DIM_IDLE;
      els_turn_dimension_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_turn_dimension.state = ELS_TURN_DIM_SET_DOC;
      els_turn_dimension_display_setting();
      break;
    case ELS_KEY_REV_FEED:
      els_turn_dimension_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_turn_dimension.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_turn_dimension.encoder_pos) * 10 * els_turn_dimension.encoder_multiplier;
        if (els_turn_dimension.finish_feed_um + delta <= ELS_TURN_DIM_FEED_MIN)
          els_turn_dimension.finish_feed_um = ELS_TURN_DIM_FEED_MIN;
        else if (els_turn_dimension.finish_feed_um + delta >= ELS_TURN_DIM_FEED_MAX)
          els_turn_dimension.finish_feed_um = ELS_TURN_DIM_FEED_MAX;
        else
          els_turn_dimension.finish_feed_um += delta;
        els_turn_dimension.encoder_pos = encoder_curr;
        els_turn_dimension.finish_feed_mm_s = els_turn_dimension.finish_feed_um / 1000.0;
        els_turn_dimension_display_setting();
      }
      break;
  }
}
// ----------------------------------------------------------------------------------
// Function 2.2: depth of cut settings.
// ----------------------------------------------------------------------------------
static void els_turn_dimension_set_depth_of_cut(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_turn_dimension.state = ELS_TURN_DIM_IDLE;
      els_turn_dimension_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_turn_dimension.state = ELS_TURN_DIM_SET_FDOC;
      els_turn_dimension_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_turn_dimension.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_turn_dimension.encoder_pos) * 10 * els_turn_dimension.encoder_multiplier;
        if (els_turn_dimension.depth_of_cut_um + delta <= ELS_TURN_DIM_DOC_MIN)
          els_turn_dimension.depth_of_cut_um = ELS_TURN_DIM_DOC_MIN;
        else if (els_turn_dimension.depth_of_cut_um + delta >= ELS_TURN_DIM_DOC_MAX)
          els_turn_dimension.depth_of_cut_um = ELS_TURN_DIM_DOC_MAX;
        else
          els_turn_dimension.depth_of_cut_um += delta;
        els_turn_dimension.encoder_pos = encoder_curr;
        els_turn_dimension_display_setting();
      }
      break;
  }
}

static void els_turn_dimension_set_finish_depth(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_turn_dimension.state = ELS_TURN_DIM_IDLE;
      els_turn_dimension_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_turn_dimension.state = ELS_TURN_DIM_SET_FEED;
      els_turn_dimension_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_turn_dimension.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_turn_dimension.encoder_pos) * 10 * els_turn_dimension.encoder_multiplier;
        if (els_turn_dimension.finish_depth_um + delta <= ELS_TURN_DIM_DOC_MIN)
          els_turn_dimension.finish_depth_um = ELS_TURN_DIM_DOC_MIN;
        else if (els_turn_dimension.finish_depth_um + delta >= ELS_TURN_DIM_DOC_MAX)
          els_turn_dimension.finish_depth_um = ELS_TURN_DIM_DOC_MAX;
        else
          els_turn_dimension.finish_depth_um += delta;
        els_turn_dimension.encoder_pos = encoder_curr;
        els_turn_dimension_display_setting();
      }
      break;
  }
}

void els_turn_dimension_set_length(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_turn_dimension.state = ELS_TURN_DIM_IDLE;
      els_turn_dimension_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_turn_dimension.state = ELS_TURN_DIM_SET_DEPTH;
      els_turn_dimension_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_turn_dimension.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_turn_dimension.encoder_pos) * 0.01 * els_turn_dimension.encoder_multiplier;
        if (els_turn_dimension.length + delta <= 0)
          els_turn_dimension.length = 0;
        else if (els_turn_dimension.length + delta >= ELS_Z_MAX_MM)
          els_turn_dimension.length = ELS_Z_MAX_MM;
        else
          els_turn_dimension.length += delta;
        els_turn_dimension.encoder_pos = encoder_curr;
        els_turn_dimension_display_setting();
      }
      break;
  }
}

void els_turn_dimension_set_depth(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_turn_dimension.state = ELS_TURN_DIM_IDLE;
      els_turn_dimension_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_turn_dimension.state = ELS_TURN_DIM_SET_LEN;
      els_turn_dimension_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_turn_dimension.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_turn_dimension.encoder_pos) * 0.01 * els_turn_dimension.encoder_multiplier;
        if (els_turn_dimension.depth + delta <= 0)
          els_turn_dimension.depth = 0;
        else if (els_turn_dimension.depth + delta >= ELS_X_MAX_MM)
          els_turn_dimension.depth = ELS_X_MAX_MM;
        else
          els_turn_dimension.depth += delta;
        els_turn_dimension.encoder_pos = encoder_curr;
        els_turn_dimension_display_setting();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 3: Axis - position, origin & jogging
// ----------------------------------------------------------------------------------
static void els_turn_dimension_set_zaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_turn_dimension.state = (els_turn_dimension.state & ELS_TURN_DIM_ZJOG) ? ELS_TURN_DIM_SET_ZAXES : ELS_TURN_DIM_IDLE;
      els_turn_dimension_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_turn_dimension.state == ELS_TURN_DIM_SET_ZAXES) {
        els_stepper_zero_z();
        els_dro_zero_z();
        els_turn_dimension_display_axes();
      }
      break;
    case ELS_KEY_SET_ZX:
      els_turn_dimension.state = ELS_TURN_DIM_SET_XAXES;
      els_turn_dimension_display_axes();
      break;
    default:
      els_turn_dimension_zjog();
      break;
  }
}

static void els_turn_dimension_set_xaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_turn_dimension.state = ELS_TURN_DIM_IDLE;
      els_turn_dimension_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_turn_dimension.state == ELS_TURN_DIM_SET_XAXES) {
        els_stepper_zero_x();
        els_dro_zero_x();
        els_turn_dimension_display_axes();
      }
      break;
    case ELS_KEY_SET_ZX:
      els_turn_dimension.state = ELS_TURN_DIM_SET_ZAXES;
      els_turn_dimension_display_axes();
      break;
    default:
      els_turn_dimension_xjog();
      break;
  }
}

// ----------------------------------------------------------------------------------
// Manual Jog
// ----------------------------------------------------------------------------------
static void els_turn_dimension_zjog(void) {
  double  delta;
  int32_t  encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_turn_dimension.encoder_pos != encoder_curr) {
    delta = (encoder_curr - els_turn_dimension.encoder_pos) * (0.01 * els_turn_dimension.encoder_multiplier);
    els_turn_dimension.encoder_pos = encoder_curr;
    els_stepper_move_z(delta, els_config->z_jog_mm_s);
  }
}

static void els_turn_dimension_xjog(void) {
  double delta;
  int32_t encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_turn_dimension.encoder_pos != encoder_curr) {
    delta = (encoder_curr - els_turn_dimension.encoder_pos) * (0.01 * els_turn_dimension.encoder_multiplier);
    els_turn_dimension.encoder_pos = encoder_curr;
    els_stepper_move_x(delta, els_config->x_jog_mm_s);
  }
}
