#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

// periph
#include "keypad.h"
#include "encoder.h"

// operations
#include "turning.h"
#include "threading.h"
#include "turn_dimension.h"
#include "bore_dimension.h"
#include "bore_hole.h"
#include "threading_ext.h"
#include "threading_int.h"
#include "taper_ext_r.h"
#include "taper_ext_l.h"
#include "taper_int.h"
#include "convex_ext_r.h"
#include "convex_ext_l.h"

#include "tft/tft.h"
#include "tft/fonts/fonts.h"

#include "function.h"

#include "config.h"
#include "utils.h"

#include "constants.h"

typedef enum {
  ELS_FUNCTION_MENU         = 0,
  ELS_FUNCTION_CONFIG       = 1,
  // ---------------------------------------------
  // Functions accessible through menu.
  // ---------------------------------------------
  ELS_FUNCTION_TURNING      = 2,
  ELS_FUNCTION_THREADING    = 3,

  // turn stock to a specific length and diameter.
  ELS_FUNCTION_TURN_DIM     = 4,

  // bore stock to a specific length and diameter.
  ELS_FUNCTION_BORE_DIM_1   = 5,
  ELS_FUNCTION_BORE_DIM_2   = 6,

  // external thread given length and pitch.
  ELS_FUNCTION_THREAD_EXT   = 7,
  // internal thread given length and pitch.
  ELS_FUNCTION_THREAD_INT   = 8,

  // turn stock to an external taper d & l
  ELS_FUNCTION_TAPER_EXT_1  = 9,
  ELS_FUNCTION_TAPER_EXT_2  = 10,

  // turn stock to an internal taper d & l
  ELS_FUNCTION_TAPER_INT    = 11,

  // turn external convex radius right (cw) and left (ccw) approach.
  ELS_FUNCTION_CONVEX_EXT_1 = 12,
  ELS_FUNCTION_CONVEX_EXT_2 = 13,

  // turn external concave radius right (cw) and left (ccw) approach.
  ELS_FUNCTION_CONKAV_EXT_1 = 14,
  ELS_FUNCTION_CONKAV_EXT_2 = 15,

  // rounded groove
  ELS_FUNCTION_GROOVE_RND   = 16,
  // straight groove
  ELS_FUNCTION_GROOVE_STD   = 17,

  // max
  ELS_FUNCTION_MAX          = 18,
} els_function_type_t;

static char *function_type_labels[] = {
  "FUNCTION MENU",
  "SETTINGS",
  "MANUAL TURNING",
  "MANUAL THREADING",
  "TURNING",
  "BORING - POCKET",
  "BORING - HOLE",
  "THREAD EXTERNAL",
  "THREAD INTERNAL",
  "TAPER EXTERNAL - R",
  "TAPER EXTERNAL - L",
  "TAPER INTERNAL",
  "CONVEX EXTERNAL - R",
  "CONVEX EXTERNAL - L",
  "CONCAVE EXTERNAL - R",
  "CONCAVE EXTERNAL - L",
  "GROOVE ROUNDED",
  "GROOVE STANDARD"
};

//==============================================================================
// Externs
//==============================================================================
extern tft_device_t tft;

extern const tft_font_t noto_sans_mono_bold_26;
extern const tft_font_t gears_regular_32;
extern const tft_font_t gears_regular_50;

//==============================================================================
// Internal State
//==============================================================================
static struct {
  els_function_type_t function_sel;
  els_function_type_t function_curr;

  bool locked;
  int32_t encoder_curr;

  bool refresh_todo;
} els_function;

//==============================================================================
// Internal functions
//==============================================================================
static void els_function_start(els_function_type_t);
static void els_function_stop(void);
static void els_function_change(els_function_type_t);

static void els_function_menu_start(void);
static void els_function_menu_update(void);
static void els_function_menu_stop(void);

static void els_function_menu_display_init(void);
static void els_function_menu_display_header(void);
static void els_function_menu_display_refresh(void);

static void els_function_not_implemented_start(void);
static void els_function_not_implemented_update(void);
static void els_function_not_implemented_stop(void);
//------------------------------------------------------------------------------

