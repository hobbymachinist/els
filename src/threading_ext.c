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
#include "tft/image.h"
#include "tft/fonts/fonts.h"

#include "constants.h"

#include "config.h"
#include "dro.h"
#include "encoder.h"
#include "spindle.h"
#include "stepper.h"
#include "keypad.h"
#include "threading_ext.h"
#include "utils.h"

#define ELS_THREADING_DOC_MIN   (10)
#define ELS_THREADING_DOC_MAX   (2000)
#define PRECISION               (1e-2)

//==============================================================================
// Externs
//==============================================================================
extern tft_device_t tft;

extern const tft_font_t noto_sans_mono_bold_arrows_24;
extern const tft_font_t noto_sans_mono_bold_26;
extern const tft_font_t gears_regular_32;
extern const tft_font_t gears_regular_50;
extern const tft_font_t inconsolata_lgc_bold_14;

extern const tft_image_t external_thread;
extern const tft_image_t external_thread_l;
extern const tft_image_t infeed_st;
extern const tft_image_t infeed_dia;
extern const tft_image_t infeed_dia_l;

//==============================================================================
// Config
//==============================================================================
typedef enum {
  ELS_THREADING_IDLE       = 1,
  ELS_THREADING_PAUSED     = 2,
  ELS_THREADING_ACTIVE     = 4,
  ELS_THREADING_SET_ZAXES  = 8,
  ELS_THREADING_ZJOG       = 16,
  ELS_THREADING_SET_XAXES  = 32,
  ELS_THREADING_XJOG       = 64,
  ELS_THREADING_SET_PITCH  = 128,
  ELS_THREADING_SET_DOC    = 256,
  ELS_THREADING_SET_LEN    = 512,
  ELS_THREADING_SET_DEPTH  = 1024
} els_threading_ext_state_t;

typedef enum {
  ELS_THREADING_OP_NA      = -1,
  ELS_THREADING_OP_IDLE    = 0,
  ELS_THREADING_OP_READY   = 1,
  ELS_THREADING_OP_MOVEZ0  = 2,
  ELS_THREADING_OP_MOVEX0  = 3,
  ELS_THREADING_OP_START   = 4,
  ELS_THREADING_OP_ATZ0    = 5,
  ELS_THREADING_OP_THREAD  = 6,
  ELS_THREADING_OP_THREADL = 7,
  ELS_THREADING_OP_ATZL    = 8,
  ELS_THREADING_OP_ATZLXM  = 9,
  ELS_THREADING_OP_ATZ0XM  = 10,
  ELS_THREADING_OP_FEED_IN = 11,
  ELS_THREADING_OP_SPRING  = 12,
  ELS_THREADING_OP_DONE    = 13
} els_threading_ext_op_state_t;

typedef enum {
  ELS_THREADING_PITCH_MODE_TABLE,
  ELS_THREADING_PITCH_MODE_ARB
} els_threading_pitch_mode_t;

static const char *op_labels[] = {
  "CONFIG ",
  "READY  ",
  "MOVE Z0",
  "MOVE X0",
  "ORIGIN ",
  "ALIGN  ",
  "THREAD ",
  "THREAD ",
  "LIMIT  ",
  "BACKOFF",
  "RETURN ",
  "FEED IN",
  "SPRING ",
  "DONE   "
};

// #######################################################################################
// IMPORTANT:
// ---------------------------------------------------------------------------------------
//
// The maximum pitch is a function of the pulses required per rotation of leadscrew and
// the pulses from the spindle encoder.
//
// PPR of spindle encoder should be greater than the pulses required to drive leadscrew
// for the pitch.
// #######################################################################################

// Metric Thread Pitches (Coarse)
#define ELS_THREADING_PITCH_TABLE_SIZE (sizeof(pitch_table_pitch_um) / sizeof(pitch_table_pitch_um[0]))
static const uint16_t pitch_table_pitch_um[] = {
  500,
  600,
  700,
  750,
  800,
  1000,
  1000,
  1250,
  1250,
  1500,
  1500,
  1750,
  2000,
  2000,
  2500,
  2500,
  2500,
  3000,
  3000
};

// Metric threads, Male Thread Height
static const double pitch_table_height_mm[] = {
  0.307,
  0.368,
  0.429,
  0.460,
  0.491,
  0.613,
  0.613,
  0.767,
  0.767,
  0.920,
  0.920,
  1.074,
  1.227,
  1.227,
  1.534,
  1.534,
  1.534,
  1.840,
  1.840
};

static const char *pitch_table_label[] = {
  "M3",
  "M3.5",
  "M4",
  "M4.5",
  "M5",
  "M6",
  "M7",
  "M8",
  "M9",
  "M10",
  "M11",
  "M12",
  "M14",
  "M16",
  "M18",
  "M20",
  "M22",
  "M24",
  "M27"
};

//==============================================================================
// Internal state
//==============================================================================
static struct {
  int32_t  pitch_um;
  bool     pitch_reverse;

  int16_t  pitch_table_index;
  els_threading_pitch_mode_t pitch_mode;

  int32_t  timer_feed_um;

  int32_t  depth_of_cut_um;

  bool     locked;

  double   length;
  double   depth;

  double   xdelta;
  double   zdelta;

