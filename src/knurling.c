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
#include "knurling.h"
#include "utils.h"

//==============================================================================
// Externs
//==============================================================================
extern tft_device_t tft;

extern const tft_font_t noto_sans_mono_bold_arrows_24;
extern const tft_font_t noto_sans_mono_bold_26;
extern const tft_font_t noto_sans_mono_bold_20;
extern const tft_font_t gears_regular_32;
extern const tft_font_t gears_regular_50;
extern const tft_font_t inconsolata_lgc_bold_14;

//==============================================================================
// Config
//==============================================================================
typedef enum {
  ELS_KNURLING_IDLE          = 1,
  ELS_KNURLING_PAUSED        = 2,
  ELS_KNURLING_ACTIVE        = 4,
  ELS_KNURLING_SET_ZAXES     = 8,
  ELS_KNURLING_ZJOG          = 16,
  ELS_KNURLING_SET_XAXES     = 32,
  ELS_KNURLING_XJOG          = 64,
  ELS_KNURLING_SET_DEPTH     = 128,
  ELS_KNURLING_SET_LENGTH    = 256,
  ELS_KNURLING_SET_DIAMETER  = 512,
  ELS_KNURLING_SET_ANGLE     = 1024,
  ELS_KNURLING_SET_DIVISIONS = 2048
} els_knurling_state_t;

typedef enum {
  ELS_KNURLING_OP_NA      = -1,
  ELS_KNURLING_OP_IDLE    = 0,
  ELS_KNURLING_OP_READY   = 1,
  ELS_KNURLING_OP_MOVEZ0  = 2,
  ELS_KNURLING_OP_MOVEZL  = 3,
  ELS_KNURLING_OP_MOVEX0  = 4,
  ELS_KNURLING_OP_START   = 5,
  ELS_KNURLING_OP_ATZ0    = 6,
  ELS_KNURLING_OP_KNURL   = 7,
  ELS_KNURLING_OP_KNURLL  = 8,
  ELS_KNURLING_OP_ATZL    = 9,
  ELS_KNURLING_OP_ATZLXM  = 10,
  ELS_KNURLING_OP_ATZ0XM  = 11,
  ELS_KNURLING_OP_FEED_IN = 12,
  ELS_KNURLING_OP_DONE    = 13
} els_knurling_op_state_t;

typedef enum {
  ELS_KNURLING_OP_STAGE1 = 0,
  ELS_KNURLING_OP_STAGE2 = 1,
} els_knurling_op_stage_t;

static const char *op_labels[] = {
  "CONFIG ",
  "READY  ",
  "MOVE Z0",
  "MOVE ZL",
  "MOVE X0",
  "ORIGIN ",
  "ALIGN  ",
  "KNURL  ",
  "KNURL  ",
  "LIMIT  ",
  "BACKOFF",
  "RETURN ",
  "FEED IN",
  "DONE   "
};

#define ELS_KNURLING_TIMER                TIM5
#define ELS_KNURLING_TIMER_IRQ            NVIC_TIM5_IRQ
#define ELS_KNURLING_TIMER_MAX            UINT32_MAX
#define ELS_KNURLING_TIMER_RCC            RCC_TIM5
#define ELS_KNURLING_TIMER_RST            RST_TIM5

#define ELS_KNURLING_SET_ZDIR_LR          els_gpio_set(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)
#define ELS_KNURLING_SET_ZDIR_RL          els_gpio_clear(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)

#define ELS_KNURLING_SET_XDIR_BT          els_gpio_set(ELS_X_DIR_PORT, ELS_X_DIR_PIN)
#define ELS_KNURLING_SET_XDIR_TB          els_gpio_clear(ELS_X_DIR_PORT, ELS_X_DIR_PIN)

#define PRECISION                         (1e-2)
//==============================================================================
// Internal state
//==============================================================================
#define ELS_KNURLING_DEPTH_MIN            (0.05)
#define ELS_KNURLING_DEPTH_MAX            (0.5)

static struct {
  int32_t  timer_feed_um;

  bool     locked;

  double   depth;
  int32_t  depth_of_cut_um;

  double   length;
  double   diameter;
  double   angle;
  uint16_t divisions;

  double   xdelta;
  double   zdelta;

  double   xpos_prev;

  // input read for jogging etc.
  int32_t  encoder_pos;

  // encoder multiplier
  int16_t encoder_multiplier;

  // module state
  els_knurling_state_t state;

  // spindle
  els_spindle_direction_t spindle_dir;

  // tracking variables for display refresh.
  els_knurling_state_t prev_state;
  els_spindle_direction_t prev_dir;

  // operation state
  volatile els_knurling_op_state_t op_state;

  // calculated pitch
  double   pitch;

  // pitch pulse ratios
  uint32_t pitch_p;
  uint32_t pitch_n;
  uint32_t pitch_d;

  // current stage
  els_knurling_op_stage_t op_stage;

  // current phase / offset.
  uint32_t phase_offset;
  uint32_t phase_delta;

  // current error or value.
  volatile uint32_t pitch_curr;

  volatile uint32_t ztarget;
  volatile uint32_t zcurrent;

  // dro
  bool show_dro;
} els_knurling = {
  .depth = 0.2,
  .depth_of_cut_um = 100, // fixed
  .length = 20,
  .diameter = 10,
  .angle = 30,
  .divisions = 6,
  .encoder_multiplier = 1,
  .op_stage = ELS_KNURLING_OP_STAGE1
};

