#pragma once

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

#ifdef __cplusplus
extern "C" {
#endif

void els_clock_setup(void);

#ifdef __cplusplus
}
#endif
