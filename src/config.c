#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#include "config.h"
#include "encoder.h"
#include "function.h"
#include "keypad.h"
#include "kv.h"
#include "stepper.h"

#include "constants.h"

#include "tft/tft.h"
#include "tft/fonts/fonts.h"

//==============================================================================
// Externs
//==============================================================================
extern tft_device_t tft;

extern const tft_font_t noto_sans_mono_bold_26;
extern const tft_font_t noto_sans_mono_bold_20;
extern const tft_font_t gears_regular_32;
extern const tft_font_t gears_regular_50;

//==============================================================================
// State
//==============================================================================
static els_config_t _els_config;
const els_config_t *els_config = &_els_config;

//==============================================================================
// Internal State
//==============================================================================
typedef enum {
  ELS_CONFIG_X_PULSES_PER_MM      = 0,
  ELS_CONFIG_Z_PULSES_PER_MM      = 1,
  ELS_CONFIG_X_BACKLASH_UM        = 2,
  ELS_CONFIG_Z_BACKLASH_UM        = 3,
  ELS_CONFIG_SPINDLE_ENCODER_PPR  = 4,
  ELS_CONFIG_X_RETRACT_JOG_MM_S   = 5,
  ELS_CONFIG_Z_RETRACT_JOG_MM_S   = 6,
  ELS_CONFIG_X_CLOSED_LOOP        = 7,
  ELS_CONFIG_Z_CLOSED_LOOP        = 8,
  ELS_CONFIG_X_DRO_INVERT         = 9,
  ELS_CONFIG_Z_DRO_INVERT         = 10,
} els_config_setting_t;

static struct {
  bool locked;
  int32_t encoder_curr;
  els_config_setting_t setting;
  bool edit;
  uint32_t value;
} config;

const char *settings[] = {
  "X PULSES PER MM",
  "Z PULSES PER MM",
  "X BACKLASH uM",
  "Z BACKLASH uM",
  "SPINDLE ENCODER PPR",
  "X RETRACT JOG MM/S",
  "Z RETRACT JOG MM/S",
  "X CLOSED LOOP",
  "Z CLOSED LOOP",
  "X DRO INVERT",
  "Z DRO INVERT"
};

#define ELS_CONFIG_SETTING_MAX (sizeof(settings) / sizeof(settings[0]))

//==============================================================================
// Helpers
//==============================================================================

static void     els_config_menu_display_init(void);
static void     els_config_menu_display_header(void);
static void     els_config_menu_display_refresh(void);
static uint32_t els_config_get(size_t n);
static void     els_config_set(size_t n, uint32_t value);