//==============================================================================
// Internal functions
//==============================================================================
static void els_knurling_run(void);
static void els_knurling_thread_stage1(void);
static void els_knurling_thread_stage2(void);

static void els_knurling_display_refresh(void);
static void els_knurling_display_setting(void);
static void els_knurling_display_axes(void);
static void els_knurling_display_header(void);
static void els_knurling_display_diagram(void);
static void els_knurling_display_encoder_pips(void);

static void els_knurling_set_length(void);
static void els_knurling_set_depth(void);

static void els_knurling_set_diameter(void);
static void els_knurling_set_angle(void);
static void els_knurling_set_divisions(void);

static void els_knurling_axes_setup(void);
static void els_knurling_set_xaxes(void);
static void els_knurling_set_zaxes(void);

static void els_knurling_recalculate_pitch_ratio(void);

static void els_knurling_configure_timer(void);
static void els_knurling_configure_gpio(void);

static void els_knurling_timer_start(void);
static void els_knurling_timer_stop(void);

static void els_knurling_timer_isr(void) __attribute__ ((interrupt ("IRQ")));
static void els_knurling_encoder_isr(void) __attribute__ ((interrupt ("IRQ")));

static void els_knurling_keypad_process(void);

// manual jogging
static void els_knurling_zjog(void);
static void els_knurling_xjog(void);

static void els_knurling_zjog_sync(void);
static void els_knurling_xjog_sync(void);

//==============================================================================
// API
//==============================================================================
void els_knurling_setup(void) {
  // no-op
}

