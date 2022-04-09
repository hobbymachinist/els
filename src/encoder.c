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

#include "encoder.h"
#include "timer.h"
#include "gpio.h"

#include "constants.h"

//==============================================================================
// Internal State
//==============================================================================
static volatile int32_t encoder_ticks, encoder_dir;
static volatile uint64_t t1 = 0, t2 = 0;

static uint32_t direction_debounce_us, rotation_debounce_us;
static int16_t  encoder_multiplier = 1;
//==============================================================================
// ISR
//==============================================================================
void ELS_I_ENCODER_ISR(void) {

// ------------------------------------------------------
// Begin: check EXTI flag, required for shared EXTI ISRs.
// ------------------------------------------------------
#if ELS_I_ENCODER_PINA > GPIO4
  if (exti_get_flag_status(ELS_I_ENCODER_EXTI)) {
#endif
    exti_reset_request(ELS_I_ENCODER_EXTI);
    // rotation debounce
    t2 = els_timer_elapsed_microseconds();
    if (t2 - t1 > rotation_debounce_us) {
      // direction debounce
      if (els_gpio_get(ELS_I_ENCODER_PORTB, ELS_I_ENCODER_PINB)) {
        if (encoder_dir == -1 || (t2 - t1) > direction_debounce_us) {
          encoder_dir = -1;
          encoder_ticks--;
        }
      }
      else {
        // direction debounce
        if (encoder_dir == 1 || (t2 - t1) > direction_debounce_us) {
          encoder_dir = 1;
          encoder_ticks++;
        }
      }
      t1 = t2;
    }
// ------------------------------------------------------
// End: check EXTI flag, required for shared EXTI ISRs.
// ------------------------------------------------------
#if ELS_I_ENCODER_PINA > GPIO4
  }
#endif
}

//==============================================================================
// API
//==============================================================================
void els_encoder_setup(void) {
  gpio_mode_setup(ELS_I_ENCODER_PORTA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, ELS_I_ENCODER_PINA);
  gpio_mode_setup(ELS_I_ENCODER_PORTB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, ELS_I_ENCODER_PINB);

  exti_select_source(ELS_I_ENCODER_EXTI, ELS_I_ENCODER_PORTA);
  exti_set_trigger(ELS_I_ENCODER_EXTI, EXTI_TRIGGER_RISING);
  exti_enable_request(ELS_I_ENCODER_EXTI);
}

void els_encoder_start(void) {
  encoder_ticks = 0;
  direction_debounce_us = 100e3;
  rotation_debounce_us = 25e3 / 10;

  nvic_set_priority(ELS_I_ENCODER_IRQ, 4);
  nvic_enable_irq(ELS_I_ENCODER_IRQ);
}

void els_encoder_set_direction_debounce(uint32_t delay_us) {
  direction_debounce_us = delay_us;
}

void els_encoder_set_rotation_debounce(uint32_t delay_us) {
  rotation_debounce_us = delay_us;
}

void els_encoder_stop(void) {
  encoder_ticks = 0;
  nvic_disable_irq(ELS_I_ENCODER_IRQ);
}

void els_encoder_reset(void) {
  encoder_ticks = 0;
}

int32_t els_encoder_read(void) {
  return encoder_ticks;
}

void els_encoder_write(int32_t ticks) {
  encoder_ticks = ticks;
}

void els_encoder_incr(uint16_t delta) {
  encoder_ticks += delta;
}

void els_encoder_decr(uint16_t delta) {
  encoder_ticks -= delta;
}

int16_t els_encoder_get_multiplier(void) {
  return encoder_multiplier;
}

void els_encoder_inc_multiplier(void) {
  encoder_multiplier = encoder_multiplier > 10 ? 1 : encoder_multiplier * 10;
}
