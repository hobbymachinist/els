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
#include "threading.h"
#include "utils.h"

#define ELS_THREADING_TIMER         TIM4
#define ELS_THREADING_TIMER_IRQ     NVIC_TIM4_IRQ
#define ELS_THREADING_TIMER_MAX     UINT16_MAX
#define ELS_THREADING_TIMER_RCC     RCC_TIM4
#define ELS_THREADING_TIMER_RST     RST_TIM4

#define ELS_THREADING_Z_JOG_FEED_UM 6000

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
  ELS_THREADING_IDLE      = 1,
  ELS_THREADING_PAUSED    = 2,
  ELS_THREADING_ACTIVE    = 4,
  ELS_THREADING_SET_PITCH = 8,
  ELS_THREADING_SET_ZAXES = 16,
  ELS_THREADING_SET_XAXES = 32,
  ELS_THREADING_ZLIM_MIN  = 64,
  ELS_THREADING_ZLIM_MAX  = 128,
  ELS_THREADING_ZJOG      = 256,
  ELS_THREADING_SET_MIN   = 512,
  ELS_THREADING_SET_MAX   = 1024
} els_threading_state_t;

typedef enum {
  ELS_THREADING_PITCH_STD = 0,
  ELS_THREADING_PITCH_ARB = 1
} els_thread_pitch_t;

#define ELS_THREADING_ZJOG_PULSES         (els_config->z_pulses_per_mm / 10)

#define ELS_THREADING_SET_ZDIR_LR         els_gpio_set(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)
#define ELS_THREADING_SET_ZDIR_RL         els_gpio_clear(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN)

#define ELS_THREADING_Z_BACKLASH_FIX      do { \
                                           if (els_threading.zdir != 0 && els_config->z_backlash_pulses) {   \
                                             els_printf("backlash compensation\n");                          \
                                             for (size_t _n = 0; _n < els_config->z_backlash_pulses; _n++) { \
                                               els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);                  \
                                               els_delay_microseconds(ELS_BACKLASH_DELAY_US);                \
                                               els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);                \
                                               els_delay_microseconds(ELS_BACKLASH_DELAY_US);                \
                                             }                                                               \
                                           }                                                                 \
                                         } while (0)


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
  double   zpos;
  double   zmin;
  double   zmax;
  double   zdelta;
  int      zdir;

  // input read for jogging
  int32_t  encoder_pos;

  // encoder multiplier
  int16_t encoder_multiplier;

  // jogging
  int32_t  stepper_pulse_pos;
  int32_t  stepper_pulse_curr;
  int32_t  zjog_steps;

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
  .zpos = 0,
  .zmin = 0,
  .zmax = 0,
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
static void els_threading_set_min(void);
static void els_threading_set_max(void);

static void els_threading_set_zaxes(void);
static void els_threading_set_xaxes(void);

static void els_threading_recalulate_pitch_ratio(void);

static void els_threading_configure_gpio(void);
static void els_threading_configure_timer(void);

static void els_threading_timer_isr(void);
static void els_threading_timer_update(int32_t feed_um);
static void els_threading_timer_start(void);
static void els_threading_timer_stop(void);

static void els_threading_encoder_isr(void);

static void els_threading_keypad_process(void);

static void els_threading_enable_z(void);
static void els_threading_disable_z(void);

// manual jogging
static void els_threading_zjog(void);
static void els_threading_zjog_sync(void);
static void els_threading_zjog_pulse(void);

// auto jog to min / max
static void els_threading_zjog_min(void);
static void els_threading_zjog_max(void);
static void els_threading_zjog_auto(double travel);

//==============================================================================
// API
//==============================================================================
void els_threading_setup(void) {
  // no-op
}

