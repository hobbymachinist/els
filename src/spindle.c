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
#include <libopencm3/stm32/f4/timer.h>

#include "gpio.h"
#include "timer.h"

#include "spindle.h"

#include "config.h"
#include "constants.h"

#define ELS_ENCODER_TIMER1     TIM3
#define ELS_ENCODER_TIMER1_IRQ NVIC_TIM3_IRQ
#define ELS_ENCODER_TIMER1_MAX UINT16_MAX
#define ELS_ENCODER_TIMER1_RCC RCC_TIM3
#define ELS_ENCODER_TIMER1_RST RST_TIM3
#define ELS_ENCODER_TIMER1_ISR tim3_isr

//==============================================================================
// Internal state
//==============================================================================
static struct {
  uint32_t rpm;
  volatile uint32_t rotations;

  uint32_t last_rotations;
  uint64_t last_updated_at;
} spindle;


//==============================================================================
// Internal functions
//==============================================================================
static void els_spindle_configure_gpio(void);
static void els_spindle_configure_timer(void);

//==============================================================================
// ISR
//==============================================================================
void ELS_ENCODER_TIMER1_ISR(void) {
  TIM_SR(ELS_ENCODER_TIMER1) &= ~TIM_SR_UIF;
  spindle.rotations++;
}

//==============================================================================
// API
//==============================================================================
void els_spindle_setup(void) {
  els_spindle_configure_gpio();
  els_spindle_configure_timer();
}

void els_spindle_update(void) {
  uint32_t delta = spindle.rotations - spindle.last_rotations;
  uint64_t elapsed = els_timer_elapsed_microseconds();

  if (delta >= 4) {
    spindle.rpm = (spindle.rpm * 0.6) + (((delta * 60e6) / (elapsed - spindle.last_updated_at)) * 0.4);
    spindle.last_rotations = spindle.rotations;
    spindle.last_updated_at = elapsed;
  }
  else {
    if (spindle.rpm >= 600) {
      if ((elapsed - spindle.last_updated_at) >= 5e5)
        spindle.rpm = 0;
    }
    else if (spindle.rpm >= 200) {
      if ((elapsed - spindle.last_updated_at) >= 2e6)
        spindle.rpm = 0;
    }
    else if ((elapsed - spindle.last_updated_at) >= 5e6) {
      spindle.rpm = 0;
    }
  }
}

float els_spindle_get_angle(void) {
  return (timer_get_counter(ELS_ENCODER_TIMER1) * 360.0) / (els_config->spindle_encoder_ppr * 2);
}

uint16_t els_spindle_get_counter(void) {
  return timer_get_counter(ELS_ENCODER_TIMER1);
}

uint16_t els_spindle_get_rpm(void) {
  return spindle.rpm;
}

els_spindle_direction_t els_spindle_get_direction(void) {
  static els_spindle_direction_t last_direction = ELS_S_DIRECTION_IDLE;

  if (spindle.rpm > 10) {
    if (last_direction == ELS_S_DIRECTION_IDLE)
      last_direction = (TIM_CR1(ELS_ENCODER_TIMER1) & TIM_CR1_DIR_DOWN) ? ELS_S_DIRECTION_CCW : ELS_S_DIRECTION_CW;
  }
  else {
    last_direction = ELS_S_DIRECTION_IDLE;
  }

  return last_direction;
}

//==============================================================================
// Internal functions
//==============================================================================
static void els_spindle_configure_gpio(void) {
  gpio_mode_setup(ELS_S_ENCODER1_PORTA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, ELS_S_ENCODER1_PINA);
  gpio_mode_setup(ELS_S_ENCODER1_PORTB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, ELS_S_ENCODER1_PINB);

  gpio_set_af(ELS_S_ENCODER1_PORTA, ELS_S_ENCODER1_AFA, ELS_S_ENCODER1_PINA);
  gpio_set_af(ELS_S_ENCODER1_PORTB, ELS_S_ENCODER1_AFB, ELS_S_ENCODER1_PINB);
}

static void els_spindle_configure_timer(void) {
  rcc_periph_clock_enable(ELS_ENCODER_TIMER1_RCC);
  rcc_periph_reset_pulse(ELS_ENCODER_TIMER1_RST);

  timer_set_prescaler(ELS_ENCODER_TIMER1, 0);
  timer_slave_set_mode(ELS_ENCODER_TIMER1, TIM_SMCR_SMS_EM2);

  timer_ic_set_input(ELS_ENCODER_TIMER1, TIM_IC1, TIM_IC_IN_TI1);
  timer_ic_set_input(ELS_ENCODER_TIMER1, TIM_IC2, TIM_IC_IN_TI1);

  timer_set_period(ELS_ENCODER_TIMER1, (els_config->spindle_encoder_ppr * 2) - 1);

  timer_ic_set_filter(ELS_ENCODER_TIMER1, TIM_IC1, TIM_IC_CK_INT_N_4);
  timer_ic_set_filter(ELS_ENCODER_TIMER1, TIM_IC2, TIM_IC_CK_INT_N_4);

  timer_set_counter(ELS_ENCODER_TIMER1, 0);

  nvic_set_priority(ELS_ENCODER_TIMER1_IRQ, 4);
  nvic_enable_irq(ELS_ENCODER_TIMER1_IRQ);

  timer_enable_update_event(ELS_ENCODER_TIMER1);
  timer_enable_counter(ELS_ENCODER_TIMER1);
  timer_enable_irq(ELS_ENCODER_TIMER1, TIM_DIER_UIE);
}