void els_knurling_start(void) {
  char text[32];

  els_knurling_configure_timer();
  els_knurling_configure_gpio();

  tft_filled_rectangle(&tft, 0,   0, 480, 320, ILI9481_BLACK);

  tft_filled_rectangle(&tft, 0,  50, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0,  50, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 100, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 100, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 190, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 190, 480,   1, ILI9481_LITEGRAY);

  els_knurling_display_header();

  // angle symbol
  tft_font_write_bg(&tft, 165,  62, "J", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // pulse symbol
  tft_font_write_bg(&tft, 355,  58, "K", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // reset state
  els_knurling.prev_state = 0;
  els_knurling.prev_dir = 0;
  els_knurling.phase_offset = 0;
  els_knurling.op_stage = ELS_KNURLING_OP_STAGE1;
  els_knurling.state = ELS_KNURLING_IDLE;

  if (!els_knurling.show_dro)
    els_knurling_display_diagram();

  els_knurling_recalculate_pitch_ratio();
  els_knurling_axes_setup();

  // reset isr
  els_nvic_irq_set_handler(ELS_KNURLING_TIMER_IRQ, els_knurling_timer_isr);

  // gpio / exti isr
  els_nvic_irq_set_handler(ELS_S_ENCODER2_IRQ, els_knurling_encoder_isr);

  // spindle encoder
  nvic_set_priority(ELS_S_ENCODER2_IRQ, 6);
  nvic_enable_irq(ELS_S_ENCODER2_IRQ);

  els_knurling_display_setting();
  els_knurling_display_axes();
  els_knurling_display_refresh();

  els_stepper_start();
  els_knurling_timer_start();
}

void els_knurling_stop(void) {
  els_stepper_stop();
  els_knurling_timer_stop();
  nvic_disable_irq(ELS_S_ENCODER2_IRQ);
}

bool els_knurling_busy(void) {
  return els_knurling.state != ELS_KNURLING_IDLE;
}

void els_knurling_update(void) {
  static uint64_t last_refreshed_at = 0;

  uint64_t elapsed = els_timer_elapsed_microseconds();

  bool kp_locked = els_keypad_locked();
  if (els_knurling.locked != kp_locked) {
    els_knurling.locked = kp_locked;
    els_knurling_display_header();
  }

  if (els_knurling.state & (ELS_KNURLING_IDLE | ELS_KNURLING_PAUSED | ELS_KNURLING_ACTIVE))
    els_knurling_keypad_process();

  switch (els_knurling.state) {
    case ELS_KNURLING_PAUSED:
    case ELS_KNURLING_ACTIVE:
      els_knurling_run();
      break;
    case ELS_KNURLING_SET_DEPTH:
      els_knurling_set_depth();
      break;
    case ELS_KNURLING_SET_LENGTH:
      els_knurling_set_length();
      break;
    case ELS_KNURLING_SET_DIAMETER:
      els_knurling_set_diameter();
      break;
    case ELS_KNURLING_SET_ANGLE:
      els_knurling_set_angle();
      break;
    case ELS_KNURLING_SET_DIVISIONS:
      els_knurling_set_divisions();
      break;
    default:
      if (els_knurling.state & (ELS_KNURLING_SET_ZAXES | ELS_KNURLING_ZJOG))
        els_knurling_set_zaxes();
      else if (els_knurling.state & (ELS_KNURLING_SET_XAXES | ELS_KNURLING_XJOG))
        els_knurling_set_xaxes();
      break;
  }

  if (elapsed - last_refreshed_at > 1e5) {
    last_refreshed_at = elapsed;
    els_knurling_display_refresh();
  }

  int16_t em = els_encoder_get_multiplier();
  if (em != els_knurling.encoder_multiplier) {
    els_knurling.encoder_multiplier = em;
    els_knurling_display_encoder_pips();
  }
}

//==============================================================================
// ISR
//==============================================================================
static void els_knurling_timer_isr(void) {
  static volatile bool pulse_pending = false;

  TIM_SR(ELS_KNURLING_TIMER) &= ~TIM_SR_UIF;

  if (pulse_pending) {
    els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    els_knurling.zcurrent++;
    els_stepper->zpos += (els_stepper->zdir * els_knurling.zdelta);

    if (els_knurling.op_stage == ELS_KNURLING_OP_STAGE1) {
      if (fabs((0 - els_stepper->zpos) - els_knurling.length) <= PRECISION) {
        els_knurling.op_state = ELS_KNURLING_OP_KNURLL;
        els_knurling.ztarget = 0;
        els_knurling.zcurrent = 0;
      }
    }
    else {
      if (fabs(0 - els_stepper->zpos) <= PRECISION) {
        els_knurling.op_state = ELS_KNURLING_OP_KNURLL;
        els_knurling.ztarget = 0;
        els_knurling.zcurrent = 0;
      }
    }
    pulse_pending = false;
  }

  if (els_knurling.state == ELS_KNURLING_ACTIVE &&
      els_knurling.op_state == ELS_KNURLING_OP_KNURL &&
      els_knurling.zcurrent < els_knurling.ztarget) {
    els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
    pulse_pending = true;
  }
}

static void els_knurling_encoder_isr(void) {
  static volatile uint32_t x1, x2;

  x1 = els_gpio_get(ELS_S_ENCODER2_PORTA, ELS_S_ENCODER2_PINA);
  x2 = els_gpio_get(ELS_S_ENCODER2_PORTB, ELS_S_ENCODER2_PINB);

  if (exti_get_flag_status(ELS_S_ENCODER2_EXTI)) {
    exti_reset_request(ELS_S_ENCODER2_EXTI);
    if (x1) {
      if (x2)
        els_knurling.spindle_dir = ELS_S_DIRECTION_CW;
      else
        els_knurling.spindle_dir = ELS_S_DIRECTION_CCW;
    }
    else {
      if (x2)
        els_knurling.spindle_dir = ELS_S_DIRECTION_CCW;
      else
        els_knurling.spindle_dir = ELS_S_DIRECTION_CW;
    }

    if (els_knurling.state == ELS_KNURLING_ACTIVE && els_knurling.op_state == ELS_KNURLING_OP_KNURL) {
      els_knurling.ztarget += els_knurling.pitch_p;

      els_knurling.pitch_curr += els_knurling.pitch_n;
      if (els_knurling.pitch_curr >= els_knurling.pitch_d) {
        els_knurling.pitch_curr -= els_knurling.pitch_d;
        els_knurling.ztarget++;
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
static void els_knurling_display_setting(void) {
  char text[32];

  els_sprint_double3(text, sizeof(text), els_knurling.length, "L");
  if (els_knurling.state == ELS_KNURLING_SET_LENGTH)
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_knurling.depth, "d");
  if (els_knurling.state == ELS_KNURLING_SET_DEPTH)
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_knurling.diameter, "D");
  if (els_knurling.state == ELS_KNURLING_SET_DIAMETER)
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_20, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 228, text, &noto_sans_mono_bold_20, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_knurling.angle, "A");
  if (els_knurling.state == ELS_KNURLING_SET_ANGLE)
    tft_font_write_bg(&tft, 310, 252, text, &noto_sans_mono_bold_20, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 252, text, &noto_sans_mono_bold_20, ILI9481_WHITE, ILI9481_BLACK);

  snprintf(text, sizeof(text),
    "N %03d/%03d",
    (els_knurling.state == ELS_KNURLING_ACTIVE ? (int)(els_knurling.phase_offset / els_knurling.phase_delta) + 1 : 0),
    els_knurling.divisions
  );

  if (els_knurling.state == ELS_KNURLING_SET_DIVISIONS)
    tft_font_write_bg(&tft, 310, 276, text, &noto_sans_mono_bold_20, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 310, 276, text, &noto_sans_mono_bold_20, ILI9481_WHITE, ILI9481_BLACK);
}

static void els_knurling_display_axes(void) {
  char text[32];

  els_sprint_double33(text, sizeof(text), els_stepper->zpos, "Z");

  if (els_knurling.state & ELS_KNURLING_SET_ZAXES)
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), els_stepper->xpos, "X");
  if (els_knurling.state & ELS_KNURLING_SET_XAXES)
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 135, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  if (els_knurling.show_dro) {
    tft_font_write_bg(&tft, 8, 190, "DRO", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.zpos_um / 1000.0), "Z");
    tft_font_write_bg(&tft, 8, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

    els_sprint_double33(text, sizeof(text), (els_dro.xpos_um / 1000.0), "X");
    tft_font_write_bg(&tft, 8, 262, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
  }
}

static void els_knurling_display_diagram(void) {
  // ----------------------------------------------------------------------------------------------
  // diagram
  // ----------------------------------------------------------------------------------------------
  tft_filled_rectangle(&tft, 100, 235, 150, 65, ILI9481_WHITE);

  const int16_t points[][4] = {
    {150, 235, 250, 300},
    {160, 235, 250, 293},
    {170, 235, 250, 286},
    {180, 235, 250, 279},
    {190, 235, 250, 273},
    {200, 235, 250, 267},
    {210, 235, 250, 260},
    {220, 235, 250, 253},
    {230, 235, 250, 246},
    {240, 235, 250, 239},

    {150, 242, 240, 300},
    {150, 249, 230, 300},
    {150, 256, 220, 300},
    {150, 263, 210, 300},
    {150, 270, 200, 300},
    {150, 277, 190, 300},
    {150, 284, 180, 300},
    {150, 291, 170, 300},
    {150, 298, 160, 300},

    {150, 300, 250, 235},
    {160, 300, 250, 242},
    {170, 300, 250, 249},
    {180, 300, 250, 256},
    {190, 300, 250, 263},
    {200, 300, 250, 270},
    {210, 300, 250, 277},
    {220, 300, 250, 284},
    {230, 300, 250, 291},
    {240, 300, 250, 298},

    {150, 300, 250, 235},
    {150, 293, 240, 235},
    {150, 286, 230, 235},
    {150, 279, 220, 235},
    {150, 272, 210, 235},
    {150, 265, 200, 235},
    {150, 258, 190, 235},
    {150, 251, 180, 235},
    {150, 244, 170, 235},
    {150, 237, 160, 235},
  };

  for (size_t i = 0; i < sizeof(points) / sizeof(points[0]); i++)
    tft_line(&tft, points[i][0], points[i][1], points[i][2], points[i][3], ILI9481_BLACK);

  tft_font_write_bg(&tft, 150, 183, "A", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 200, 199, "L", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 235, 183, "C", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);

  tft_font_write_bg(&tft, 260, 216, "B", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 265, 253, "D", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  tft_font_write_bg(&tft, 260, 265, "D", &noto_sans_mono_bold_arrows_24, ILI9481_WHITE, ILI9481_BLACK);

  // origin
  tft_font_write_bg(&tft, 245, 264, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);

  // legend
  tft_font_write_bg(&tft, 8, 200, ".", &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  tft_font_write_bg(&tft, 26, 220, "(0,0)", &inconsolata_lgc_bold_14, ILI9481_WHITE, ILI9481_BLACK);
  // ----------------------------------------------------------------------------------------------
}
static void els_knurling_display_header(void) {
  tft_rgb_t color = (els_knurling.locked ? ILI9481_RED : ILI9481_ORANGE);

  tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
  tft_font_write_bg(&tft, 8, 0, "KNURLING", &noto_sans_mono_bold_26, ILI9481_WHITE, color);
  els_knurling_display_encoder_pips();
}

static void els_knurling_display_encoder_pips(void) {
  // multiplier is 100, 10 or 1
  size_t pips = els_knurling.encoder_multiplier;
  pips = pips > 10 ? 3 : pips > 1 ? 2 : 1;

  for (size_t n = 0, spacing = 0; n < 3; n++, spacing++)
    tft_filled_rectangle(&tft,
      440, 32 - (n * 10 + spacing * 2),
      15 + n * 10, 10,
      (n < pips ? ILI9481_WHITE : els_knurling.locked ? ILI9481_LITEGRAY : ILI9481_BGCOLOR3));
}

static void els_knurling_display_refresh(void) {
  els_spindle_direction_t dir = els_spindle_get_direction();
  if (dir != els_knurling.prev_dir) {
    els_knurling.prev_dir = dir;
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

  els_knurling_display_axes();

  static els_knurling_op_state_t prev_op_state = ELS_KNURLING_OP_NA;
  if (els_knurling.op_state != prev_op_state) {
    prev_op_state = els_knurling.op_state;
    tft_filled_rectangle(&tft, 310, 195, 169, 30, ILI9481_BLACK);
    const char *label = op_labels[els_knurling.op_state];
    tft_font_write_bg(&tft, 310, 190, label, &noto_sans_mono_bold_26, ILI9481_ORANGE, ILI9481_BLACK);
  }
}

// ----------------------------------------------------------------------------------
// Top module level keypad handler.
// ----------------------------------------------------------------------------------
static void els_knurling_keypad_process(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
      if (els_knurling.state == ELS_KNURLING_IDLE) {
        if (els_knurling.length > 0) {
          els_knurling.state = ELS_KNURLING_PAUSED;
          els_knurling.op_state = ELS_KNURLING_OP_READY;
        }
        else {
          els_knurling.state = ELS_KNURLING_SET_LENGTH;
          els_knurling_display_setting();
        }
      }
      break;
    case ELS_KEY_EXIT:
      if (els_knurling.state & (ELS_KNURLING_PAUSED | ELS_KNURLING_ACTIVE)) {
        els_knurling.state = ELS_KNURLING_IDLE;
        els_knurling.op_state = ELS_KNURLING_OP_IDLE;
        els_knurling.phase_offset = 0;
        els_knurling.op_stage = ELS_KNURLING_OP_STAGE1;
      }
      break;
    case ELS_KEY_FUN_F1:
      if (els_knurling.state & (ELS_KNURLING_IDLE | ELS_KNURLING_PAUSED)) {
        els_knurling.state = ELS_KNURLING_SET_LENGTH;
        els_knurling.encoder_pos = 0;
        els_knurling_display_setting();
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F2:
      els_knurling.show_dro = !els_knurling.show_dro;
      tft_filled_rectangle(&tft, 0, 200, 300, 120, ILI9481_BLACK);
      if (els_knurling.show_dro)
        els_knurling_display_axes();
      else
        els_knurling_display_diagram();
      break;
    case ELS_KEY_SET_ZX:
      if (els_knurling.state & (ELS_KNURLING_IDLE | ELS_KNURLING_PAUSED)) {
        els_knurling.state = ELS_KNURLING_SET_ZAXES;
        els_knurling.encoder_pos = 0;
        els_knurling_display_axes();
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
static void els_knurling_run(void) {
  if (els_spindle_get_counter() == 0)
    els_knurling.state = ELS_KNURLING_ACTIVE;

  if (els_knurling.state == ELS_KNURLING_ACTIVE) {
    if (els_knurling.op_stage == ELS_KNURLING_OP_STAGE1)
      els_knurling_thread_stage1();
    else
      els_knurling_thread_stage2();
  }
}

// main control loop
static void els_knurling_thread_stage1(void) {
  int zdir = -1;

  switch (els_knurling.op_state) {
    case ELS_KNURLING_OP_NA:
      break;
    case ELS_KNURLING_OP_IDLE:
      els_knurling.ztarget = 0;
      els_knurling.zcurrent = 0;
      break;
    case ELS_KNURLING_OP_READY:
      els_knurling.ztarget = 0;
      els_knurling.zcurrent = 0;
      els_knurling.pitch_curr = 0;
      els_knurling.op_state = ELS_KNURLING_OP_MOVEZ0;
      els_knurling_display_setting();

      if (els_config->z_closed_loop)
        els_stepper->zpos = els_dro.zpos_um / 1000.0;
      if (els_config->x_closed_loop)
        els_stepper->xpos = els_dro.xpos_um / 1000.0;

      break;
    case ELS_KNURLING_OP_MOVEZ0:
      if (fabs(els_stepper->zpos) > PRECISION)
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);
      els_knurling.op_state = ELS_KNURLING_OP_MOVEX0;
      break;
    case ELS_KNURLING_OP_MOVEX0:
      if (els_stepper->zbusy)
        break;
      if (fabs(els_stepper->xpos) > PRECISION)
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_jog_mm_s);
      els_knurling.op_state = ELS_KNURLING_OP_START;
      break;
    case ELS_KNURLING_OP_START:
      if (els_stepper->xbusy)
        break;
      els_knurling.op_state = ELS_KNURLING_OP_ATZ0XM;
      break;
    case ELS_KNURLING_OP_ATZ0:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        if (els_knurling.spindle_dir == ELS_S_DIRECTION_CW) {
          ELS_KNURLING_SET_ZDIR_LR;
          if (els_stepper->zdir != -zdir)
            els_stepper_z_backlash_fix();
          els_stepper->zdir = -zdir;
        }
        else {
          ELS_KNURLING_SET_ZDIR_RL;
          if (els_stepper->zdir != zdir)
            els_stepper_z_backlash_fix();
          els_stepper->zdir = zdir;
        }
        if (els_spindle_get_counter() == els_knurling.phase_offset) {
          els_knurling.ztarget = 0;
          els_knurling.zcurrent = 0;
          els_knurling.op_state = ELS_KNURLING_OP_KNURL;
        }
      }
      break;
    case ELS_KNURLING_OP_KNURL:
      // handled by ISR
      break;
    case ELS_KNURLING_OP_KNURLL:
      els_knurling.xpos_prev = els_stepper->xpos;
      els_stepper_move_x(2 - els_stepper->xpos, els_config->x_jog_mm_s);
      els_knurling.op_state = ELS_KNURLING_OP_ATZL;
      break;
    case ELS_KNURLING_OP_ATZL:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);
        els_knurling.op_state = ELS_KNURLING_OP_ATZLXM;
      }
      break;
    case ELS_KNURLING_OP_ATZLXM:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        els_stepper_move_x(els_knurling.xpos_prev - els_stepper->xpos, els_config->x_jog_mm_s);
        els_knurling.op_state = ELS_KNURLING_OP_ATZ0XM;
      }
      break;
    case ELS_KNURLING_OP_ATZ0XM:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        if (fabs(els_knurling.depth + els_stepper->xpos) >= PRECISION) {
          double xd;
          xd = MIN(
            els_knurling.depth + els_stepper->xpos,
            els_knurling.depth_of_cut_um / 1000.0
          );

          els_stepper_move_x(-xd, els_config->x_jog_mm_s);
          els_knurling.op_state = ELS_KNURLING_OP_FEED_IN;
        }
        else if (els_knurling.phase_offset + els_knurling.phase_delta < els_config->spindle_encoder_ppr * 2) {
          els_knurling.phase_offset += els_knurling.phase_delta;
          els_stepper_move_x(0 - els_stepper->xpos, els_config->x_jog_mm_s);
          els_knurling.op_state = ELS_KNURLING_OP_FEED_IN;
          els_knurling_display_setting();
        }
        else {
          els_stepper_move_x(0 - els_stepper->xpos, els_config->x_jog_mm_s);
          els_knurling.op_state = ELS_KNURLING_OP_DONE;
        }
      }
      break;
    case ELS_KNURLING_OP_FEED_IN:
      if (!els_stepper->xbusy && !els_stepper->zbusy)
        els_knurling.op_state = ELS_KNURLING_OP_ATZ0;
      break;
    case ELS_KNURLING_OP_DONE:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        els_knurling.op_stage = ELS_KNURLING_OP_STAGE2;
        els_knurling.op_state = ELS_KNURLING_OP_READY;
        els_knurling.state = ELS_KNURLING_ACTIVE;
        els_knurling.phase_offset = 0;
        els_knurling_display_setting();
      }
      break;
    default:
      break;
  }
}