  double   xpos_prev;

  bool     diagonal_infeed;
  double   zpos_start;

  uint8_t  spring_passes;
  uint8_t  spring_pass_count;

  // input read for jogging etc.
  int32_t  encoder_pos;

  // encoder multiplier
  int16_t encoder_multiplier;

  // module state
  els_threading_ext_state_t state;

  // spindle
  els_spindle_direction_t spindle_dir;

  // tracking variables for display refresh.
  els_threading_ext_state_t prev_state;
  els_spindle_direction_t prev_dir;

  // operation state
  volatile els_threading_ext_op_state_t op_state;

  // thread pitch ratio
  uint32_t pitch_n;
  uint32_t pitch_d;

  // current error or value.
  volatile uint32_t pitch_curr;

  // dro
  bool show_dro;
} els_threading_ext = {
  .depth_of_cut_um = 100,
  .pitch_table_index = 4,
  .length = 10,
  .spring_passes = 1,
  .spring_pass_count = 0,
  .encoder_multiplier = 1,
  .diagonal_infeed = true,
  .pitch_reverse = 0
};

//==============================================================================
// Internal functions
//==============================================================================
static void els_threading_ext_set_zdir(void);
static void els_threading_ext_run(void);
static void els_threading_ext_thread(void);

static void els_threading_ext_display_refresh(void);
static void els_threading_ext_display_setting(void);
static void els_threading_ext_display_axes(void);
static void els_threading_ext_display_header(void);
static void els_threading_ext_display_diagram(void);
static void els_threading_ext_display_encoder_pips(void);

static void els_threading_ext_set_pitch(void);
static void els_threading_ext_set_depth_of_cut(void);
static void els_threading_ext_set_length(void);
static void els_threading_ext_set_depth(void);

static void els_threading_ext_axes_setup(void);
static void els_threading_ext_set_xaxes(void);
static void els_threading_ext_set_zaxes(void);

static void els_threading_ext_recalculate_pitch_ratio(void);

static void els_threading_ext_configure_gpio(void);

static void els_threading_ext_encoder_isr(void) __attribute__ ((interrupt ("IRQ")));

static void els_threading_ext_keypad_process(void);

// manual jogging
static void els_threading_ext_zjog(void);
static void els_threading_ext_xjog(void);

static void els_threading_ext_zjog_sync(void);
static void els_threading_ext_xjog_sync(void);

//==============================================================================
// API
//==============================================================================
void els_threading_ext_setup(void) {
  // no-op
}

