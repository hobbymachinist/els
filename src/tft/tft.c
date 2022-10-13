#include "tft.h"
#include <libopencm3/stm32/gpio.h>

void tft_gpio_set(uint32_t port, uint32_t pins) {
  GPIO_BSRR(port) = pins;
}

void tft_gpio_clear(uint32_t port, uint32_t pins) {
  GPIO_BSRR(port) = (pins << 16);
}

uint32_t tft_gpio_get(uint32_t port, uint32_t pins) {
  return (GPIO_IDR(port) & pins);
}

void tft_gpio_mode_input(uint32_t port, uint32_t pins) {
  gpio_mode_setup(port, GPIO_MODE_INPUT, GPIO_PUPD_NONE, pins);
}

void tft_gpio_mode_output(uint32_t port, uint32_t pins) {
  gpio_mode_setup(port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pins);
  gpio_set_output_options(port, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, pins);
}