//==============================================================================
// Operations Registry
//==============================================================================
static const struct {
  void (*setup_cb)(void);
  void (*start_cb)(void);
  void (*update_cb)(void);
  void (*stop_cb)(void);
  bool (*busy_cb)(void);
} els_function_registry[] = {
  {
    .setup_cb = NULL,
    .start_cb = els_function_menu_start,
    .update_cb = els_function_menu_update,
    .stop_cb = els_function_menu_stop,
    .busy_cb = NULL
  },
  {
    .setup_cb = NULL,
    .start_cb = els_config_start,
    .update_cb = els_config_update,
    .stop_cb = els_config_stop,
    .busy_cb = NULL
  },
  {
    .setup_cb = els_turning_setup,
    .start_cb = els_turning_start,
    .update_cb = els_turning_update,
    .stop_cb = els_turning_stop,
    .busy_cb = els_turning_busy
  },
  {
    .setup_cb = els_threading_setup,
    .start_cb = els_threading_start,
    .update_cb = els_threading_update,
    .stop_cb = els_threading_stop,
    .busy_cb = els_threading_busy
  },
  {
    .setup_cb = els_turn_dimension_setup,
    .start_cb = els_turn_dimension_start,
    .update_cb = els_turn_dimension_update,
    .stop_cb = els_turn_dimension_stop,
    .busy_cb = els_turn_dimension_busy
  },
  {
    .setup_cb = els_bore_dimension_setup,
    .start_cb = els_bore_dimension_start,
    .update_cb = els_bore_dimension_update,
    .stop_cb = els_bore_dimension_stop,
    .busy_cb = els_bore_dimension_busy
  },
  {
    .setup_cb = els_bore_hole_setup,
    .start_cb = els_bore_hole_start,
    .update_cb = els_bore_hole_update,
    .stop_cb = els_bore_hole_stop,
    .busy_cb = els_bore_hole_busy
  },
  {
    .setup_cb = els_threading_ext_setup,
    .start_cb = els_threading_ext_start,
    .update_cb = els_threading_ext_update,
    .stop_cb = els_threading_ext_stop,
    .busy_cb = els_threading_ext_busy
  },
  {
    .setup_cb = els_threading_int_setup,
    .start_cb = els_threading_int_start,
    .update_cb = els_threading_int_update,
    .stop_cb = els_threading_int_stop,
    .busy_cb = els_threading_int_busy
  },
  {
    .setup_cb = els_taper_ext_r_setup,
    .start_cb = els_taper_ext_r_start,
    .update_cb = els_taper_ext_r_update,
    .stop_cb = els_taper_ext_r_stop,
    .busy_cb = els_taper_ext_r_busy
  },
  {
    .setup_cb = els_taper_ext_l_setup,
    .start_cb = els_taper_ext_l_start,
    .update_cb = els_taper_ext_l_update,
    .stop_cb = els_taper_ext_l_stop,
    .busy_cb = els_taper_ext_l_busy
  },
  {
    .setup_cb = els_taper_int_setup,
    .start_cb = els_taper_int_start,
    .update_cb = els_taper_int_update,
    .stop_cb = els_taper_int_stop,
    .busy_cb = els_taper_int_busy
  },
  {
    .setup_cb = els_convex_ext_r_setup,
    .start_cb = els_convex_ext_r_start,
    .update_cb = els_convex_ext_r_update,
    .stop_cb = els_convex_ext_r_stop,
    .busy_cb = els_convex_ext_r_busy
  },
  {
    .setup_cb = els_convex_ext_l_setup,
    .start_cb = els_convex_ext_l_start,
    .update_cb = els_convex_ext_l_update,
    .stop_cb = els_convex_ext_l_stop,
    .busy_cb = els_convex_ext_l_busy
  }
};

#define ELS_FUNCTION_REGISTRY_SIZE (sizeof(els_function_registry) / sizeof(els_function_registry[0]))

//------------------------------------------------------------------------------
void els_function_init(void) {
  // setup operations
  for(size_t i = 0; i < ELS_FUNCTION_REGISTRY_SIZE; i++) {
    if (els_function_registry[i].setup_cb)
      els_function_registry[i].setup_cb();
  }

  els_function.function_sel = ELS_FUNCTION_CONVEX_EXT_1;
  els_function_start(ELS_FUNCTION_CONVEX_EXT_1);
}