void els_threading_ext_start(void) {
  char text[32];

  els_threading_ext_configure_gpio();

  tft_filled_rectangle(&tft, 0,   0, 480, 320, TFT_COLOR_BLACK);

  tft_filled_rectangle(&tft, 0,  50, 480,   5, TFT_COLOR_GRAY);
  tft_filled_rectangle(&tft, 0,  50, 480,   1, TFT_COLOR_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 100, 480,   5, TFT_COLOR_GRAY);
  tft_filled_rectangle(&tft, 0, 100, 480,   1, TFT_COLOR_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 190, 480,   5, TFT_COLOR_GRAY);
  tft_filled_rectangle(&tft, 0, 190, 480,   1, TFT_COLOR_LITEGRAY);

  els_threading_ext_display_header();

  // angle symbol
  tft_font_write_bg(&tft, 165,  62, "J", &gears_regular_32, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  // pulse symbol
  tft_font_write_bg(&tft, 355,  58, "K", &gears_regular_32, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  // reset state
  els_threading_ext.prev_state = 0;
  els_threading_ext.prev_dir = 0;

  if (!els_threading_ext.show_dro)
    els_threading_ext_display_diagram();

  els_threading_ext.pitch_um = pitch_table_pitch_um[els_threading_ext.pitch_table_index];
  els_threading_ext.depth    = pitch_table_height_mm[els_threading_ext.pitch_table_index];

  els_threading_ext_recalculate_pitch_ratio();
  els_threading_ext_axes_setup();

  els_threading_ext.state = ELS_THREADING_IDLE;

  // gpio / exti isr
  els_nvic_irq_set_handler(ELS_S_ENCODER2_IRQ, els_threading_ext_encoder_isr);

  // spindle encoder
  nvic_set_priority(ELS_S_ENCODER2_IRQ, 6);
  nvic_enable_irq(ELS_S_ENCODER2_IRQ);

  els_threading_ext_display_setting();
  els_threading_ext_display_axes();
  els_threading_ext_display_refresh();

  els_stepper_start();
}

void els_threading_ext_stop(void) {
  els_stepper_stop();
  nvic_disable_irq(ELS_S_ENCODER2_IRQ);
}

bool els_threading_ext_busy(void) {
  return els_threading_ext.state != ELS_THREADING_IDLE;
}

void els_threading_ext_update(void) {
  static uint64_t last_refreshed_at = 0;

  uint64_t elapsed = els_timer_elapsed_microseconds();

  bool kp_locked = els_keypad_locked();
  if (els_threading_ext.locked != kp_locked) {
    els_threading_ext.locked = kp_locked;
    els_threading_ext_display_header();
  }

  if (els_threading_ext.state & (ELS_THREADING_IDLE | ELS_THREADING_PAUSED | ELS_THREADING_ACTIVE))
    els_threading_ext_keypad_process();

  if (els_threading_ext.state &
     (ELS_THREADING_PAUSED | ELS_THREADING_ACTIVE | ELS_THREADING_SET_XAXES | ELS_THREADING_SET_ZAXES))
    els_stepper_enable();
  else
    els_stepper_disable();

  switch (els_threading_ext.state) {
    case ELS_THREADING_PAUSED:
    case ELS_THREADING_ACTIVE:
      els_threading_ext_run();
      break;
    case ELS_THREADING_SET_PITCH:
      els_threading_ext_set_pitch();
      break;
    case ELS_THREADING_SET_DOC:
      els_threading_ext_set_depth_of_cut();
      break;
    case ELS_THREADING_SET_LEN:
      els_threading_ext_set_length();
      break;
    case ELS_THREADING_SET_DEPTH:
      els_threading_ext_set_depth();
      break;
    default:
      if (els_threading_ext.state & (ELS_THREADING_SET_ZAXES | ELS_THREADING_ZJOG))
        els_threading_ext_set_zaxes();
      else if (els_threading_ext.state & (ELS_THREADING_SET_XAXES | ELS_THREADING_XJOG))
        els_threading_ext_set_xaxes();
      break;
  }

  if (elapsed - last_refreshed_at > 1e5) {
    last_refreshed_at = elapsed;
    els_threading_ext_display_refresh();
  }

  int16_t em = els_encoder_get_multiplier();
  if (em != els_threading_ext.encoder_multiplier) {
    els_threading_ext.encoder_multiplier = em;
    els_threading_ext_display_encoder_pips();
  }
}

//==============================================================================
// ISR
//==============================================================================
static void els_threading_ext_encoder_isr(void) {
  static volatile uint32_t x1, x2;
  static volatile bool reset_pulse_pending = false;

  x1 = els_gpio_get(ELS_S_ENCODER2_PORTA, ELS_S_ENCODER2_PINA);
  x2 = els_gpio_get(ELS_S_ENCODER2_PORTB, ELS_S_ENCODER2_PINB);

  if (exti_get_flag_status(ELS_S_ENCODER2_EXTI)) {
    exti_reset_request(ELS_S_ENCODER2_EXTI);
    if (x1) {
      if (x2)
        els_threading_ext.spindle_dir = ELS_S_DIRECTION_CW;
      else
        els_threading_ext.spindle_dir = ELS_S_DIRECTION_CCW;
    }
    else {
      if (x2)
        els_threading_ext.spindle_dir = ELS_S_DIRECTION_CCW;
      else
        els_threading_ext.spindle_dir = ELS_S_DIRECTION_CW;
    }

    if (els_threading_ext.state == ELS_THREADING_ACTIVE && els_threading_ext.op_state == ELS_THREADING_OP_THREAD) {
      if (reset_pulse_pending) {
        reset_pulse_pending = false;
        els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
        els_stepper->zpos += (els_stepper->zdir * els_threading_ext.zdelta);

        if (els_config->z_closed_loop) {
          double dro_zpos = (double)els_dro.zpos_um * 1e-3;
          if ((els_stepper->zdir * (dro_zpos - els_threading_ext.zpos_start)) >= els_threading_ext.length) {
            els_threading_ext.op_state = ELS_THREADING_OP_THREADL;
            els_stepper->zpos = dro_zpos;
          }
        }
        else {
          if ((els_stepper->zdir * (els_stepper->zpos - els_threading_ext.zpos_start)) >= els_threading_ext.length)
            els_threading_ext.op_state = ELS_THREADING_OP_THREADL;
        }
      }

      els_threading_ext.pitch_curr += els_threading_ext.pitch_n;
      if (els_threading_ext.pitch_curr >= els_threading_ext.pitch_d) {
        els_threading_ext.pitch_curr -= els_threading_ext.pitch_d;
        els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
        reset_pulse_pending = true;
      }
    }
  }
}

//==============================================================================
// Internal functions
//==============================================================================

// ----------------------------------------------------------------------------------
// Display functions
// ----------------------------------------------------------------------------------
static void els_threading_ext_display_setting(void) {
  char text[32];

  els_sprint_double3(text, sizeof(text), els_threading_ext.length, "L");
  if (els_threading_ext.state == ELS_THREADING_SET_LEN)
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
  else
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  els_sprint_double1(text, sizeof(text), els_threading_ext.depth_of_cut_um / 1000.0, "Xs ");
  if (els_threading_ext.state == ELS_THREADING_SET_DOC)
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
  else
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  els_sprint_double23(text, sizeof(text), els_threading_ext.pitch_um / 1000.0, "P");
  if (els_threading_ext.state == ELS_THREADING_SET_PITCH && els_threading_ext.pitch_mode == ELS_THREADING_PITCH_MODE_ARB)
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_26, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
  else
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  els_sprint_double23(text, sizeof(text), els_threading_ext.depth, "H");
  if (els_threading_ext.state == ELS_THREADING_SET_DEPTH)
    tft_font_write_bg(&tft, 310, 262, text, &noto_sans_mono_bold_26, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
  else
    tft_font_write_bg(&tft, 310, 262, text, &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  uint16_t x = (els_threading_ext.show_dro ? 220 : 8), y = 262;
  tft_filled_rectangle(&tft, x, y, 80, 40, TFT_COLOR_BLACK);

  if (els_threading_ext.pitch_mode == ELS_THREADING_PITCH_MODE_TABLE) {
    tft_font_write_bg(&tft, x, y, pitch_table_label[els_threading_ext.pitch_table_index],
      &noto_sans_mono_bold_26,
      (els_threading_ext.state == ELS_THREADING_SET_PITCH ? TFT_COLOR_YELLOW : TFT_COLOR_WHITE),
      TFT_COLOR_BLACK);
  }

  x = (els_threading_ext.show_dro ? 220 : 8), y = 230;
  tft_filled_rectangle(&tft, x, y, 80, 40, TFT_COLOR_BLACK);
  if (els_threading_ext.diagonal_infeed)
    tft_image_draw(&tft, x, y, (els_threading_ext.pitch_reverse ? &infeed_dia_l : &infeed_dia), TFT_COLOR_YELLOW);
  else
    tft_image_draw(&tft, x, y, &infeed_st, TFT_COLOR_YELLOW);
}

static void els_threading_ext_display_axes(void) {
  char text[32];

  els_sprint_double33(text, sizeof(text), els_stepper->zpos, "Z");

  if (els_threading_ext.state & ELS_THREADING_SET_ZAXES)
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
  else
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  els_sprint_double33(text, sizeof(text), els_stepper->xpos, "X");
  if (els_threading_ext.state & ELS_THREADING_SET_XAXES)
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
  else
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  if (els_threading_ext.show_dro) {
    tft_font_write_bg(&tft, 8, 190, "DRO", &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.zpos_um / 1000.0), "Z");
    tft_font_write_bg(&tft, 8, 228, text, &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.xpos_um / 1000.0), "X");
    tft_font_write_bg(&tft, 8, 262, text, &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);
  }
}

static void els_threading_ext_display_diagram(void) {
  // ----------------------------------------------------------------------------------------------
  // diagram
  // ----------------------------------------------------------------------------------------------
  if (els_threading_ext.pitch_reverse)
    tft_image_draw(&tft, 100, 235, &external_thread_l, TFT_COLOR_WHITE);
  else
    tft_image_draw(&tft, 100, 235, &external_thread, TFT_COLOR_WHITE);

  tft_font_write_bg(&tft, 166, 183, "A", &noto_sans_mono_bold_arrows_24, TFT_COLOR_WHITE, TFT_COLOR_BLACK);
  tft_font_write_bg(&tft, 210, 199, "L", &inconsolata_lgc_bold_14, TFT_COLOR_WHITE, TFT_COLOR_BLACK);
  tft_font_write_bg(&tft, 240, 183, "C", &noto_sans_mono_bold_arrows_24, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  tft_font_write_bg(&tft, 260, 200, "D", &noto_sans_mono_bold_arrows_24, TFT_COLOR_WHITE, TFT_COLOR_BLACK);
  tft_font_write_bg(&tft, 280, 227, "H", &inconsolata_lgc_bold_14, TFT_COLOR_WHITE, TFT_COLOR_BLACK);
  tft_font_write_bg(&tft, 260, 227, "B", &noto_sans_mono_bold_arrows_24, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  // origin
  if (els_threading_ext.pitch_reverse)
    tft_font_write_bg(&tft, 165, 264, ".", &noto_sans_mono_bold_26, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
  else
    tft_font_write_bg(&tft, 245, 264, ".", &noto_sans_mono_bold_26, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);

  // legend
  tft_font_write_bg(&tft, 8, 180, ".", &noto_sans_mono_bold_26, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
  tft_font_write_bg(&tft, 26, 200, "(0,0)", &inconsolata_lgc_bold_14, TFT_COLOR_WHITE, TFT_COLOR_BLACK);
  // ----------------------------------------------------------------------------------------------
}
static void els_threading_ext_display_header(void) {
  if (els_threading_ext.pitch_reverse) {
    tft_rgb_t color = (els_threading_ext.locked ? TFT_COLOR_RED : TFT_COLOR_GREEN);
    tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
    tft_font_write_bg(&tft, 8, 0, "THREAD EXTERNAL - L", &noto_sans_mono_bold_26, TFT_COLOR_BLACK, color);
  }
  else {
    tft_rgb_t color = (els_threading_ext.locked ? TFT_COLOR_RED : TFT_COLOR_FOREST);
    tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
    tft_font_write_bg(&tft, 8, 0, "THREAD EXTERNAL - R", &noto_sans_mono_bold_26, TFT_COLOR_WHITE, color);
  }

  els_threading_ext_display_encoder_pips();
}

static void els_threading_ext_display_encoder_pips(void) {
  // multiplier is 100, 10 or 1
  size_t pips = els_threading_ext.encoder_multiplier;
  pips = pips > 10 ? 3 : pips > 1 ? 2 : 1;

  tft_rgb_t color = (els_threading_ext.pitch_reverse ? TFT_COLOR_BLACK : TFT_COLOR_WHITE);

  for (size_t n = 0, spacing = 0; n < 3; n++, spacing++)
    tft_filled_rectangle(&tft,
      440, 32 - (n * 10 + spacing * 2),
      15 + n * 10, 10,
      (n < pips ? color : els_threading_ext.locked ? TFT_COLOR_LITEGRAY : TFT_COLOR_BGCOLOR2));
}

static void els_threading_ext_display_refresh(void) {
  els_spindle_direction_t dir = els_spindle_get_direction();
  if (dir != els_threading_ext.prev_dir) {
    els_threading_ext.prev_dir = dir;
    tft_filled_rectangle(&tft, 0, 55, 60, 45, TFT_COLOR_BLACK);
    switch (dir) {
      case ELS_S_DIRECTION_CW:
        // cw symbol
        tft_font_write_bg(&tft, 5, 62, "A", &gears_regular_32, TFT_COLOR_WHITE, TFT_COLOR_BLACK);
        break;
      case ELS_S_DIRECTION_CCW:
        // ccw symbol
        tft_font_write_bg(&tft, 5, 62, "B", &gears_regular_32, TFT_COLOR_WHITE, TFT_COLOR_BLACK);
        break;
      default:
        // spindle symbol
        tft_font_write_bg(&tft, 3, 52, "N", &gears_regular_50, TFT_COLOR_WHITE, TFT_COLOR_BLACK);
        break;
    }
  }

  char text[32];
  snprintf(text, sizeof(text), "%04d", els_spindle_get_rpm());
  tft_font_write_bg(&tft, 62, 52, text, &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  double angle = els_spindle_get_angle();
  snprintf(text, sizeof(text), "%03d.%02d", (int)angle, (int)((angle - (int)angle) * 100));
  tft_font_write_bg(&tft, 200, 52, text, &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  snprintf(text, sizeof(text), "%04d", els_spindle_get_counter());
  tft_font_write_bg(&tft, 396, 52, text, &noto_sans_mono_bold_26, TFT_COLOR_WHITE, TFT_COLOR_BLACK);

  els_threading_ext_display_axes();

  static els_threading_ext_op_state_t prev_op_state = ELS_THREADING_OP_NA;
  if (els_threading_ext.op_state != prev_op_state) {
    prev_op_state = els_threading_ext.op_state;
    tft_filled_rectangle(&tft, 310, 195, 169, 30, TFT_COLOR_BLACK);
    if (els_threading_ext.spring_pass_count > 0) {
      tft_font_write_bg(&tft, 310, 190, "SPRING", &noto_sans_mono_bold_26, TFT_COLOR_YELLOW, TFT_COLOR_BLACK);
    }
    else {
      const char *label = op_labels[els_threading_ext.op_state];
      tft_font_write_bg(&tft, 310, 190, label, &noto_sans_mono_bold_26, TFT_COLOR_FOREST, TFT_COLOR_BLACK);
    }
  }
}

// ----------------------------------------------------------------------------------
// Top module level keypad handler.
// ----------------------------------------------------------------------------------
static void els_threading_ext_keypad_process(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
      if (els_threading_ext.state == ELS_THREADING_IDLE) {
        if (els_threading_ext.length > 0 && els_threading_ext.depth > 0) {
          els_threading_ext.state = ELS_THREADING_PAUSED;
          els_threading_ext.op_state = ELS_THREADING_OP_READY;
        }
        else {
          els_threading_ext.state = ELS_THREADING_SET_LEN;
          els_threading_ext_display_setting();
        }
      }
      break;
    case ELS_KEY_EXIT:
      if (els_threading_ext.state & (ELS_THREADING_PAUSED | ELS_THREADING_ACTIVE)) {
        els_threading_ext.state = ELS_THREADING_IDLE;
        els_threading_ext.op_state = ELS_THREADING_OP_IDLE;
      }
      break;
    case ELS_KEY_SET_FEED:
      if (els_threading_ext.state & (ELS_THREADING_IDLE | ELS_THREADING_PAUSED)) {
        els_threading_ext.state = ELS_THREADING_SET_PITCH;
        els_threading_ext.encoder_pos = 0;
        els_threading_ext_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F1:
      if (els_threading_ext.state & (ELS_THREADING_IDLE | ELS_THREADING_PAUSED)) {
        els_threading_ext.state = ELS_THREADING_SET_LEN;
        els_threading_ext.encoder_pos = 0;
        els_threading_ext_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F2:
      els_threading_ext.show_dro = !els_threading_ext.show_dro;
      tft_filled_rectangle(&tft, 0, 200, 300, 120, TFT_COLOR_BLACK);
      if (els_threading_ext.show_dro)
        els_threading_ext_display_axes();
      else
        els_threading_ext_display_diagram();
      els_threading_ext_display_setting();
      break;
    case ELS_KEY_SET_ZX:
      if (els_threading_ext.state & (ELS_THREADING_IDLE | ELS_THREADING_PAUSED)) {
        els_threading_ext.state = ELS_THREADING_SET_ZAXES;
        els_threading_ext.encoder_pos = 0;
        els_threading_ext_display_axes();
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
//
// sets zdir depending on type of thread wanted & compensates for backlash.
static void els_threading_ext_set_zdir(void) {
  int zdir = (els_threading_ext.pitch_reverse ? 1 : -1);

  if (zdir == 1) {
    els_stepper_set_zdir_incr();
    if (els_stepper->zdir != zdir) {
      els_stepper->zdir = zdir;
      els_stepper_z_backlash_fix();
    }
  }
  else {
    els_stepper_set_zdir_decr();
    if (els_stepper->zdir != zdir) {
      els_stepper->zdir = zdir;
      els_stepper_z_backlash_fix();
    }
  }
  els_stepper->zdir = zdir;
}

static void els_threading_ext_run(void) {
  if (els_threading_ext.state == ELS_THREADING_PAUSED && !els_stepper->xbusy && !els_stepper->zbusy) {
    els_threading_ext_set_zdir();
    if (els_spindle_get_counter() == 0)
      els_threading_ext.state = ELS_THREADING_ACTIVE;
  }

  if (els_threading_ext.state == ELS_THREADING_ACTIVE)
    els_threading_ext_thread();
}

// main control loop
static void els_threading_ext_thread(void) {
  switch (els_threading_ext.op_state) {
    case ELS_THREADING_OP_NA:
      break;
    case ELS_THREADING_OP_IDLE:
      break;
    case ELS_THREADING_OP_READY:
      els_threading_ext.op_state = ELS_THREADING_OP_MOVEZ0;
      els_stepper_sync();
      break;
    case ELS_THREADING_OP_MOVEZ0:
      if (fabs(els_stepper->zpos) > PRECISION)
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);
      els_threading_ext.op_state = ELS_THREADING_OP_MOVEX0;
      els_threading_ext.zpos_start = 0;
      break;
    case ELS_THREADING_OP_MOVEX0:
      if (els_stepper->zbusy)
        break;
      if (fabs(els_stepper->xpos) > PRECISION)
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_jog_mm_s);
      els_threading_ext.op_state = ELS_THREADING_OP_START;
      break;
    case ELS_THREADING_OP_START:
      if (els_stepper->xbusy)
        break;
      els_threading_ext.op_state = ELS_THREADING_OP_ATZ0;
      break;
    case ELS_THREADING_OP_ATZ0:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        els_threading_ext_set_zdir();
        if (els_spindle_get_counter() == 0)
          els_threading_ext.op_state = ELS_THREADING_OP_THREAD;
      }
      break;
    case ELS_THREADING_OP_THREAD:
      // handled by ISR
      break;
    case ELS_THREADING_OP_THREADL:
      els_threading_ext.xpos_prev = els_stepper->xpos;
      els_stepper_move_x(0.5 - els_stepper->xpos, els_config->x_jog_mm_s);
      els_threading_ext.op_state = ELS_THREADING_OP_ATZL;
      break;
    case ELS_THREADING_OP_ATZL:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        els_stepper_move_z(els_threading_ext.zpos_start - els_stepper->zpos, els_config->z_jog_mm_s);
        els_threading_ext.op_state = ELS_THREADING_OP_ATZLXM;
      }
      break;
    case ELS_THREADING_OP_ATZLXM:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        els_stepper_move_x(els_threading_ext.xpos_prev - els_stepper->xpos, els_config->x_jog_mm_s);
        els_threading_ext.op_state = ELS_THREADING_OP_ATZ0XM;
      }
      break;
    case ELS_THREADING_OP_ATZ0XM:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        if (els_threading_ext.spring_pass_count > 0)
          els_threading_ext.op_state = ELS_THREADING_OP_SPRING;
        else if ((els_threading_ext.depth + els_stepper->xpos) > PRECISION) {
          double xd, zd;

          xd = MIN(
            els_threading_ext.depth + els_stepper->xpos,
            els_threading_ext.depth_of_cut_um / 1000.0
          );
          els_stepper_move_x(-xd, els_config->x_jog_mm_s);

          // -------------------------------------------------------------------
          // EXPERIMENTAL - only works for 60° standard threads
          // -------------------------------------------------------------------
          // 15° - 29.5° in-feed for reduced tool stress.
          //
          // Use ~26.565° in-feed angle
          //
          // 1. 0.5543 multiplier for 29.0°
          // 2. 0.5658 multiplier for 29.5°
          //
          // -------------------------------------------------------------------
          if (els_threading_ext.diagonal_infeed) {
            zd = (xd * 0.5) * els_stepper->zdir;
            els_threading_ext.zpos_start += zd;
            els_stepper_move_z(zd, els_config->z_jog_mm_s);
          }
          els_threading_ext.op_state = ELS_THREADING_OP_FEED_IN;
        }
        else {
          els_threading_ext.op_state = ELS_THREADING_OP_SPRING;
        }
      }
      break;
    case ELS_THREADING_OP_FEED_IN:
      if (!els_stepper->xbusy && !els_stepper->zbusy)
        els_threading_ext.op_state = ELS_THREADING_OP_ATZ0;
      break;
    case ELS_THREADING_OP_SPRING:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        if (els_threading_ext.spring_pass_count >= els_threading_ext.spring_passes) {
          els_threading_ext.op_state = ELS_THREADING_OP_DONE;
          els_threading_ext.spring_pass_count = 0;
          els_stepper_move_x(0 - els_stepper->xpos, els_config->x_jog_mm_s);
          els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);
        }
        else {
          els_threading_ext.op_state = ELS_THREADING_OP_ATZ0;
          els_threading_ext.spring_pass_count++;
        }
      }
      break;
    case ELS_THREADING_OP_DONE:
      // beer time
      if (els_stepper->xbusy || els_stepper->zbusy)
        break;

      els_threading_ext.op_state = ELS_THREADING_OP_IDLE;
      els_threading_ext.state = ELS_THREADING_IDLE;
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.1: pitch settings.
// ----------------------------------------------------------------------------------
static void els_threading_ext_recalculate_pitch_ratio(void) {
  uint32_t n = (els_threading_ext.pitch_um * els_config->z_pulses_per_mm) / 1000;
  uint32_t d = els_config->spindle_encoder_ppr;

  uint32_t g = els_gcd(n, d);

  els_threading_ext.pitch_n = n / g;
  els_threading_ext.pitch_d = d / g;
  els_threading_ext.pitch_curr = 0;

  els_printf("pitch n = %lu d = %lu\n", els_threading_ext.pitch_n, els_threading_ext.pitch_d);
}

static void els_threading_ext_set_pitch(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_threading_ext.state = ELS_THREADING_IDLE;
      els_threading_ext_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_threading_ext.pitch_mode = !els_threading_ext.pitch_mode;
      els_threading_ext_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_threading_ext.state = ELS_THREADING_SET_DEPTH;
      els_threading_ext_display_setting();
      break;
    case ELS_KEY_REV_FEED:
      els_threading_ext.pitch_reverse = !els_threading_ext.pitch_reverse;
      els_threading_ext_display_header();
      els_threading_ext_display_encoder_pips();
      tft_filled_rectangle(&tft, 0, 200, 300, 120, TFT_COLOR_BLACK);
      if (els_threading_ext.show_dro)
        els_threading_ext_display_axes();
      else
        els_threading_ext_display_diagram();
      els_threading_ext_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_threading_ext.encoder_pos != encoder_curr) {
        if (els_threading_ext.pitch_mode == ELS_THREADING_PITCH_MODE_TABLE) {
          if (encoder_curr > els_threading_ext.encoder_pos)
            els_threading_ext.pitch_table_index++;
          else
            els_threading_ext.pitch_table_index--;

          if (els_threading_ext.pitch_table_index < 0)
            els_threading_ext.pitch_table_index = ELS_THREADING_PITCH_TABLE_SIZE - 1;
          else if (els_threading_ext.pitch_table_index >= (int16_t)ELS_THREADING_PITCH_TABLE_SIZE)
            els_threading_ext.pitch_table_index = 0;

          els_threading_ext.pitch_um = pitch_table_pitch_um[els_threading_ext.pitch_table_index];
          els_threading_ext.depth    = pitch_table_height_mm[els_threading_ext.pitch_table_index];
        }
        else {
          els_threading_ext.pitch_um += (encoder_curr - els_threading_ext.encoder_pos);
        }

        els_threading_ext_recalculate_pitch_ratio();
        els_threading_ext.encoder_pos = encoder_curr;
        els_threading_ext_display_setting();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.2: depth of cut settings.
// ----------------------------------------------------------------------------------
static void els_threading_ext_set_depth_of_cut(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_threading_ext.state = ELS_THREADING_IDLE;
      els_threading_ext_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_threading_ext.state = ELS_THREADING_SET_LEN;
      els_threading_ext_display_setting();
      break;
    case ELS_KEY_REV_FEED:
      els_threading_ext.diagonal_infeed = !els_threading_ext.diagonal_infeed;
      els_threading_ext_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_threading_ext.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_threading_ext.encoder_pos) * 10 * els_threading_ext.encoder_multiplier;
        if (els_threading_ext.depth_of_cut_um + delta <= ELS_THREADING_DOC_MIN)
          els_threading_ext.depth_of_cut_um = ELS_THREADING_DOC_MIN;
        else if (els_threading_ext.depth_of_cut_um + delta >= ELS_THREADING_DOC_MAX)
          els_threading_ext.depth_of_cut_um = ELS_THREADING_DOC_MAX;
        else
          els_threading_ext.depth_of_cut_um += delta;
        els_threading_ext.encoder_pos = encoder_curr;
        els_threading_ext_display_setting();
      }
      break;
  }
}

void els_threading_ext_set_length(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_threading_ext.state = ELS_THREADING_IDLE;
      els_threading_ext_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_threading_ext.state = ELS_THREADING_SET_DOC;
      els_threading_ext_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_threading_ext.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_threading_ext.encoder_pos) * 0.01 * els_threading_ext.encoder_multiplier;
        if (els_threading_ext.length + delta <= 0)
          els_threading_ext.length = 0;
        else if (els_threading_ext.length + delta >= ELS_Z_MAX_MM)
          els_threading_ext.length = ELS_Z_MAX_MM;
        else
          els_threading_ext.length += delta;
        els_threading_ext.encoder_pos = encoder_curr;
        els_threading_ext_display_setting();
      }
      break;
  }
}

void els_threading_ext_set_depth(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_threading_ext.state = ELS_THREADING_IDLE;
      els_threading_ext_display_setting();
      break;
    case ELS_KEY_SET_FEED:
      els_threading_ext.state = ELS_THREADING_SET_PITCH;
      els_threading_ext_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_threading_ext.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_threading_ext.encoder_pos) * 0.001 * els_threading_ext.encoder_multiplier;
        if (els_threading_ext.depth + delta <= 0)
          els_threading_ext.depth = 0;
        else if (els_threading_ext.depth + delta >= 5.00)
          els_threading_ext.depth = 5.00;
        else
          els_threading_ext.depth += delta;
        els_threading_ext.encoder_pos = encoder_curr;
        els_threading_ext_display_setting();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 3: Axis - position, origin & jogging
// ----------------------------------------------------------------------------------
static void els_threading_ext_set_zaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_threading_ext.state = (els_threading_ext.state & ELS_THREADING_ZJOG) ? ELS_THREADING_SET_ZAXES : ELS_THREADING_IDLE;
      els_threading_ext_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_threading_ext.state == ELS_THREADING_SET_ZAXES) {
        els_stepper->zpos = 0;
        els_dro_zero_z();
        els_threading_ext_display_axes();
      }
      break;
    case ELS_KEY_JOG_ZX_ORI:
      if (!els_stepper->zbusy)
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);
      break;
    case ELS_KEY_SET_ZX:
      els_threading_ext.state = ELS_THREADING_SET_XAXES;
      els_threading_ext_display_axes();
      break;
    default:
      els_threading_ext_zjog();
      break;
  }
}