void els_threading_start(void) {
  char text[32];

  els_threading_configure_gpio();
  els_threading_configure_timer();

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
  els_threading.zpos = 0;
  els_threading.zmin = 0;
  els_threading.zmax = 10; // XXX

  // reset isr
  els_nvic_irq_set_handler(ELS_THREADING_TIMER_IRQ, els_threading_timer_isr);

  // gpio / exti isr
  els_nvic_irq_set_handler(ELS_S_ENCODER2_IRQ, els_threading_encoder_isr);

  els_threading_timer_update(ELS_THREADING_Z_JOG_FEED_UM);
  els_threading_timer_start();

  els_threading.state = ELS_THREADING_IDLE;

  els_threading_display_pitch();

  tft_font_write_bg(&tft, 8, 190, "DRO", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
  els_threading_display_axes();

  els_threading_display_refresh();

  els_threading_recalulate_pitch_ratio();

  // spindle encoder
  nvic_set_priority(ELS_S_ENCODER2_IRQ, 4);
  nvic_enable_irq(ELS_S_ENCODER2_IRQ);

  els_threading_enable_z();
}

void els_threading_stop(void) {
  els_threading_timer_stop();
  nvic_disable_irq(ELS_S_ENCODER2_IRQ);
  els_threading_disable_z();
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
    case ELS_THREADING_SET_MIN:
      els_threading_set_min();
      break;
    case ELS_THREADING_SET_MAX:
      els_threading_set_max();
      break;
    case ELS_THREADING_SET_XAXES:
      els_threading_set_xaxes();
      break;
    default:
      if (els_threading.state &
         (ELS_THREADING_SET_ZAXES | ELS_THREADING_ZLIM_MIN | ELS_THREADING_ZLIM_MAX | ELS_THREADING_ZJOG))
        els_threading_set_zaxes();
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
static volatile bool z_pul_state = 0;
static void els_threading_timer_isr(void) {
  TIM_SR(ELS_THREADING_TIMER) &= ~TIM_SR_UIF;

  if (els_threading.state & ELS_THREADING_ZJOG) {
    if (els_threading.zjog_steps > 0) {
      if (z_pul_state)
        els_gpio_set(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);
      else
        els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);

      if (!z_pul_state) {
        els_threading.zpos += (els_threading.zdir * els_threading.zdelta);
        els_threading.zjog_steps--;
      }

      z_pul_state = !z_pul_state;
    }
  }
}

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
      els_threading.zpos += (els_threading.zdir * els_threading.zdelta);

      if (els_threading.zpos <= els_threading.zmin)
        els_threading.state = ELS_THREADING_ZLIM_MIN;
      else if (els_threading.zpos >= els_threading.zmax)
        els_threading.state = ELS_THREADING_ZLIM_MAX;
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

  if (els_threading.pitch_reverse)
    tft_font_write_bg(&tft, 186, 102, "R", &noto_sans_mono_bold_26, ILI9481_GREEN, ILI9481_BLACK);
  else
    tft_filled_rectangle(&tft, 186, 110, 30, 30, ILI9481_BLACK);

  els_sprint_double2(text, sizeof(text), els_threading.pitch_um / 1000.0, "PITCH");
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

  els_sprint_double33(text, sizeof(text), els_threading.zpos, "Z");
  tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_threading.zmin, "MIN");
  if (els_threading.state == ELS_THREADING_SET_MIN)
    tft_font_write_bg(&tft, 270, 102, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 270, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double3(text, sizeof(text), els_threading.zmax, "MAX");
  if (els_threading.state == ELS_THREADING_SET_MAX)
    tft_font_write_bg(&tft, 270, 142, text, &noto_sans_mono_bold_26, ILI9481_YELLOW, ILI9481_BLACK);
  else
    tft_font_write_bg(&tft, 270, 142, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  tft_font_write_bg(&tft, 8, 142, "LIMITS ON", &noto_sans_mono_bold_26, ILI9481_FOREST, ILI9481_BLACK);
}

static void els_threading_display_header(void) {
  tft_rgb_t color = els_threading.locked ? ILI9481_RED : ILI9481_FOREST;

  tft_filled_rectangle(&tft, 0, 0, 480, 50, color);
  tft_font_write_bg(&tft, 8, 0, "MANUAL THREADING", &noto_sans_mono_bold_26, ILI9481_WHITE, color);
  els_threading_display_encoder_pips();
}

static void els_threading_display_encoder_pips(void) {
  // multiplier is 100, 10 or 1
  size_t pips = els_threading.encoder_multiplier;
  pips = pips > 10 ? 3 : pips > 1 ? 2 : 1;

  for (size_t n = 0, spacing = 0; n < 3; n++, spacing++)
    tft_filled_rectangle(&tft,
      440, 32 - (n * 10 + spacing * 2),
      15 + n * 10, 10,
      (n < pips ? ILI9481_WHITE : els_threading.locked ? ILI9481_LITEGRAY : ILI9481_BGCOLOR2));

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
      case ELS_THREADING_ZLIM_MIN:
        tft_font_write_bg(&tft, x, y, "Z MIN", &noto_sans_mono_bold_26, ILI9481_RED, ILI9481_BLACK);
        break;
      case ELS_THREADING_ZLIM_MAX:
        tft_font_write_bg(&tft, x, y, "Z MAX", &noto_sans_mono_bold_26, ILI9481_RED, ILI9481_BLACK);
        break;
      case ELS_THREADING_SET_MIN:
        tft_font_write_bg(&tft, x, y, "SET MIN", &noto_sans_mono_bold_26, ILI9481_FOREST, ILI9481_BLACK);
        break;
      case ELS_THREADING_SET_MAX:
        tft_font_write_bg(&tft, x, y, "SET MAX", &noto_sans_mono_bold_26, ILI9481_FOREST, ILI9481_BLACK);
        break;
      default:
        if (els_threading.state & ELS_THREADING_ZJOG)
          tft_font_write_bg(&tft, x, y, "Z JOG", &noto_sans_mono_bold_26, ILI9481_GREEN, ILI9481_BLACK);
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

  els_sprint_double3(text, sizeof(text), els_threading.zpos, "Z");
  tft_font_write_bg(&tft, 8, 102, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), (els_dro.zpos_um / 1000.0), "Z");
  tft_font_write_bg(&tft, 8, 228, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);

  els_sprint_double33(text, sizeof(text), (els_dro.xpos_um / 1000.0), "X");
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
      break;
    case ELS_KEY_SET_ZX:
      if (els_threading.state & (ELS_THREADING_IDLE | ELS_THREADING_PAUSED)) {
        els_threading.state = ELS_THREADING_SET_ZAXES;
        els_encoder_set_rotation_debounce(25e3);
        els_threading.encoder_pos = 0;
        els_threading.stepper_pulse_curr = 0;
        els_threading.stepper_pulse_pos  = 0;
        els_encoder_reset();
      }
      break;
    case ELS_KEY_FUN_F2:
      els_encoder_reset();
      els_threading.state = ELS_THREADING_SET_MIN;
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
    case ELS_S_DIRECTION_CW:
      if (els_threading.zjog_steps == 0) {
        if (els_threading.zdir != -zdir) {
          ELS_THREADING_SET_ZDIR_LR;
          if (els_threading.zdir != 0)
            ELS_THREADING_Z_BACKLASH_FIX;
        }
        els_threading.zdir = -zdir;
      }
      if (els_threading.state == ELS_THREADING_PAUSED && els_spindle_get_counter() == 0) {
        els_threading.pitch_curr = 0;
        els_threading.state = ELS_THREADING_ACTIVE;
      }
      break;
    case ELS_S_DIRECTION_CCW:
      if (els_threading.zjog_steps == 0) {
        if (els_threading.zdir != zdir) {
          ELS_THREADING_SET_ZDIR_RL;
          if (els_threading.zdir != 0)
            ELS_THREADING_Z_BACKLASH_FIX;
        }
        els_threading.zdir = zdir;
      }
      if (els_threading.state == ELS_THREADING_PAUSED && els_spindle_get_counter() == 0) {
        els_threading.pitch_curr = 0;
        els_threading.state = ELS_THREADING_ACTIVE;
      }
      break;
    default:
      els_threading.zdir = 0;
      if (els_threading.state == ELS_THREADING_ACTIVE)
        els_threading.state = ELS_THREADING_PAUSED;
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 2: pitch settings.
// ----------------------------------------------------------------------------------
//
static void els_threading_recalulate_pitch_ratio(void) {
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
      break;
    case ELS_KEY_SET_FEED:
      els_threading.pitch_type = (els_threading.pitch_type + 1) % 2;
      els_threading_display_pitch_type();
      break;
    case ELS_KEY_REV_FEED:
      els_threading.pitch_reverse = !els_threading.pitch_reverse;
      els_threading_display_pitch();
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
            els_threading.pitch_um += 10;
          else
            els_threading.pitch_um = els_threading.pitch_um >= 110 ? els_threading.pitch_um - 10 : 100;
        }

        els_threading.encoder_pos = encoder_curr;
        els_threading_display_pitch();
        els_threading_recalulate_pitch_ratio();
      }
      break;
  }
}

