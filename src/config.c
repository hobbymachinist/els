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
static struct {
  bool locked;
  int32_t encoder_curr;
  size_t setting;
  bool edit;
  uint32_t value;
} config;

const char *settings[] = {
  "X PULSES PER MM",
  "Z PULSES PER MM",
  "X BACKLASH uM",
  "Z BACKLASH uM",
  "SPINDLE ENCODER PPR"
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

  printf("x pulses per mm = %lu\n", _els_config.x_pulses_per_mm);
  printf("z pulses per mm = %lu\n", _els_config.z_pulses_per_mm);
  printf("x backlash um = %lu\n", _els_config.x_backlash_um);
  printf("z backlash um = %lu\n", _els_config.z_backlash_um);
  printf("spindle encoder ppr = %lu\n", _els_config.spindle_encoder_ppr);
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
      config.value++;
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
  els_encoder_set_rotation_debounce(10e3);
  els_encoder_set_direction_debounce(100e3);
}

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
static uint32_t els_config_get(size_t setting) {
  switch (setting) {
    case 0:
      return _els_config.x_pulses_per_mm;
    case 1:
      return _els_config.z_pulses_per_mm;
    case 2:
      return _els_config.x_backlash_um;
    case 3:
      return _els_config.z_backlash_um;
    case 4:
      return _els_config.spindle_encoder_ppr;
    default:
      return 0;
  }
}

static void els_config_set(size_t setting, uint32_t value) {
  switch (setting) {
    case 0:
      _els_config.x_pulses_per_mm = value;
      els_kv_write(ELS_KV_X_PULSES_PER_MM, &value, sizeof(value));
      break;
    case 1:
      _els_config.z_pulses_per_mm = value;
      els_kv_write(ELS_KV_Z_PULSES_PER_MM, &value, sizeof(value));
      break;
    case 2:
      _els_config.x_backlash_um = value;
      els_kv_write(ELS_KV_X_BACKLASH_UM, &value, sizeof(value));
      break;
    case 3:
      _els_config.z_backlash_um = value;
      els_kv_write(ELS_KV_Z_BACKLASH_UM, &value, sizeof(value));
      break;
    case 4:
      _els_config.spindle_encoder_ppr = value;
      els_kv_write(ELS_KV_SPINDLE_ENCODER_PPR, &value, sizeof(value));
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

  for (size_t n = config.setting; n < ELS_CONFIG_SETTING_MAX; n++) {
    y = y_start + row * height;
    snprintf(row_text, sizeof(row_text), "%d", n + 1);
    snprintf(value_text, sizeof(value_text), "%lu", config.edit && row == 0 ? config.value : els_config_get(n));

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

  for (size_t n = row; n < ELS_CONFIG_SETTING_MAX; n++)
    tft_filled_rectangle(&tft, 0, y_start + n * height, 480, height, ILI9481_BLACK);
}