static void els_knurling_thread_stage2(void) {
  int zdir = 1;

  switch (els_knurling.op_state) {
    case ELS_KNURLING_OP_NA:
      break;
    case ELS_KNURLING_OP_IDLE:
      els_knurling.ztarget = 0;
      els_knurling.zcurrent = 0;
      break;
    case ELS_KNURLING_OP_READY:
      els_knurling.ztarget = 0;
      els_knurling.zcurrent = 0;
      els_knurling.pitch_curr = 0;
      els_knurling.op_state = ELS_KNURLING_OP_MOVEZL;
      els_knurling_display_setting();
      break;
    case ELS_KNURLING_OP_MOVEZL:
      if (fabs(-els_knurling.length - els_stepper->zpos) > PRECISION)
        els_stepper_move_z(-els_knurling.length - els_stepper->zpos, els_config->z_jog_mm_s);
      els_knurling.op_state = ELS_KNURLING_OP_MOVEX0;
      break;
    case ELS_KNURLING_OP_MOVEX0:
      if (els_stepper->zbusy)
        break;
      if (fabs(els_stepper->xpos) > PRECISION)
        els_stepper_move_x(0 - els_stepper->xpos, els_config->x_jog_mm_s);
      els_knurling.op_state = ELS_KNURLING_OP_START;
      break;
    case ELS_KNURLING_OP_START:
      if (els_stepper->xbusy)
        break;
      els_knurling.op_state = ELS_KNURLING_OP_ATZLXM;
      break;
    case ELS_KNURLING_OP_ATZL:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        if (els_knurling.spindle_dir == ELS_S_DIRECTION_CW) {
          ELS_KNURLING_SET_ZDIR_RL;
          if (els_stepper->zdir != -zdir)
            els_stepper_z_backlash_fix();
          els_stepper->zdir = -zdir;
        }
        else {
          ELS_KNURLING_SET_ZDIR_LR;
          if (els_stepper->zdir != zdir)
            els_stepper_z_backlash_fix();
          els_stepper->zdir = zdir;
        }
        if (els_spindle_get_counter() == els_knurling.phase_offset) {
          els_knurling.ztarget = 0;
          els_knurling.zcurrent = 0;
          els_knurling.op_state = ELS_KNURLING_OP_KNURL;
        }
      }
      break;
    case ELS_KNURLING_OP_KNURL:
      // handled by ISR
      break;
    case ELS_KNURLING_OP_KNURLL:
      els_knurling.xpos_prev = els_stepper->xpos;
      els_stepper_move_x(2 - els_stepper->xpos, els_config->x_jog_mm_s);
      els_knurling.op_state = ELS_KNURLING_OP_ATZ0;
      break;
    case ELS_KNURLING_OP_ATZ0:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        els_stepper_move_z(-els_knurling.length - els_stepper->zpos, els_config->z_jog_mm_s);
        els_knurling.op_state = ELS_KNURLING_OP_ATZ0XM;
      }
      break;
    case ELS_KNURLING_OP_ATZ0XM:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        els_stepper_move_x(els_knurling.xpos_prev - els_stepper->xpos, els_config->x_jog_mm_s);
        els_knurling.op_state = ELS_KNURLING_OP_ATZLXM;
      }
      break;
    case ELS_KNURLING_OP_ATZLXM:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        if (fabs(els_knurling.depth + els_stepper->xpos) >= PRECISION) {
          double xd;
          xd = MIN(
            els_knurling.depth + els_stepper->xpos,
            els_knurling.depth_of_cut_um / 1000.0
          );

          els_stepper_move_x(-xd, els_config->x_jog_mm_s);
          els_knurling.op_state = ELS_KNURLING_OP_FEED_IN;
        }
        else if (els_knurling.phase_offset + els_knurling.phase_delta < els_config->spindle_encoder_ppr * 2) {
          els_knurling.phase_offset += els_knurling.phase_delta;
          els_stepper_move_x(0 - els_stepper->xpos, els_config->x_jog_mm_s);
          els_knurling.op_state = ELS_KNURLING_OP_FEED_IN;
          els_knurling_display_setting();
        }
        else {
          els_stepper_move_x(0 - els_stepper->xpos, els_config->x_jog_mm_s);
          els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);
          els_knurling.op_state = ELS_KNURLING_OP_DONE;
        }
      }
      break;
    case ELS_KNURLING_OP_FEED_IN:
      if (!els_stepper->xbusy && !els_stepper->zbusy)
        els_knurling.op_state = ELS_KNURLING_OP_ATZL;
      break;
    case ELS_KNURLING_OP_DONE:
      if (!els_stepper->xbusy && !els_stepper->zbusy) {
        els_knurling.op_state = ELS_KNURLING_OP_IDLE;
        els_knurling.state = ELS_KNURLING_IDLE;
        els_knurling.op_stage = ELS_KNURLING_OP_STAGE1;
        els_knurling.phase_offset = 0;
        els_knurling_display_setting();
      }
      break;
    default:
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.1: knurl settings.
// ----------------------------------------------------------------------------------
static void els_knurling_recalculate_pitch_ratio(void) {
  // Calculate pitch
  //
  // http://www.mitsubishicarbide.com/index.php?cID=2884
  //
  // tan(θ) = P / πD
  //
  // P = tan(θ) * πD
  //
  els_knurling.pitch = tan(els_knurling.angle * M_PI / 180.0) * M_PI * els_knurling.diameter;

  uint32_t n, d, g;

  // Z stepper pulses required per rev for pitch.
  n = els_knurling.pitch * els_config->z_pulses_per_mm;

  // encoder pulses generated per rev.
  d = els_config->spindle_encoder_ppr;

  // whole Z pulses generated for one encoder pulse.
  els_knurling.pitch_p = n / d;

  // fractional Z pulse error
  n -= (els_knurling.pitch_p * d);
  g = els_gcd(n, d);
  els_knurling.pitch_n = n / g;
  els_knurling.pitch_d = d / g;
  els_knurling.pitch_curr = 0;

  els_knurling.phase_delta = (els_config->spindle_encoder_ppr * 2) / els_knurling.divisions;
  els_knurling.phase_offset = 0;

  printf("pitch %.2f p = %lu n = %lu d = %lu\n",
    els_knurling.pitch, els_knurling.pitch_p, els_knurling.pitch_n, els_knurling.pitch_d);
}