// ----------------------------------------------------------------------------------
// Function 3: Z Axis - limits & jogging
// ----------------------------------------------------------------------------------
static void els_threading_set_zaxes(void) {
  els_threading_zjog_sync();
  els_threading_zjog_pulse();

  switch(els_keypad_read()) {
    case ELS_KEY_EXIT:
      els_threading.state = (els_threading.state & ELS_THREADING_ZJOG) ? ELS_THREADING_SET_ZAXES : ELS_THREADING_IDLE;
      els_threading.stepper_pulse_pos = els_threading.stepper_pulse_curr = 0;
      els_threading.zjog_steps = 0;
      break;
    case ELS_KEY_SET_ZX_MIN:
      if (els_threading.state == ELS_THREADING_SET_ZAXES) {
        els_threading.zmin = els_threading.zpos;
        els_threading_display_axes();
      }
      break;
    case ELS_KEY_SET_ZX_MAX:
      if (els_threading.state == ELS_THREADING_SET_ZAXES) {
        els_threading.zmax = els_threading.zpos;
        els_threading_display_axes();
      }
      break;
    case ELS_KEY_SET_ZX_ORI:
      if (els_threading.state == ELS_THREADING_SET_ZAXES) {
        els_dro_zero_z();
        els_threading.zpos = 0;
        els_threading_display_axes();
      }
      break;
    case ELS_KEY_JOG_ZX_MIN:
      els_threading_zjog_min();
      break;
    case ELS_KEY_JOG_ZX_MAX:
      els_threading_zjog_max();
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


static void els_threading_set_min(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_threading.state = ELS_THREADING_IDLE;
      els_threading_display_axes();
      break;
    case ELS_KEY_FUN_F2:
      els_threading.state = ELS_THREADING_SET_MAX;
      els_threading_display_axes();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_threading.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_threading.encoder_pos);
        els_threading.zmin += (delta * 0.01 * els_threading.encoder_multiplier);
        els_threading.encoder_pos = encoder_curr;
        els_threading_display_axes();
      }
      break;
  }
}

