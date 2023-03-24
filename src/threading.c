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
#include "keypad.h"
#include "stepper.h"
#include "threading.h"
#include "utils.h"

#define ELS_THREADING_PITCH_MIN_UM  100
#define ELS_THREADING_PITCH_MAX_UM  5000

//==============================================================================
// Externs
//==============================================================================
extern tft_device_t tft;

extern const tft_font_t noto_sans_mono_bold_26;
extern const tft_font_t gears_regular_32;
extern const tft_font_t gears_regular_50;
extern const tft_font_t inconsolata_lgc_bold_14;

//==============================================================================
// Config
//==============================================================================
typedef enum {
  ELS_THREADING_IDLE       = 1,
  ELS_THREADING_PAUSED     = 2,
  ELS_THREADING_ACTIVE     = 4,
  ELS_THREADING_SET_PITCH  = 8,
  ELS_THREADING_SET_ZAXES  = 16,
  ELS_THREADING_SET_XAXES  = 32,
  ELS_THREADING_SET_LENGTH = 64,
  ELS_THREADING_ZLIM       = 128
} els_threading_state_t;

typedef enum {
  ELS_THREADING_PITCH_STD = 0,
  ELS_THREADING_PITCH_ARB = 1
} els_thread_pitch_t;

#define ELS_THREADING_SET_ZDIR_LR   els_gpio_set(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)
#define ELS_THREADING_SET_ZDIR_RL   els_gpio_clear(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)

#define PRECISION                   (1e-2)

//==============================================================================
// Internal state
//==============================================================================
static struct {
  uint16_t pitch_um;
  bool     pitch_reverse;

  els_thread_pitch_t pitch_type;
  int16_t  pitch_table_index;

  bool     locked;

  // z-axis state & config
  double   length;
  double   zdelta;

  // input read for jogging
  int32_t  encoder_pos;

  // encoder multiplier
  int16_t encoder_multiplier;

  // spindle
  els_spindle_direction_t spindle_dir;

  // tracking variables for display refresh.
  els_threading_state_t prev_state;
  els_spindle_direction_t prev_dir;

  // thread pitch ratio
  uint32_t pitch_n;
  uint32_t pitch_d;

  // current error or value.
  volatile uint32_t pitch_curr;