static void els_knurling_set_diameter(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_knurling.state = ELS_KNURLING_IDLE;
      els_knurling_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_knurling.state = ELS_KNURLING_SET_ANGLE;
      els_knurling_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_knurling.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_knurling.encoder_pos) * 0.01 * els_knurling.encoder_multiplier;
        if (els_knurling.diameter + delta >= 1)
          els_knurling.diameter += delta;
        els_knurling.encoder_pos = encoder_curr;
        els_knurling_display_setting();
        els_knurling_recalculate_pitch_ratio();
      }
      break;
  }
}

static void els_knurling_set_angle(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_knurling.state = ELS_KNURLING_IDLE;
      els_knurling_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_knurling.state = ELS_KNURLING_SET_DIVISIONS;
      els_knurling_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_knurling.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_knurling.encoder_pos) * 0.01 * els_knurling.encoder_multiplier;
        if (els_knurling.angle + delta < 20)
          els_knurling.angle = 20;
        else if (els_knurling.angle + delta > 60)
          els_knurling.angle = 60;
        else
          els_knurling.angle += delta;
        els_knurling.encoder_pos = encoder_curr;
        els_knurling_display_setting();
        els_knurling_recalculate_pitch_ratio();
      }
      break;
  }
}

static void els_knurling_set_divisions(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_knurling.state = ELS_KNURLING_IDLE;
      els_knurling_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_knurling.state = ELS_KNURLING_SET_LENGTH;
      els_knurling_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_knurling.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_knurling.encoder_pos);
        if (els_knurling.divisions + delta >= 2 && els_knurling.divisions + delta <= 60)
          els_knurling.divisions += delta;
        els_knurling.encoder_pos = encoder_curr;
        els_knurling_display_setting();
        els_knurling_recalculate_pitch_ratio();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2.2: depth of cut settings.