static void els_threading_set_max(void) {
  int32_t encoder_curr;
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
    case ELS_KEY_EXIT:
      els_threading.state = ELS_THREADING_IDLE;
      els_threading_display_axes();
      break;
    case ELS_KEY_FUN_F2:
      els_threading.state = ELS_THREADING_SET_MIN;
      els_threading_display_axes();
      break;
    default:
      encoder_curr = els_encoder_read();
      if (els_threading.encoder_pos != encoder_curr) {
        int32_t delta = (encoder_curr - els_threading.encoder_pos);
        els_threading.zmax += (delta * 0.01 * els_threading.encoder_multiplier);
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
  double delta;
  int32_t encoder_curr;

  encoder_curr = els_encoder_read();
  if (els_threading.encoder_pos != encoder_curr) {
    // ----------------------------------------------------------------------------------
    // Jog pulse calculation
    // ----------------------------------------------------------------------------------
    delta = (encoder_curr - els_threading.encoder_pos) * 0.01 * els_threading.encoder_multiplier;
    els_threading.stepper_pulse_curr += (delta * els_config->z_pulses_per_mm);
    els_threading.encoder_pos = encoder_curr;
    els_threading_display_axes();
  }
}

static void els_threading_zjog_sync(void) {
  if ((els_threading.state & ELS_THREADING_ZJOG) && els_threading.zjog_steps == 0) {
    els_threading.state &= ~ELS_THREADING_ZJOG;

    if ((els_threading.state & (ELS_THREADING_ZLIM_MIN | ELS_THREADING_ZLIM_MAX)) &&
        ((els_threading.zpos >= els_threading.zmin || (els_threading.zmin - els_threading.zpos) <= 1e-3) &&
         (els_threading.zpos <= els_threading.zmax || (els_threading.zpos - els_threading.zmax) <= 1e-3))) {
      els_threading.state = ELS_THREADING_IDLE;
    }
  }
}

static void els_threading_zjog_pulse(void) {
  bool backlash;
  int steps, dir = 0;

  if (els_threading.stepper_pulse_pos != els_threading.stepper_pulse_curr) {
    if (els_threading.stepper_pulse_curr < els_threading.stepper_pulse_pos)
      dir = -1;
    else
      dir = 1;

    if (els_threading.zdir != dir && els_threading.zjog_steps > 0)
      return;

    if (dir > 0)
      ELS_THREADING_SET_ZDIR_LR;
    else
      ELS_THREADING_SET_ZDIR_RL;

    backlash = (els_threading.zdir != dir);
    els_threading.zdir = dir;
    if (backlash) {
      ELS_THREADING_Z_BACKLASH_FIX;
    }

    steps = abs(els_threading.stepper_pulse_curr - els_threading.stepper_pulse_pos);
    els_threading.stepper_pulse_pos = els_threading.stepper_pulse_curr;
    els_threading.zjog_steps += steps;
    els_threading.state |= ELS_THREADING_ZJOG;
  }
}

// ----------------------------------------------------------------------------------
// Auto Jog to Min / Max
// ----------------------------------------------------------------------------------
static void els_threading_zjog_auto(double travel) {
  if (travel > 0) {
    ELS_THREADING_SET_ZDIR_LR;
    if (els_threading.zdir != 1)
      ELS_THREADING_Z_BACKLASH_FIX;
    els_threading.zdir = 1;
    els_threading.zjog_steps = (int32_t)round(travel * els_config->z_pulses_per_mm);
    els_threading.state |= ELS_THREADING_ZJOG;
  }
  else if (travel < 0) {
    ELS_THREADING_SET_ZDIR_RL;
    if (els_threading.zdir != -1)
      ELS_THREADING_Z_BACKLASH_FIX;
    els_threading.zdir = -1;
    els_threading.zjog_steps = (int32_t)round(-travel * els_config->z_pulses_per_mm);
    els_threading.state |= ELS_THREADING_ZJOG;
  }
}

static void els_threading_zjog_min(void) {
  double travel = els_threading.zmin - els_threading.zpos;
  els_printf("jog: to min, travel: %.2fmm\n", travel);
  els_threading_zjog_auto(travel);
}

static void els_threading_zjog_max(void) {
  double travel = els_threading.zmax - els_threading.zpos;

  els_printf("jog: to max, travel: %.2fmm\n", travel);
  els_threading_zjog_auto(travel);
}

// ----------------------------------------------------------------------------------
// GPIO: setup pins.
// ----------------------------------------------------------------------------------
static void els_threading_configure_gpio(void) {
  els_gpio_mode_output(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  els_gpio_mode_output(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN);
  els_gpio_mode_output(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);

  els_threading_disable_z();

  gpio_mode_setup(ELS_S_ENCODER2_PORTA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, ELS_S_ENCODER2_PINA);
  gpio_mode_setup(ELS_S_ENCODER2_PORTB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, ELS_S_ENCODER2_PINB);

  exti_select_source(ELS_S_ENCODER2_EXTI, ELS_S_ENCODER2_PORTA);
  exti_set_trigger(ELS_S_ENCODER2_EXTI, EXTI_TRIGGER_RISING);
  exti_enable_request(ELS_S_ENCODER2_EXTI);
}

static void els_threading_enable_z(void) {
  // active low.
  #if ELS_Z_ENA_ACTIVE_LOW
    els_gpio_clear(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  #else
    els_gpio_set(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  #endif
}

static void els_threading_disable_z(void) {
  // active low.
  #if ELS_Z_ENA_ACTIVE_LOW
    els_gpio_set(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  #else
    els_gpio_clear(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  #endif
}

// ----------------------------------------------------------------------------------
// Timer functions.
// ----------------------------------------------------------------------------------
//
// Clock setup.
static void els_threading_configure_timer(void) {
  rcc_periph_clock_enable(ELS_THREADING_TIMER_RCC);
  rcc_periph_reset_pulse(ELS_THREADING_TIMER_RST);

  // clock division 0, alignment edge, count up.
  timer_set_mode(ELS_THREADING_TIMER, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  // 10us counter, ~100KHz
  timer_set_prescaler(ELS_THREADING_TIMER, ((rcc_apb1_frequency * 2) / 100e3) - 1);

  // disable preload
  timer_disable_preload(ELS_THREADING_TIMER);
  timer_continuous_mode(ELS_THREADING_TIMER);

  nvic_set_priority(ELS_THREADING_TIMER_IRQ, 4);
  nvic_enable_irq(ELS_THREADING_TIMER_IRQ);
  timer_enable_update_event(ELS_THREADING_TIMER);
}

static void els_threading_timer_start(void) {
  timer_enable_counter(ELS_THREADING_TIMER);
  timer_enable_irq(ELS_THREADING_TIMER, TIM_DIER_UIE);
}

static void els_threading_timer_stop(void) {
  timer_disable_irq(ELS_THREADING_TIMER, TIM_DIER_UIE);
  timer_disable_counter(ELS_THREADING_TIMER);
}

bool els_threading_busy(void) {
  return els_threading.state != ELS_THREADING_IDLE;
}

// Update timer frequency for required feed rate.
static void els_threading_timer_update(int32_t feed_um) {
  uint32_t res;

  // Z-axis movement per pulse in mm.
  els_threading.zdelta = (1.0 / (double)els_config->z_pulses_per_mm);

  // Figure out how many pulses per second is required for the feed speed.
  // Ignore rounding errors, we don't need to be super accurate here.
  //
  res = (feed_um * els_config->z_pulses_per_mm) / 1000;
  // We need to clock at twice the rate to toggle the gpio.
  res = res * 2;

  // Clock's at 100Khz, figure out the period
  // TODO: check if period is 0 - 65535
  res = 100e3 / res;

  els_threading_timer_stop();
  timer_set_period(ELS_THREADING_TIMER, res);
}