void els_function_update(void) {
  switch (els_keypad_peek()) {
    case ELS_KEY_FUN_TURN:
      els_function_change(ELS_FUNCTION_TURN_DIM);
      els_keypad_flush();
      break;
    case ELS_KEY_FUN_THREAD:
      els_function_change(ELS_FUNCTION_THREAD_EXT);
      els_keypad_flush();
      break;
    case ELS_KEY_SETTINGS:
      els_function_change(ELS_FUNCTION_CONFIG);
      els_keypad_flush();
      break;
    case ELS_KEY_FUN_SELECT:
      els_function_change(ELS_FUNCTION_MENU);
      els_keypad_flush();
      break;
    default:
      break;
  }

  if (els_function.function_curr < ELS_FUNCTION_REGISTRY_SIZE)
    els_function_registry[els_function.function_curr].update_cb();
  else
    els_function_not_implemented_update();
}

void els_function_restore(void) {
  els_function_change(els_function.function_sel);
}

//==============================================================================
// Internal functions
//==============================================================================
//
// Function Change
//------------------------------------------------------------------------------
static void els_function_stop(void) {
  if (els_function.function_curr < ELS_FUNCTION_REGISTRY_SIZE)
    els_function_registry[els_function.function_curr].stop_cb();
  else
    els_function_not_implemented_stop();
}

static void els_function_start(els_function_type_t f) {
  els_function.function_curr = f;
  if (els_function.function_curr < ELS_FUNCTION_REGISTRY_SIZE)
    els_function_registry[els_function.function_curr].start_cb();
  else
    els_function_not_implemented_start();
}

static void els_function_change(els_function_type_t f) {
  if (els_keypad_locked() || els_function.function_curr == f)
    return;

  if (els_function.function_curr < ELS_FUNCTION_REGISTRY_SIZE) {
    if (els_function_registry[els_function.function_curr].busy_cb &&
        els_function_registry[els_function.function_curr].busy_cb()) {
      printf("%s is busy, exit submenu or stop operation first.\n", function_type_labels[els_function.function_curr]);
      return;
    }
    els_function_registry[els_function.function_curr].stop_cb();
    els_function_start(f);
  }
  else {
    els_function_not_implemented_stop();
    els_function_start(f);
  }
}

//------------------------------------------------------------------------------
// Menu Module
//------------------------------------------------------------------------------
static void els_function_menu_start(void) {
  els_function.encoder_curr = 0;
  els_encoder_reset();
  els_encoder_set_direction_debounce(100e3);
  els_encoder_set_rotation_debounce(50e3);
  els_function_menu_display_init();
}

static void els_function_menu_stop(void) {
  els_encoder_set_rotation_debounce(10e3);
  els_encoder_set_direction_debounce(100e3);
}

static void els_function_menu_update(void) {
  int32_t enc;
  bool kp_locked = els_keypad_locked();

  if (kp_locked != els_function.locked) {
    els_function.locked = kp_locked;
    els_function_menu_display_header();
  }

  //----------------------------------------------------------------------------------------------
  // function scroll
  //----------------------------------------------------------------------------------------------
  enc = els_encoder_read();
  if (enc < els_function.encoder_curr && els_function.function_sel > ELS_FUNCTION_TURNING) {
    els_function.function_sel--;
    els_function_menu_display_refresh();
  }

  if (enc > els_function.encoder_curr && els_function.function_sel < ELS_FUNCTION_MAX - 1) {
    els_function.function_sel++;
    els_function_menu_display_refresh();
  }

  els_function.encoder_curr = enc;

  //----------------------------------------------------------------------------------------------
  // function selection
  //----------------------------------------------------------------------------------------------
  switch(els_keypad_read()) {
    case ELS_KEY_OK:
      els_printf("changing to function: %d\n", els_function.function_sel);
      els_function_change(els_function.function_sel);
      break;
    default:
      break;
  }
}