static void els_threading_ext_set_xaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_threading_ext.state = ELS_THREADING_IDLE;
      els_threading_ext_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_threading_ext.state == ELS_THREADING_SET_XAXES) {
        els_stepper->xpos = 0;
        els_dro_zero_x();
        els_threading_ext_display_axes();
      }
      break;
    case ELS_KEY_JOG_ZX_ORI:
      if (!els_stepper->xbusy)
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_jog_mm_s);
      break;
    case ELS_KEY_SET_ZX:
      els_threading_ext.state = ELS_THREADING_SET_ZAXES;
      els_threading_ext_display_axes();
      break;
    default:
      els_threading_ext_xjog();
      break;
  }
}

// ----------------------------------------------------------------------------------
// Manual Jog
// ----------------------------------------------------------------------------------

static void els_threading_ext_zjog(void) {
  double delta, step;
  int32_t encoder_curr;

  els_threading_ext_zjog_sync();
  encoder_curr = els_encoder_read();
  if (els_threading_ext.encoder_pos != encoder_curr) {
    step = els_threading_ext.encoder_multiplier == 1 ? 0.005 : 0.01 * els_threading_ext.encoder_multiplier;
    delta = (encoder_curr - els_threading_ext.encoder_pos) * step;
    els_threading_ext.state |= ELS_THREADING_ZJOG;
    els_stepper_move_z(delta, els_config->z_jog_mm_s);
    els_threading_ext.encoder_pos = encoder_curr;
    els_threading_ext_display_axes();
  }
}