//==============================================================================
// API
//==============================================================================
void els_config_setup(void) {
  memset(&_els_config, 0, sizeof(_els_config));

  els_kv_read(ELS_KV_X_PULSES_PER_MM, &_els_config.x_pulses_per_mm, sizeof(_els_config.x_pulses_per_mm));
  if (_els_config.x_pulses_per_mm == 0) {
    _els_config.x_pulses_per_mm = ELS_X_PULSES_PER_MM;
    printf("set default x pulses per mm = %lu\n", _els_config.x_pulses_per_mm);
    els_kv_write(ELS_KV_X_PULSES_PER_MM, &_els_config.x_pulses_per_mm, sizeof(_els_config.x_pulses_per_mm));
  }

  els_kv_read(ELS_KV_Z_PULSES_PER_MM, &_els_config.z_pulses_per_mm, sizeof(_els_config.z_pulses_per_mm));
  if (_els_config.z_pulses_per_mm == 0) {
    _els_config.z_pulses_per_mm = ELS_Z_PULSES_PER_MM;
    printf("set default z pulses per mm = %lu\n", _els_config.z_pulses_per_mm);
    els_kv_write(ELS_KV_Z_PULSES_PER_MM, &_els_config.z_pulses_per_mm, sizeof(_els_config.z_pulses_per_mm));
  }

  els_kv_read(ELS_KV_X_BACKLASH_UM, &_els_config.x_backlash_um, sizeof(_els_config.x_backlash_um));
  if (_els_config.x_backlash_um == 0) {
    _els_config.x_backlash_um = ELS_X_BACKLASH_UM;
    printf("set default x backlash um = %lu\n", _els_config.x_backlash_um);
    els_kv_write(ELS_KV_X_BACKLASH_UM, &_els_config.x_backlash_um, sizeof(_els_config.x_backlash_um));
  }
  _els_config.x_backlash_pulses = (_els_config.x_backlash_um * _els_config.x_pulses_per_mm) / 1000;

  els_kv_read(ELS_KV_Z_BACKLASH_UM, &_els_config.z_backlash_um, sizeof(_els_config.z_backlash_um));
  if (_els_config.z_backlash_um == 0) {
    _els_config.z_backlash_um = ELS_Z_BACKLASH_UM;
    printf("set default z backlash um = %lu\n", _els_config.z_backlash_um);
    els_kv_write(ELS_KV_Z_BACKLASH_UM, &_els_config.z_backlash_um, sizeof(_els_config.z_backlash_um));
  }
  _els_config.z_backlash_pulses = (_els_config.z_backlash_um * _els_config.z_pulses_per_mm) / 1000;

  els_kv_read(ELS_KV_SPINDLE_ENCODER_PPR, &_els_config.spindle_encoder_ppr, sizeof(_els_config.spindle_encoder_ppr));
  if (_els_config.spindle_encoder_ppr == 0) {
    _els_config.spindle_encoder_ppr = ELS_S_ENCODER_PPR;
    printf("set default spindle encoder ppr = %lu\n", _els_config.spindle_encoder_ppr);
    els_kv_write(ELS_KV_SPINDLE_ENCODER_PPR, &_els_config.spindle_encoder_ppr, sizeof(_els_config.spindle_encoder_ppr));
  }

  els_kv_read(ELS_KV_X_RETRACT_JOG_MM_S, &_els_config.x_retract_jog_mm_s, sizeof(_els_config.x_retract_jog_mm_s));
  if (_els_config.x_retract_jog_mm_s == 0) {
    _els_config.x_retract_jog_mm_s = 1;
    printf("set default x min jog mm/s = %lu\n", _els_config.x_retract_jog_mm_s);
    els_kv_write(ELS_KV_X_RETRACT_JOG_MM_S, &_els_config.x_retract_jog_mm_s, sizeof(_els_config.x_retract_jog_mm_s));
  }

  els_kv_read(ELS_KV_Z_RETRACT_JOG_MM_S, &_els_config.z_retract_jog_mm_s, sizeof(_els_config.z_retract_jog_mm_s));
  if (_els_config.z_retract_jog_mm_s == 0) {
    _els_config.z_retract_jog_mm_s = 2;
    printf("set default z min jog mm/s = %lu\n", _els_config.z_retract_jog_mm_s);
    els_kv_write(ELS_KV_Z_RETRACT_JOG_MM_S, &_els_config.z_retract_jog_mm_s, sizeof(_els_config.z_retract_jog_mm_s));
  }

  els_kv_read(ELS_KV_X_CLOSED_LOOP, &_els_config.x_closed_loop, sizeof(_els_config.x_closed_loop));
  els_kv_read(ELS_KV_Z_CLOSED_LOOP, &_els_config.z_closed_loop, sizeof(_els_config.z_closed_loop));

  els_kv_read(ELS_KV_X_DRO_INVERT, &_els_config.x_dro_invert, sizeof(_els_config.x_dro_invert));
  els_kv_read(ELS_KV_Z_DRO_INVERT, &_els_config.z_dro_invert, sizeof(_els_config.z_dro_invert));

  printf("x pulses per mm = %lu\n", _els_config.x_pulses_per_mm);
  printf("z pulses per mm = %lu\n", _els_config.z_pulses_per_mm);
  printf("x backlash um = %lu\n", _els_config.x_backlash_um);
  printf("z backlash um = %lu\n", _els_config.z_backlash_um);
  printf("spindle encoder ppr = %lu\n", _els_config.spindle_encoder_ppr);
  printf("x min jog mm/s = %lu\n", _els_config.x_retract_jog_mm_s);
  printf("z min jog mm/s = %lu\n", _els_config.z_retract_jog_mm_s);
  printf("x closed loop = %d\n", _els_config.x_closed_loop);
  printf("z closed loop = %d\n", _els_config.z_closed_loop);
  printf("x dro invert = %d\n", _els_config.x_dro_invert);
  printf("z dro invert = %d\n", _els_config.z_dro_invert);
}

void els_config_start(void) {
  config.encoder_curr = 0;
  els_encoder_reset();
  els_encoder_set_direction_debounce(100e3);
  els_encoder_set_rotation_debounce(50e3);
  els_config_menu_display_init();
  els_config_menu_display_header();
  els_config_menu_display_refresh();
}