//------------------------------------------------------------------------------
// Menu Module Display
//------------------------------------------------------------------------------
static void els_function_menu_display_init(void) {
  tft_filled_rectangle(&tft, 0,   0, 480, 320, ILI9481_BLACK);
  els_function_menu_display_header();
  els_function_menu_display_refresh();
}

static void els_function_menu_display_header(void) {
  if (els_function.locked) {
    tft_filled_rectangle(&tft, 0, 0, 480, 50, ILI9481_RED);
    tft_font_write_bg(&tft, 8, 0, "FUNCTIONS", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_RED);
    tft_font_write_bg(&tft, 446, 6, "C", &gears_regular_32, ILI9481_WHITE, ILI9481_RED);
  }
  else {
    tft_filled_rectangle(&tft, 0,   0, 480,  50, ILI9481_DIANNE);
    tft_filled_rectangle(&tft, 0,  50, 480,   1, ILI9481_LITEGRAY);

    tft_font_write_bg(&tft, 8, 0, "FUNCTIONS", &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_DIANNE);
    tft_font_write_bg(&tft, 446, 6, "D", &gears_regular_32, ILI9481_WHITE, ILI9481_DIANNE);
  }
}

static void els_function_menu_display_refresh(void) {
  char text[16];
  size_t row = 0, y_start = 52, y, id;

  for (els_function_type_t f = els_function.function_sel; f < ELS_FUNCTION_MAX && f < els_function.function_sel + 5; f++) {
    y = y_start + row * 50;
    id = f - ELS_FUNCTION_CONFIG;
    snprintf(text, sizeof(text), "%d", id);

    if (row == 0) {
      tft_filled_rectangle(&tft, 0, y, 480, 50, ILI9481_WHITE);
      tft_font_write_bg(&tft, 8, y, text, &noto_sans_mono_bold_26, ILI9481_BLACK, ILI9481_WHITE);
      tft_font_write_bg_xadv(&tft, 64, y, function_type_labels[f], &noto_sans_mono_bold_26, ILI9481_BLACK, ILI9481_WHITE, 14);
    }
    else {
      tft_filled_rectangle(&tft, 0, y, 480, 50, ILI9481_BLACK);
      tft_font_write_bg(&tft, 8, y, text, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK);
      tft_font_write_bg_xadv(&tft, 64, y, function_type_labels[f], &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_BLACK, 14);
    }
    row++;
  }

  for (size_t n = row; n < 5; n++)
    tft_filled_rectangle(&tft, 0, y_start + n * 50, 480, 50, ILI9481_BLACK);
}

//------------------------------------------------------------------------------
// Default TODO Display
//------------------------------------------------------------------------------
static void els_function_not_implemented_start(void) {
  tft_filled_rectangle(&tft, 0, 0, 480, 320, ILI9481_BLACK);
  els_function.refresh_todo = true;
}

static void els_function_not_implemented_stop(void) {
  // NOP
}

static void els_function_not_implemented_update(void) {
  bool kp_locked = els_keypad_locked();

  if (kp_locked != els_function.locked) {
    els_function.locked = kp_locked;
    els_function.refresh_todo = true;
  }

  if (els_function.refresh_todo) {
    const char *label = function_type_labels[els_function.function_curr];
    if (els_function.locked) {
      tft_filled_rectangle(&tft, 0, 0, 480, 50, ILI9481_RED);
      tft_font_write_bg(&tft, 8, 0, label, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_RED);
      tft_font_write_bg(&tft, 446, 6, "C", &gears_regular_32, ILI9481_WHITE, ILI9481_RED);
    }
    else {
      tft_filled_rectangle(&tft, 0, 0, 480, 50, ILI9481_GRAY);
      tft_font_write_bg(&tft, 8, 0, label, &noto_sans_mono_bold_26, ILI9481_WHITE, ILI9481_GRAY);
      tft_font_write_bg(&tft, 446, 6, "D", &gears_regular_32, ILI9481_WHITE, ILI9481_GRAY);
    }
    tft_font_write_bg(&tft, 175, 150, "TO DO", &noto_sans_mono_bold_26, ILI9481_RED, ILI9481_BLACK);
  }

  els_function.refresh_todo = false;
  int key = els_keypad_read();
  if (key > 0)
    printf("key press: %d\n", key);
}
