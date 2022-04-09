#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libopencm3/cm3/dwt.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/exti.h>

//==============================================================================
// Platform
//==============================================================================
#include "clock.h"
#include "delay.h"
#include "gpio.h"
#include "nvic.h"
#include "usart.h"
#include "timer.h"

//==============================================================================
// TFT
//==============================================================================
#include "tft/tft.h"

tft_device_t tft = {
  .ili9481.rd   = {.port = GPIOB, .pin = GPIO0},
  .ili9481.wr   = {.port = GPIOB, .pin = GPIO6},
  .ili9481.rs   = {.port = GPIOB, .pin = GPIO7},
  .ili9481.cs   = {.port = GPIOB, .pin = GPIO8},
  .ili9481.rst  = {.port = GPIOB, .pin = GPIO9},
  .ili9481.data = {.port = GPIOA, .pin = GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5 | GPIO6 | GPIO7 }
};

//==============================================================================
// ELS Core
//==============================================================================
#include "constants.h"

// peripherals
#include "dro.h"
#include "eeprom.h"
#include "encoder.h"
#include "keypad.h"
#include "spindle.h"
#include "stepper.h"

// config & db
#include "config.h"
#include "kv.h"

// operations
#include "turning.h"
#include "threading.h"

// els functions
#include "function.h"

//==============================================================================
// Helpers
//==============================================================================
static void els_screenshot(void) {
  for (size_t row = 0; row < 320; row++) {
    for (size_t col = 0; col < 480; col++) {
      tft_rgb_t color;
      tft_pixel_get(&tft, col, row, &color);
      printf("%02x%02x%02x", color.r, color.g, color.b);
      if ((col + 1) % 20 == 0)
        printf("\n");
    }
  }
}

//==============================================================================
// Main loop
//==============================================================================
int main(void) {
  els_nvic_setup();
  els_clock_setup();
  els_gpio_setup();
  els_usart_setup();

  printf("\nels v0.1.0 - build %s %s\n", GIT_SHA, BUILD_TS);

  // core
  els_timer_setup();

  // peripherals
  els_encoder_setup();
  els_encoder_start();
  els_keypad_setup();

  // IMPORTANT: needs to happen as late as possible.
  els_delay_setup();

  // init kv db & load config
  els_eeprom_setup();
  els_kv_setup();
  els_config_setup();

  els_dro_setup();
  els_stepper_setup();
  els_spindle_setup();

  // setup display
  tft_init(&tft);
  tft_invert(&tft, true);
  //tft_rotate(&tft, ROTATE_90);
  tft_rotate(&tft, ROTATE_270);

  // functions manager, needs to be called after config read.
  els_function_init();

  while (1) {
    switch (els_keypad_peek()) {
      case 0x00:
        els_screenshot();
        els_keypad_flush();
        break;
      case ELS_KEY_LOCK:
        if (els_keypad_locked()) {
          printf("keypad: unlock\n");
          els_keypad_unlock();
        }
        else {
          printf("keypad: lock\n");
          els_keypad_lock();
        }
        els_keypad_flush();
        break;
      case ELS_KEY_ENC_MULT:
        els_encoder_inc_multiplier();
        els_keypad_flush();
        break;
      default:
        if (els_keypad_locked())
          els_keypad_flush();
        break;
    }

    els_function_update();
    els_spindle_update();
  }

  return 0;
}