void els_config_update(void) {
  int32_t enc;
  bool kp_locked = els_keypad_locked();

  if (kp_locked != config.locked) {
    config.locked = kp_locked;
    els_config_menu_display_header();
  }

  //----------------------------------------------------------------------------------------------
  // function scroll
  //----------------------------------------------------------------------------------------------
  enc = els_encoder_read();
  if (config.edit) {
    if (enc < config.encoder_curr && config.value > 0) {
      config.value--;
      els_config_menu_display_refresh();
    }
    else if (enc > config.encoder_curr) {
      switch (config.setting) {
        case ELS_CONFIG_X_CLOSED_LOOP:
        case ELS_CONFIG_Z_CLOSED_LOOP:
        case ELS_CONFIG_X_DRO_INVERT:
        case ELS_CONFIG_Z_DRO_INVERT:
          config.value = 1;
          break;
        default:
          config.value++;
          break;
      }
      els_config_menu_display_refresh();
    }
  }
  else {
    if (enc < config.encoder_curr && config.setting > 0) {
      config.setting--;
      els_config_menu_display_refresh();
    }

    if (enc > config.encoder_curr && config.setting < ELS_CONFIG_SETTING_MAX - 1) {
      config.setting++;
      els_config_menu_display_refresh();
    }
  }

  config.encoder_curr = enc;

  switch(els_keypad_read()) {
    case ELS_KEY_OK:
      config.edit = !config.edit;
      if (config.edit)
        config.value = els_config_get(config.setting);
      else
        els_config_set(config.setting, config.value);
      els_config_menu_display_refresh();
      break;
    case ELS_KEY_EXIT:
      if (config.edit) {
        config.edit = false;
        els_config_menu_display_refresh();
      }
      else {
        // pulses per mm changes
        els_stepper_setup();
        els_function_restore();
      }
      break;
    default:
      break;
  }
}

void els_config_stop(void) {
  els_encoder_set_rotation_debounce(25e3);
  els_encoder_set_direction_debounce(100e3);
}

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
static uint32_t els_config_get(size_t setting) {
  switch (setting) {
    case ELS_CONFIG_X_PULSES_PER_MM:
      return _els_config.x_pulses_per_mm;
    case ELS_CONFIG_Z_PULSES_PER_MM:
      return _els_config.z_pulses_per_mm;
    case ELS_CONFIG_X_BACKLASH_UM:
      return _els_config.x_backlash_um;
    case ELS_CONFIG_Z_BACKLASH_UM:
      return _els_config.z_backlash_um;
    case ELS_CONFIG_SPINDLE_ENCODER_PPR:
      return _els_config.spindle_encoder_ppr;
    case ELS_CONFIG_X_RETRACT_JOG_MM_S:
      return _els_config.x_retract_jog_mm_s;
    case ELS_CONFIG_Z_RETRACT_JOG_MM_S:
      return _els_config.z_retract_jog_mm_s;
    case ELS_CONFIG_X_CLOSED_LOOP:
      return _els_config.x_closed_loop;
    case ELS_CONFIG_Z_CLOSED_LOOP:
      return _els_config.z_closed_loop;
    case ELS_CONFIG_X_DRO_INVERT:
      return _els_config.x_dro_invert;
    case ELS_CONFIG_Z_DRO_INVERT:
      return _els_config.z_dro_invert;
    default:
      return 0;
  }
}

static void els_config_set(size_t setting, uint32_t value) {
  switch (setting) {
    case ELS_CONFIG_X_PULSES_PER_MM:
      _els_config.x_pulses_per_mm = value;
      els_kv_write(ELS_KV_X_PULSES_PER_MM, &value, sizeof(value));
      break;
    case ELS_CONFIG_Z_PULSES_PER_MM:
      _els_config.z_pulses_per_mm = value;
      els_kv_write(ELS_KV_Z_PULSES_PER_MM, &value, sizeof(value));
      break;
    case ELS_CONFIG_X_BACKLASH_UM:
      _els_config.x_backlash_um = value;
      els_kv_write(ELS_KV_X_BACKLASH_UM, &value, sizeof(value));
      break;
    case ELS_CONFIG_Z_BACKLASH_UM:
      _els_config.z_backlash_um = value;
      els_kv_write(ELS_KV_Z_BACKLASH_UM, &value, sizeof(value));
      break;
    case ELS_CONFIG_SPINDLE_ENCODER_PPR:
      _els_config.spindle_encoder_ppr = value;
      els_kv_write(ELS_KV_SPINDLE_ENCODER_PPR, &value, sizeof(value));
      break;
    case ELS_CONFIG_X_RETRACT_JOG_MM_S:
      _els_config.x_retract_jog_mm_s = value;
      els_kv_write(ELS_KV_X_RETRACT_JOG_MM_S, &value, sizeof(value));
      break;
    case ELS_CONFIG_Z_RETRACT_JOG_MM_S:
      _els_config.z_retract_jog_mm_s = value;
      els_kv_write(ELS_KV_Z_RETRACT_JOG_MM_S, &value, sizeof(value));
      break;
    case ELS_CONFIG_X_CLOSED_LOOP:
      _els_config.x_closed_loop = value > 0 ? true : false;
      els_kv_write(ELS_KV_X_CLOSED_LOOP, &_els_config.x_closed_loop, sizeof(_els_config.x_closed_loop));
      break;
    case ELS_CONFIG_Z_CLOSED_LOOP:
      _els_config.z_closed_loop = value > 0 ? true : false;
      els_kv_write(ELS_KV_Z_CLOSED_LOOP, &_els_config.z_closed_loop, sizeof(_els_config.z_closed_loop));
      break;
    case ELS_CONFIG_X_DRO_INVERT:
      _els_config.x_dro_invert = value > 0 ? true : false;
      els_kv_write(ELS_KV_X_DRO_INVERT, &_els_config.x_dro_invert, sizeof(_els_config.x_dro_invert));
      break;
    case ELS_CONFIG_Z_DRO_INVERT:
      _els_config.z_dro_invert = value > 0 ? true : false;
      els_kv_write(ELS_KV_Z_DRO_INVERT, &_els_config.z_dro_invert, sizeof(_els_config.z_dro_invert));
      break;
    default:
      break;
  }
}