// ----------------------------------------------------------------------------------
static void els_knurling_set_depth(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_knurling.state = ELS_KNURLING_IDLE;
      els_knurling_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_knurling.state = ELS_KNURLING_SET_DIAMETER;
      els_knurling_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_knurling.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_knurling.encoder_pos) * 0.01 * els_knurling.encoder_multiplier;
        if (els_knurling.depth + delta <= ELS_KNURLING_DEPTH_MIN)
          els_knurling.depth = ELS_KNURLING_DEPTH_MIN;
        else if (els_knurling.depth + delta >= ELS_KNURLING_DEPTH_MAX)
          els_knurling.depth = ELS_KNURLING_DEPTH_MAX;
        else
          els_knurling.depth += delta;
        els_knurling.encoder_pos = encoder_curr;
        els_knurling_display_setting();
      }
      break;
  }
}

void els_knurling_set_length(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_knurling.state = ELS_KNURLING_IDLE;
      els_knurling_display_setting();
      break;
    case ELS_KEY_FUN_F1:
      els_knurling.state = ELS_KNURLING_SET_DEPTH;
      els_knurling_display_setting();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_knurling.encoder_pos != encoder_curr) {
        double delta = (encoder_curr - els_knurling.encoder_pos) * 0.01 * els_knurling.encoder_multiplier;
        if (els_knurling.length + delta <= 0)
          els_knurling.length = 0;
        else if (els_knurling.length + delta >= ELS_Z_MAX_MM)
          els_knurling.length = ELS_Z_MAX_MM;
        else
          els_knurling.length += delta;
        els_knurling.encoder_pos = encoder_curr;
        els_knurling_display_setting();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 3: Axis - position, origin & jogging
// ----------------------------------------------------------------------------------
static void els_knurling_set_zaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_knurling.state = (els_knurling.state & ELS_KNURLING_ZJOG) ? ELS_KNURLING_SET_ZAXES : ELS_KNURLING_IDLE;
      els_knurling_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_knurling.state == ELS_KNURLING_SET_ZAXES) {
        els_stepper->zpos = 0;
        els_dro_zero_z();
        els_knurling_display_axes();
      }
      break;
    case ELS_KEY_SET_ZX:
      els_knurling.state = ELS_KNURLING_SET_XAXES;
      els_knurling_display_axes();
      break;
    default:
      els_knurling_zjog();
      break;
  }
}

