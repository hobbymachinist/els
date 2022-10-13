#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// speeds up render by avoiding function calls.

#include <libopencm3/stm32/gpio.h>

#define tft_gpio_set(port, pins) GPIO_BSRR(port) = pins
#define tft_gpio_clear(port, pins) GPIO_BSRR(port) = (pins << 16)

#include "delay.h"

#define tft_delay_microseconds(us) els_delay_microseconds(us)

#ifdef __cplusplus
}
#endif