//------------------------------------------------------------------------------
// Display
//------------------------------------------------------------------------------
static void els_config_menu_display_init(void) {
  tft_filled_rectangle(&tft, 0, 0, 480, 320, ILI9481_BLACK);
  els_config_menu_display_header();
  els_config_menu_display_refresh();
}

static void els_config_menu_display_header(void) {
  if (config.locked) {
    tft_filled_rectangle(&tft, 0, 0, 480, 50, ILI9481_RED);
    tft_font_write_bg(&tft, 8, 0, "SETTINGS", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_RED);
    tft_font_write_bg(&tft, 446, 6, "C", &gears_regular_32, ILI9481_WHITE, ILI9481_RED);
  }
  else {
    tft_filled_rectangle(&tft, 0,   0, 480,  50, ILI9481_DIANNE);
    tft_filled_rectangle(&tft, 0,  50, 480,   1, ILI9481_LITEGRAY);

    tft_font_write_bg(&tft, 8, 0, "SETTINGS", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_DIANNE);
    tft_font_write_bg(&tft, 446, 6, "D", &gears_regular_32, ILI9481_WHITE, ILI9481_DIANNE);
  }
}

static void els_config_menu_display_refresh(void) {
  char row_text[16], value_text[16];
  size_t row = 0, y_start = 52, y, id;
  size_t height = 40;

  for (size_t n = config.setting; n < ELS_CONFIG_SETTING_MAX && row < 6; n++) {
    y = y_start + row * height;
    snprintf(row_text, sizeof(row_text), "%d", n + 1);

    switch (n) {
      case ELS_CONFIG_X_CLOSED_LOOP:
      case ELS_CONFIG_Z_CLOSED_LOOP:
      case ELS_CONFIG_X_DRO_INVERT:
      case ELS_CONFIG_Z_DRO_INVERT:
        snprintf(value_text, sizeof(value_text), "%s",
          (config.edit && row == 0 ? config.value : els_config_get(n)) ? "ON" : "OFF");
        break;
      default:
        snprintf(value_text, sizeof(value_text), "%lu", config.edit && row == 0 ? config.value : els_config_get(n));
        break;
    }

    if (row == 0) {
      tft_rgb_t bg_color = config.edit ? ILI9481_YELLOW : ILI9481_WHITE;
      tft_filled_rectangle(&tft, 0, y, 480, height, bg_color);
      tft_font_write_bg(&tft, 8, y, row_text, &noto_sans_mono_bold_20, ILI9481_BLACK, bg_color);
      tft_font_write_bg_xadv(&tft, 50, y, settings[n], &noto_sans_mono_bold_20, ILI9481_BLACK, bg_color, 14);
      tft_font_write_bg(&tft, 380, y, value_text, &noto_sans_mono_bold_20, ILI9481_BLACK, bg_color);
    }
    else {
      tft_filled_rectangle(&tft, 0, y, 480, height, ILI9481_BLACK);
      tft_font_write_bg(&tft, 8, y, row_text, &noto_sans_mono_bold_20, ILI9481_WHITE, ILI9481_BLACK);
      tft_font_write_bg_xadv(&tft, 50, y, settings[n], &noto_sans_mono_bold_20, ILI9481_WHITE, ILI9481_BLACK, 14);
      tft_font_write_bg(&tft, 380, y, value_text, &noto_sans_mono_bold_20, ILI9481_WHITE, ILI9481_BLACK);
    }

    row++;
  }

  for (size_t n = row; n < 6; n++)
    tft_filled_rectangle(&tft, 0, y_start + n * height, 480, height, ILI9481_BLACK);
}