static void els_knurling_set_xaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_knurling.state = ELS_KNURLING_IDLE;
      els_knurling_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_knurling.state == ELS_KNURLING_SET_XAXES) {
        els_stepper->xpos = 0;
        els_dro_zero_x();
        els_knurling_display_axes();
      }
      break;
    case ELS_KEY_SET_ZX:
      els_knurling.state = ELS_KNURLING_SET_ZAXES;
      els_knurling_display_axes();
      break;
    default:
      els_knurling_xjog();
      break;
  }
}

// ----------------------------------------------------------------------------------
// Manual Jog
// ----------------------------------------------------------------------------------

static void els_knurling_zjog(void) {
  double delta;
  int32_t encoder_curr;

  els_knurling_zjog_sync();
  encoder_curr = els_encoder_read();
  if (els_knurling.encoder_pos != encoder_curr) {
    delta = (encoder_curr - els_knurling.encoder_pos) * 0.01 * els_knurling.encoder_multiplier;
    els_knurling.state |= ELS_KNURLING_ZJOG;
    els_stepper_move_z(delta, els_config->z_jog_mm_s);
    els_knurling.encoder_pos = encoder_curr;
    els_knurling_display_axes();
  }
}

static void els_knurling_zjog_sync(void) {
  if ((els_knurling.state & ELS_KNURLING_ZJOG) && !els_stepper->zbusy) {
    els_knurling.state &= ~ELS_KNURLING_ZJOG;
  }
}

