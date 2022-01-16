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
#include "rcc_config.h"

#define ELS_TIMER     TIM2
#define ELS_TIMER_IRQ NVIC_TIM2_IRQ
#define ELS_TIMER_MAX UINT16_MAX
#define ELS_TIMER_RCC RCC_TIM2
#define ELS_TIMER_RST RST_TIM2
#define ELS_TIMER_ISR tim2_isr

//==============================================================================
// Internal state
//==============================================================================
static uint64_t overflow_microseconds;

//==============================================================================
// ISR
//==============================================================================
void ELS_TIMER_ISR(void) {
  TIM_SR(ELS_TIMER) &= ~TIM_SR_UIF;
  overflow_microseconds += ELS_TIMER_MAX;
}

//==============================================================================
// API
//==============================================================================
void els_timer_setup(void) {
  rcc_periph_clock_enable(ELS_TIMER_RCC);
  rcc_periph_reset_pulse(ELS_TIMER_RST);

  // clock division 0, alignment edge, count up.
  timer_set_mode(ELS_TIMER, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

  // 100us counter, ~10KHz
  timer_set_prescaler(ELS_TIMER, ((rcc_apb1_frequency * 2) / 1e4) - 1);

  // keep running
  timer_disable_preload(ELS_TIMER);
  timer_continuous_mode(ELS_TIMER);

  // set to timer max
  timer_set_period(ELS_TIMER, ELS_TIMER_MAX);

  nvic_set_priority(ELS_TIMER_IRQ, 4);
  nvic_enable_irq(ELS_TIMER_IRQ);
  timer_enable_update_event(ELS_TIMER);

  overflow_microseconds = 0;
  timer_set_counter(ELS_TIMER, 0);
  timer_enable_counter(ELS_TIMER);
  timer_enable_irq(ELS_TIMER, TIM_DIER_UIE);
}

uint64_t els_timer_elapsed_microseconds(void) {
  return (timer_get_counter(ELS_TIMER) + overflow_microseconds) * 100;
}

uint32_t els_timer_elapsed_milliseconds(void) {
  return (timer_get_counter(ELS_TIMER) + overflow_microseconds) / 10;
}