static void els_threading_ext_zjog_sync(void) {
  if ((els_threading_ext.state & ELS_THREADING_ZJOG) && !els_stepper->zbusy) {
    els_threading_ext.state &= ~ELS_THREADING_ZJOG;
  }
}

static void els_threading_ext_xjog(void) {
  double delta, step;
  int32_t encoder_curr;

  els_threading_ext_xjog_sync();
  encoder_curr = els_encoder_read();
  if (els_threading_ext.encoder_pos != encoder_curr) {
    step = els_threading_ext.encoder_multiplier == 1 ? 0.005 : 0.01 * els_threading_ext.encoder_multiplier;
    delta = (encoder_curr - els_threading_ext.encoder_pos) * step;
    els_threading_ext.state |= ELS_THREADING_XJOG;
    els_stepper_move_x(delta, els_config->x_jog_mm_s);
    els_threading_ext.encoder_pos = encoder_curr;
    els_threading_ext_display_axes();
  }
}

static void els_threading_ext_xjog_sync(void) {
  if ((els_threading_ext.state & ELS_THREADING_XJOG) && !els_stepper->xbusy) {
    els_threading_ext.state &= ~ELS_THREADING_XJOG;
  }
}

static void els_threading_ext_axes_setup(void) {
  // Z-axis movement per pulse in mm.
  els_threading_ext.zdelta  = (1.0 / (double)els_config->z_pulses_per_mm);

  // X-axis movement per pulse in mm.
  els_threading_ext.xdelta  = (1.0 / (double)els_config->x_pulses_per_mm);
}

// ----------------------------------------------------------------------------------
// GPIO: setup pins.
// ----------------------------------------------------------------------------------
static void els_threading_ext_configure_gpio(void) {
  gpio_mode_setup(ELS_S_ENCODER2_PORTA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, ELS_S_ENCODER2_PINA);
  gpio_mode_setup(ELS_S_ENCODER2_PORTB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, ELS_S_ENCODER2_PINB);

  exti_select_source(ELS_S_ENCODER2_EXTI, ELS_S_ENCODER2_PORTA);
  exti_set_trigger(ELS_S_ENCODER2_EXTI, EXTI_TRIGGER_RISING);
  exti_enable_request(ELS_S_ENCODER2_EXTI);
}
