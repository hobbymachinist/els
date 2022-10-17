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
#include "dro.h"
#include "nvic.h"
#include "config.h"

// linear glass scale resolution in microns.
#define  ELS_DRO_X_RESOLUTION 5
#define  ELS_DRO_Z_RESOLUTION 5

//==============================================================================
// State
//==============================================================================
volatile els_dro_t els_dro;

//==============================================================================
// Internal functions
//==============================================================================
static void els_dro_configure_gpio(void);
static void els_dro_configure_isr(void);

//==============================================================================
// ISR
//==============================================================================
static void els_dro_x_isr(void) __attribute__ ((interrupt ("IRQ")));
static void els_dro_z_isr(void) __attribute__ ((interrupt ("IRQ")));

//==============================================================================
// API
//==============================================================================
void els_dro_setup(void) {
  els_dro.xpos_um = 0;
  els_dro.zpos_um = 0;

  els_dro_configure_gpio();
  els_dro_configure_isr();
}

void els_dro_zero_x(void) {
  els_dro.xpos_um = 0;
}

void els_dro_zero_z(void) {
  els_dro.zpos_um = 0;
}

//==============================================================================
// Internal functions
//==============================================================================
static void els_dro_configure_gpio(void) {
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO11);
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO12);

  gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO1);
  gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO5);

  exti_select_source(EXTI11, GPIOA);
  exti_set_trigger(EXTI11, EXTI_TRIGGER_BOTH);
  exti_enable_request(EXTI11);

  exti_select_source(EXTI12, GPIOA);
  exti_set_trigger(EXTI12, EXTI_TRIGGER_BOTH);
  exti_enable_request(EXTI12);

  exti_select_source(EXTI1, GPIOB);
  exti_set_trigger(EXTI1, EXTI_TRIGGER_BOTH);
  exti_enable_request(EXTI1);

  exti_select_source(EXTI5, GPIOB);
  exti_set_trigger(EXTI5, EXTI_TRIGGER_BOTH);
  exti_enable_request(EXTI5);
}

static void els_dro_configure_isr(void) {
  els_nvic_irq_set_handler(NVIC_EXTI15_10_IRQ, els_dro_x_isr);
  nvic_set_priority(NVIC_EXTI15_10_IRQ, 1);
  nvic_enable_irq(NVIC_EXTI15_10_IRQ);

  els_nvic_irq_set_handler(NVIC_EXTI1_IRQ, els_dro_z_isr);
  nvic_set_priority(NVIC_EXTI1_IRQ, 1);
  nvic_enable_irq(NVIC_EXTI1_IRQ);

  els_nvic_irq_set_handler(NVIC_EXTI9_5_IRQ, els_dro_z_isr);
  nvic_set_priority(NVIC_EXTI9_5_IRQ, 1);
  nvic_enable_irq(NVIC_EXTI9_5_IRQ);
}

//==============================================================================
// ISR
//==============================================================================
static void els_dro_x_isr(void) {
  volatile uint32_t x1, x2;
  const int32_t x_dro_dir = els_config->x_dro_invert ? -1 : 1;

  x1 = els_gpio_get(GPIOA, GPIO11);
  x2 = els_gpio_get(GPIOA, GPIO12);

  // ------------ X AXIS ------------------------------------------------------
  if (exti_get_flag_status(EXTI11)) {
    exti_reset_request(EXTI11);
    if (x1) {
      if (x2)
        els_dro.xpos_um -= (x_dro_dir * ELS_DRO_X_RESOLUTION);
      else
        els_dro.xpos_um += (x_dro_dir * ELS_DRO_X_RESOLUTION);
    }
    else {
      if (x2)
        els_dro.xpos_um += (x_dro_dir * ELS_DRO_X_RESOLUTION);
      else
        els_dro.xpos_um -= (x_dro_dir * ELS_DRO_X_RESOLUTION);
    }
  }

  if (exti_get_flag_status(EXTI12)) {
    exti_reset_request(EXTI12);
    if (x1) {
      if (x2)
        els_dro.xpos_um += (x_dro_dir * ELS_DRO_X_RESOLUTION);
      else
        els_dro.xpos_um -= (x_dro_dir * ELS_DRO_X_RESOLUTION);
    }
    else {
      if (x2)
        els_dro.xpos_um -= (x_dro_dir * ELS_DRO_X_RESOLUTION);
      else
        els_dro.xpos_um += (x_dro_dir * ELS_DRO_X_RESOLUTION);
    }
  }
}


static void els_dro_z_isr(void) {
  volatile uint32_t x1, x2, exti1, exti5;
  const int32_t z_dro_dir = els_config->z_dro_invert ? -1 : 1;

  x1 = els_gpio_get(GPIOB, GPIO1);
  x2 = els_gpio_get(GPIOB, GPIO5);
  exti1 = exti_get_flag_status(EXTI1);
  exti5 = exti_get_flag_status(EXTI5);

  // ------------ Z AXIS ------------------------------------------------------
  if (exti1) {
    exti_reset_request(EXTI1);
    if (x1) {
      if (x2)
        els_dro.zpos_um -= (z_dro_dir * ELS_DRO_Z_RESOLUTION);
      else
        els_dro.zpos_um += (z_dro_dir * ELS_DRO_Z_RESOLUTION);
    }
    else {
      if (x2)
        els_dro.zpos_um += (z_dro_dir * ELS_DRO_Z_RESOLUTION);
      else
        els_dro.zpos_um -= (z_dro_dir * ELS_DRO_Z_RESOLUTION);
    }
  }

  if (exti5) {
    exti_reset_request(EXTI5);
    if (x1) {
      if (x2)
        els_dro.zpos_um += (z_dro_dir * ELS_DRO_Z_RESOLUTION);
      else
        els_dro.zpos_um -= (z_dro_dir * ELS_DRO_Z_RESOLUTION);
    }
    else {
      if (x2)
        els_dro.zpos_um -= (z_dro_dir * ELS_DRO_Z_RESOLUTION);
      else
        els_dro.zpos_um += (z_dro_dir * ELS_DRO_Z_RESOLUTION);
    }
  }
}
