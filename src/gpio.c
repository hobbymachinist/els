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

#include "gpio.h"
#include "constants.h"

//==============================================================================
// API
//==============================================================================
void els_gpio_setup(void) {

  //------------------------------------------------------------------------------
  // Indicator LED
  //------------------------------------------------------------------------------
  els_gpio_mode_output(LED_PORT, LED_PIN);
  gpio_clear(LED_PORT, LED_PIN);

  //------------------------------------------------------------------------------
  // Z-Axis Stepper
  //------------------------------------------------------------------------------
  els_gpio_mode_output(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  els_gpio_mode_output(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN);
  els_gpio_mode_output(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);

  els_gpio_clear(ELS_Z_ENA_PORT, ELS_Z_ENA_PIN);
  els_gpio_clear(ELS_Z_DIR_PORT, ELS_Z_DIR_PIN);
  els_gpio_clear(ELS_Z_PUL_PORT, ELS_Z_PUL_PIN);

  //------------------------------------------------------------------------------
  // X-Axis Stepper
  //------------------------------------------------------------------------------
  els_gpio_mode_output(ELS_X_ENA_PORT, ELS_X_ENA_PIN);
  els_gpio_mode_output(ELS_X_DIR_PORT, ELS_X_DIR_PIN);
  els_gpio_mode_output(ELS_X_PUL_PORT, ELS_X_PUL_PIN);

  els_gpio_clear(ELS_X_ENA_PORT, ELS_X_ENA_PIN);
  els_gpio_clear(ELS_X_DIR_PORT, ELS_X_DIR_PIN);
  els_gpio_clear(ELS_X_PUL_PORT, ELS_X_PUL_PIN);
}

void els_gpio_mode_input(uint32_t port, uint32_t pins) {
  gpio_mode_setup(port, GPIO_MODE_INPUT, GPIO_PUPD_NONE, pins);
}

void els_gpio_mode_output(uint32_t port, uint32_t pins) {
  gpio_mode_setup(port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pins);
  gpio_set_output_options(port, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, pins);
}