  // module state
  els_threading_state_t state;

} els_threading = {
  // default to M5
  .pitch_table_index = 4,
  .pitch_um = 800,
  .length = 10
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
//
// The
#define ELS_THREADING_PITCH_TABLE_SIZE (sizeof(pitch_table_um) / sizeof(pitch_table_um[0]))
static const uint16_t pitch_table_um[] = {
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

static const char *pitch_table_label[] = {
// Metric thread, Male Thread Height, Female Thread Depth, Drill Size
"M3   H>0.307 H<0.271",
"M3.5 H>0.368 H<0.325",
"M4   H>0.429 H<0.379",
"M4.5 H>0.460 H<0.406",
"M5   H>0.491 H<0.433",
"M6   H>0.613 H<0.541",
"M7   H>0.613 H<0.541",
"M8   H>0.767 H<0.677",
"M9   H>0.767 H<0.677",
"M10  H>0.920 H<0.812",
"M11  H>0.920 H<0.812",
"M12  H>1.074 H<0.947",
"M14  H>1.227 H<1.083",
"M16  H>1.227 H<1.083",
"M18  H>1.534 H<1.353",
"M20  H>1.534 H<1.353",
"M22  H>1.534 H<1.353",
"M24  H>1.840 H<1.624",
"M27  H>1.840 H<1.624"
};

//==============================================================================
// Internal functions
//==============================================================================
static void els_threading_run(void);
static void els_threading_display_refresh(void);
static void els_threading_display_pitch(void);
static void els_threading_display_pitch_type(void);
static void els_threading_display_axes(void);
static void els_threading_display_header(void);
static void els_threading_display_encoder_pips(void);

static void els_threading_set_pitch(void);
static void els_threading_set_length(void);

static void els_threading_set_zaxes(void);
static void els_threading_set_xaxes(void);

static void els_threading_handle_zlim(void);

static void els_threading_recalculate_pitch_ratio(void);

static void els_threading_configure_gpio(void);

static void els_threading_encoder_isr(void) __attribute__ ((interrupt ("IRQ")));

static void els_threading_keypad_process(void);

static void els_threading_enable_z(void);
static void els_threading_disable_z(void);

// manual jogging
static void els_threading_zjog(void);

//==============================================================================
// API
//==============================================================================
void els_threading_setup(void) {
  // no-op
}

void els_threading_start(void) {
  char text[32];

  els_threading_configure_gpio();

  tft_filled_rectangle(&tft, 0,   0, 480, 320, ILI9481_BLACK);

  tft_filled_rectangle(&tft, 0,  50, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0,  50, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 100, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 100, 480,   1, ILI9481_LITEGRAY);

  tft_filled_rectangle(&tft, 0, 190, 480,   5, ILI9481_GRAY);
  tft_filled_rectangle(&tft, 0, 190, 480,   1, ILI9481_LITEGRAY);

  els_threading_display_header();

  // angle symbol
  tft_font_write_bg(&tft, 165,  62, "J", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // pulse symbol
  tft_font_write_bg(&tft, 355,  58, "K", &gears_regular_32, ILI9481_WHITE, ILI9481_BLACK);

  // reset state
  els_threading.prev_state = 0;
  els_threading.prev_dir = 0;

  // gpio / exti isr
  els_nvic_irq_set_handler(ELS_S_ENCODER2_IRQ, els_threading_encoder_isr);

  els_threading.state = ELS_THREADING_IDLE;

  els_threading_display_pitch();

  tft_font_write_bg(&tft, 8, 190, "DRO", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
  els_threading_display_axes();

  els_threading_display_refresh();

  els_threading_recalculate_pitch_ratio();

  // spindle encoder
  nvic_set_priority(ELS_S_ENCODER2_IRQ, 4);
  nvic_enable_irq(ELS_S_ENCODER2_IRQ);

  els_stepper_start();
  els_stepper_disable_x();

  // Z-axis movement per pulse in mm.
  els_threading.zdelta = (1.0 / (double)els_config->z_pulses_per_mm);
}

void els_threading_stop(void) {
  nvic_disable_irq(ELS_S_ENCODER2_IRQ);
  els_stepper_stop();
}

void els_threading_update(void) {
  static uint64_t last_refreshed_at = 0;

  uint64_t elapsed = els_timer_elapsed_microseconds();

  bool kp_locked = els_keypad_locked();
  if (els_threading.locked != kp_locked) {
    els_threading.locked = kp_locked;
    els_threading_display_header();
  }

  if (els_threading.state & (ELS_THREADING_IDLE | ELS_THREADING_PAUSED | ELS_THREADING_ACTIVE))
    els_threading_keypad_process();

  switch (els_threading.state) {
    case ELS_THREADING_PAUSED:
    case ELS_THREADING_ACTIVE:
      els_threading_run();
      break;
    case ELS_THREADING_SET_PITCH:
      els_threading_set_pitch();
      break;
    case ELS_THREADING_SET_LENGTH:
      els_threading_set_length();
      break;
    case ELS_THREADING_SET_XAXES:
      els_threading_set_xaxes();
      break;
    case ELS_THREADING_SET_ZAXES:
      els_threading_set_zaxes();
      break;
    case ELS_THREADING_ZLIM:
      els_threading_handle_zlim();
      break;
    default:
      break;
  }

  if (elapsed - last_refreshed_at > 1e5) {
    last_refreshed_at = elapsed;
    els_threading_display_refresh();
  }

  int16_t em = els_encoder_get_multiplier();
  if (em != els_threading.encoder_multiplier) {
    els_threading.encoder_multiplier = em;
    els_threading_display_encoder_pips();
  }
}

//==============================================================================
// ISR
//==============================================================================
static volatile bool reset_pulse_pending = false;

static void els_threading_encoder_isr(void) {
#if ELS_S_ENCODER2_PINA > GPIO4
  if (exti_get_flag_status(ELS_S_ENCODER2_EXTI)) {
#endif

  exti_reset_request(ELS_S_ENCODER2_EXTI);
  if (els_gpio_get(ELS_S_ENCODER2_PORTA, ELS_S_ENCODER2_PINA)) {
    if (els_gpio_get(ELS_S_ENCODER2_PORTB, ELS_S_ENCODER2_PINB)) {
      els_threading.spindle_dir = ELS_S_DIRECTION_CW;
    }
    else {
      els_threading.spindle_dir = ELS_S_DIRECTION_CCW;
    }
  }
  else {
    if (els_gpio_get(ELS_S_ENCODER2_PORTB, ELS_S_ENCODER2_PINB)) {
      els_threading.spindle_dir = ELS_S_DIRECTION_CCW;
    }
    else {
      els_threading.spindle_dir = ELS_S_DIRECTION_CW;
    }
  }

  if (els_threading.state == ELS_THREADING_ACTIVE) {
    if (reset_pulse_pending) {
      els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
      reset_pulse_pending = false;

      if (els_config->z_closed_loop)
        els_stepper->zpos = (double)els_dro.zpos_um * 1e-3;
      else
        els_stepper->zpos += (els_stepper->zdir * els_threading.zdelta);

      if (els_stepper->zdir * els_stepper->zpos >= els_threading.length)
        els_threading.state = ELS_THREADING_ZLIM;
    }

    els_threading.pitch_curr += els_threading.pitch_n;
    if (els_threading.pitch_curr >= els_threading.pitch_d) {
      els_threading.pitch_curr -= els_threading.pitch_d;
      els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
      reset_pulse_pending = true;
    }
  }

#if ELS_S_ENCODER2_PINA > GPIO4
  }
#endif
}

//==============================================================================
// Internal functions
//==============================================================================

// ----------------------------------------------------------------------------------
// Display functions
// ----------------------------------------------------------------------------------
static void els_threading_display_pitch(void) {
  char text[32];

  els_sprint_double13(text, sizeof(text), els_threading.pitch_um / 1000.0, "PITCH");
  if (els_threading.state == ELS_THREADING_SET_PITCH)
    tft_font_write_bg(&tft, 249, 228, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 249, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);


  if (els_threading.pitch_type == ELS_THREADING_PITCH_STD) {
    tft_filled_rectangle(&tft, 249, 200, 260, 25, ILI9481_BLACK);
    tft_font_write_bg(&tft,
      249, 200,
      pitch_table_label[els_threading.pitch_table_index],
      &inconsolata_lgc_bold_14, ILI9481_YELLOW, ILI9481_BLACK);
  }
}

static void els_threading_display_pitch_type(void) {
  if (els_threading.pitch_type == ELS_THREADING_PITCH_STD)
    tft_font_write_bg(&tft, 249, 262, "COARSE", &noto_sans_mono_bold_26, ILI9481_FOREST, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 249, 262, "CUSTOM", &noto_sans_mono_bold_26, ILI9481_FOREST, ILI9481_BLACK);
}

static void els_threading_display_axes(void) {
  char text[32];

  els_sprint_double33(text, sizeof(text), els_stepper->zpos, "Z");
  if (els_threading.state & ELS_THREADING_SET_ZAXES)
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_threading.length, "LEN");
  if (els_threading.state == ELS_THREADING_SET_LENGTH)
    tft_font_write_bg(&tft, 270, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 270, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  tft_font_write_bg(&tft, 8, 142, "LIMITS ON", &noto_sans_mono_bold_26, ILI9481_FOREST, ILI9481_BLACK);
}

static void els_threading_display_header(void) {
  if (els_threading.pitch_reverse) {
    tft_rgb_t color = els_threading.locked ? ILI9481_RED : ILI9481_GREEN;
    tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
    tft_font_write_bg(&tft, 8, 0, "MANUAL THREADING - L", &noto_sans_mono_bold_26, ILI9481_BLACK, color);
  }
  else {
    tft_rgb_t color = els_threading.locked ? ILI9481_RED : ILI9481_FOREST;
    tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
    tft_font_write_bg(&tft, 8, 0, "MANUAL THREADING - R", &noto_sans_mono_bold_26, ILI9481_WHITE, color);
  }

  els_threading_display_encoder_pips();
}

static void els_threading_display_encoder_pips(void) {
  // multiplier is 100, 10 or 1
  size_t pips = els_threading.encoder_multiplier;
  pips = pips > 10 ? 3 : pips > 1 ? 2 : 1;

  tft_rgb_t color = (els_threading.pitch_reverse ? ILI9481_BLACK : ILI9481_WHITE);

  for (size_t n = 0, spacing = 0; n < 3; n++, spacing++)
    tft_filled_rectangle(&tft,
      440, 32 - (n * 10 + spacing * 2),
      15 + n * 10, 10,
      (n < pips ? color : els_threading.locked ? ILI9481_LITEGRAY : ILI9481_BGCOLOR2));
}

static void els_threading_display_refresh(void) {
  els_spindle_direction_t dir = els_spindle_get_direction();
  if (dir != els_threading.prev_dir) {
    els_threading.prev_dir = dir;
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

  if (els_threading.prev_state != els_threading.state) {
    els_threading.prev_state = els_threading.state;
    tft_filled_rectangle(&tft, 249, 265, 149, 35, ILI9481_BLACK);
    const uint16_t x = 249, y = 262;
    switch (els_threading.state) {
      case ELS_THREADING_IDLE:
        tft_font_write_bg(&tft, x, y, "IDLE", &noto_sans_mono_bold_26, ILI9481_FOREST, ILI9481_BLACK);
        break;
      case ELS_THREADING_PAUSED:
        tft_font_write_bg(&tft, x, y, "READY", &noto_sans_mono_bold_26, ILI9481_ORANGE, ILI9481_BLACK);
        break;
      case ELS_THREADING_ACTIVE:
        tft_font_write_bg(&tft, x, y, "ACTIVE", &noto_sans_mono_bold_26, ILI9481_GREEN, ILI9481_BLACK);
        break;
      case ELS_THREADING_SET_PITCH:
        els_threading_display_pitch_type();
        break;
      case ELS_THREADING_SET_ZAXES:
        tft_font_write_bg(&tft, x, y, "Z AXIS", &noto_sans_mono_bold_26, ILI9481_FOREST, ILI9481_BLACK);
        break;
      case ELS_THREADING_SET_XAXES:
        tft_font_write_bg(&tft, x, y, "X AXIS", &noto_sans_mono_bold_26, ILI9481_FOREST, ILI9481_BLACK);
        break;
      case ELS_THREADING_SET_LENGTH:
        tft_font_write_bg(&tft, x, y, "LENGTH", &noto_sans_mono_bold_26, ILI9481_FOREST, ILI9481_BLACK);
        break;
      case ELS_THREADING_ZLIM:
        tft_font_write_bg(&tft, x, y, "Z LIMIT", &noto_sans_mono_bold_26, ILI9481_RED, ILI9481_BLACK);
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
  if (els_threading.state & ELS_THREADING_SET_ZAXES)
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), (els_dro.zpos_um / 1000.0), "Z");
  if (els_threading.state & ELS_THREADING_SET_ZAXES)
    tft_font_write_bg(&tft, 8, 228, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), (els_dro.xpos_um / 1000.0), "X");
  if (els_threading.state & ELS_THREADING_SET_XAXES)
    tft_font_write_bg(&tft, 8, 262, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 8, 262, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
}

// ----------------------------------------------------------------------------------
// Top module level keypad handler.
// ----------------------------------------------------------------------------------
static void els_threading_keypad_process(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
      if (els_threading.state == ELS_THREADING_IDLE)
        els_threading.state = ELS_THREADING_PAUSED;
      break;
    case ELS_KEY_EXIT:
      if (els_threading.state & (ELS_THREADING_PAUSED | ELS_THREADING_ACTIVE))
        els_threading.state = ELS_THREADING_IDLE;
      els_encoder_set_rotation_debounce(25e3);
      break;
    case ELS_KEY_SET_FEED:
      els_threading.state = ELS_THREADING_SET_PITCH;
      els_threading.encoder_pos = 0;
      els_encoder_set_rotation_debounce(50e3);
      els_encoder_reset();
      els_threading_display_pitch();
      break;
    case ELS_KEY_SET_ZX:
      if (els_threading.state & (ELS_THREADING_IDLE | ELS_THREADING_PAUSED)) {
        els_threading.state = ELS_THREADING_SET_ZAXES;
        els_encoder_set_rotation_debounce(25e3);
        els_threading.encoder_pos = 0;
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F1:
      els_encoder_reset();
      els_threading.state = ELS_THREADING_SET_LENGTH;
      els_threading_display_axes();
      break;
    default:
      break;
  }
}

// ---------------------------------------------------------------------------------------
// Function 1: primary threading handler, detects direction change and toggles the DIR pin.
// ---------------------------------------------------------------------------------------
static void els_threading_run(void) {
  int zdir = (els_threading.pitch_reverse ? 1 : -1);

  switch (els_threading.spindle_dir) {
    case ELS_S_DIRECTION_CCW:
    case ELS_S_DIRECTION_CW:
      if (els_stepper->zbusy)
        break;

      if (els_stepper->zdir != zdir) {
        els_stepper->zdir = zdir;
        if (zdir == 1)
          ELS_THREADING_SET_ZDIR_LR;
        else
          ELS_THREADING_SET_ZDIR_RL;
        els_stepper_z_backlash_fix();
      }

      if (els_threading.state == ELS_THREADING_PAUSED && els_spindle_get_counter() == 0) {
        els_threading.pitch_curr = 0;
        els_threading.state = ELS_THREADING_ACTIVE;
      }
      break;

    default:
      if (els_threading.state == ELS_THREADING_ACTIVE)
        els_threading.state = ELS_THREADING_PAUSED;
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2: pitch settings.
// ----------------------------------------------------------------------------------
//
static void els_threading_recalculate_pitch_ratio(void) {
  uint32_t n = (els_threading.pitch_um * els_config->z_pulses_per_mm) / 1000;
  uint32_t d = els_config->spindle_encoder_ppr;

  uint32_t g = els_gcd(n, d);

  els_threading.pitch_n = n / g;
  els_threading.pitch_d = d / g;
  els_threading.pitch_curr = 0;

  els_printf("pitch n = %lu d = %lu\n", els_threading.pitch_n, els_threading.pitch_d);
}

static void els_threading_set_pitch(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_threading.state = ELS_THREADING_IDLE;
      els_threading_display_pitch();
      break;
    case ELS_KEY_SET_FEED:
      els_threading.pitch_type = (els_threading.pitch_type + 1) % 2;
      els_threading_display_pitch_type();
      break;
    case ELS_KEY_REV_FEED:
      els_threading.pitch_reverse = !els_threading.pitch_reverse;
      els_threading_display_pitch();
      els_threading_display_header();
      els_threading_display_encoder_pips();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (encoder_curr != els_threading.encoder_pos) {
        if (els_threading.pitch_type == ELS_THREADING_PITCH_STD) {
          if (encoder_curr > els_threading.encoder_pos)
            els_threading.pitch_table_index++;
          else
            els_threading.pitch_table_index--;

          if (els_threading.pitch_table_index >= (int16_t)(ELS_THREADING_PITCH_TABLE_SIZE & 0xff))
            els_threading.pitch_table_index = 0;
          if (els_threading.pitch_table_index < 0)
            els_threading.pitch_table_index = ELS_THREADING_PITCH_TABLE_SIZE - 1;

          els_threading.pitch_um = pitch_table_um[els_threading.pitch_table_index];
        }
        else {
          if (encoder_curr > els_threading.encoder_pos)
            els_threading.pitch_um = els_threading.pitch_um + els_threading.encoder_multiplier < ELS_THREADING_PITCH_MAX_UM ?
              els_threading.pitch_um + els_threading.encoder_multiplier : ELS_THREADING_PITCH_MAX_UM;
          else
            els_threading.pitch_um = els_threading.pitch_um > els_threading.encoder_multiplier + ELS_THREADING_PITCH_MIN_UM ?
              els_threading.pitch_um - els_threading.encoder_multiplier : ELS_THREADING_PITCH_MIN_UM;
        }

        els_threading.encoder_pos = encoder_curr;
        els_threading_display_pitch();
        els_threading_recalculate_pitch_ratio();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 3: Z Axis - limits & jogging
// ----------------------------------------------------------------------------------
static void els_threading_set_zaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_EXIT:
      els_threading.state = ELS_THREADING_IDLE;
      break;

    case ELS_KEY_SET_ZX_ORI:
      if (els_stepper->zbusy)
        break;
      els_dro_zero_z();
      els_stepper_zero_z();
      els_threading_display_axes();
      break;

    case ELS_KEY_JOG_ZX_ORI:
      if (!els_stepper->zbusy) {
        if (els_config->z_closed_loop)
          els_stepper->zpos = (double)els_dro.zpos_um * 1e-3;
        els_stepper_move_z(0 - els_stepper->zpos, els_config->z_jog_mm_s);
      }
      break;

    case ELS_KEY_SET_ZX:
      els_threading.state = ELS_THREADING_SET_XAXES;
      els_threading_display_axes();
      break;
    default:
      els_threading_zjog();
      break;
  }
}

static void els_threading_set_xaxes(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_threading.state = ELS_THREADING_IDLE;
      els_threading_display_axes();
      break;
    case ELS_KEY_SET_ZX_ORI:
      els_dro_zero_x();
      els_threading_display_axes();
      break;
    case ELS_KEY_SET_ZX:
      els_encoder_reset();
      els_threading.state = ELS_THREADING_SET_ZAXES;
      els_threading_display_axes();
      break;
    default:
      break;
  }
}

static void els_threading_handle_zlim(void) {
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      if (!els_stepper->zbusy) {
        els_threading.state = ELS_THREADING_IDLE;
        els_threading_display_axes();
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
        els_threading.state = ELS_THREADING_IDLE;
      break;
  }
}

static void els_threading_set_length(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_threading.state = ELS_THREADING_IDLE;
      els_threading_display_axes();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_threading.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_threading.encoder_pos);
        els_threading.length += (delta * 0.01 * els_threading.encoder_multiplier);
        els_threading.encoder_pos = encoder_curr;
        els_threading_display_axes();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Manual Jog
// ----------------------------------------------------------------------------------
static void els_threading_zjog(void) {
  double  delta, step;
  int32_t  encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_threading.encoder_pos != encoder_curr) {
    step = els_threading.encoder_multiplier == 1 ? 0.005 : 0.01 * els_threading.encoder_multiplier;
    delta = (encoder_curr - els_threading.encoder_pos) * step;
    els_threading.encoder_pos = encoder_curr;
    els_stepper_move_z(delta, els_config->z_jog_mm_s);
  }
}

// ----------------------------------------------------------------------------------
// GPIO: setup pins.
// ----------------------------------------------------------------------------------
static void els_threading_configure_gpio(void) {
  gpio_mode_setup(ELS_S_ENCODER2_PORTA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, ELS_S_ENCODER2_PINA);
  gpio_mode_setup(ELS_S_ENCODER2_PORTB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, ELS_S_ENCODER2_PINB);

  exti_select_source(ELS_S_ENCODER2_EXTI, ELS_S_ENCODER2_PORTA);
  exti_set_trigger(ELS_S_ENCODER2_EXTI, EXTI_TRIGGER_RISING);
  exti_enable_request(ELS_S_ENCODER2_EXTI);
}

bool els_threading_busy(void) {
  return els_threading.state != ELS_THREADING_IDLE;
}