static void els_knurling_xjog(void) {
  double delta;
  int32_t encoder_curr;

  els_knurling_xjog_sync();
  encoder_curr = els_encoder_read();
  if (els_knurling.encoder_pos != encoder_curr) {
    delta = (encoder_curr - els_knurling.encoder_pos) * 0.01 * els_knurling.encoder_multiplier;
    els_knurling.state |= ELS_KNURLING_XJOG;
    els_stepper_move_x(delta, els_config->x_jog_mm_s);
    els_knurling.encoder_pos = encoder_curr;
    els_knurling_display_axes();
  }
}

static void els_knurling_xjog_sync(void) {
  if ((els_knurling.state & ELS_KNURLING_XJOG) && !els_stepper->xbusy) {
    els_knurling.state &= ~ELS_KNURLING_XJOG;
  }
}

static void els_knurling_axes_setup(void) {
  // Z-axis movement per pulse in mm.
  els_knurling.zdelta  = (1.0 / (double)els_config->z_pulses_per_mm);

  // X-axis movement per pulse in mm.
  els_knurling.xdelta  = (1.0 / (double)els_config->x_pulses_per_mm);
}

// ----------------------------------------------------------------------------------
// GPIO: setup pins.
// ----------------------------------------------------------------------------------
static void els_knurling_configure_gpio(void) {
  gpio_mode_setup(ELS_S_ENCODER2_PORTA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, ELS_S_ENCODER2_PINA);
  gpio_mode_setup(ELS_S_ENCODER2_PORTB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, ELS_S_ENCODER2_PINB);

  exti_select_source(ELS_S_ENCODER2_EXTI, ELS_S_ENCODER2_PORTA);
  exti_set_trigger(ELS_S_ENCODER2_EXTI, EXTI_TRIGGER_RISING);
  exti_enable_request(ELS_S_ENCODER2_EXTI);
}

// ----------------------------------------------------------------------------------
// Timer functions.
// ----------------------------------------------------------------------------------
//
// Clock setup.
static void els_knurling_configure_timer(void) {
  rcc_periph_clock_enable(ELS_KNURLING_TIMER_RCC);
  rcc_periph_reset_pulse(ELS_KNURLING_TIMER_RST);

  // clock division 0, alignment edge, count up.
  timer_set_mode(ELS_KNURLING_TIMER, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  // 10us counter, ~100KHz
  timer_set_prescaler(ELS_KNURLING_TIMER, ((rcc_apb1_frequency * 2) / 100e3) - 1);

  // disable preload
  timer_disable_preload(ELS_KNURLING_TIMER);
  timer_continuous_mode(ELS_KNURLING_TIMER);

  nvic_set_priority(ELS_KNURLING_TIMER_IRQ, 4);
  nvic_enable_irq(ELS_KNURLING_TIMER_IRQ);
  timer_enable_update_event(ELS_KNURLING_TIMER);

  // 25KHz Z stepper pulse train
  timer_set_period(ELS_KNURLING_TIMER, 2);
}

static void els_knurling_timer_start(void) {
  timer_enable_counter(ELS_KNURLING_TIMER);
  timer_enable_irq(ELS_KNURLING_TIMER, TIM_DIER_UIE);
}

static void els_knurling_timer_stop(void) {
  timer_disable_irq(ELS_KNURLING_TIMER, TIM_DIER_UIE);
  timer_disable_counter(ELS_KNURLING_TIMER);
}
