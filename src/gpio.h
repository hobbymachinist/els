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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t port;
  uint32_t pin;
} els_gpio_t;

//==============================================================================
// GPIO Mapping Summary.
//==============================================================================

// PA0  - TFT Data
// PA1  - TFT Data
// PA2  - TFT Data
// PA3  - TFT Data
// PA4  - TFT Data
// PA5  - TFT Data
// PA6  - TFT Data
// PA7  - TFT Data
// PA8  -
// PA9  - USART TX
// PA10 - USART RX
// PA11 - DRO X-A (Int)
// PA12 - DRO X-B (Int)
// PA13 -
// PA14 -
// PA15 -
//
// PB0  - TFT RD
// PB1  - DRO Z-A (Int)
// PB2  -
// PB3  - Spindle Enc A (Int)
// PB4  - Spindle Enc B
// PB5  - DRO Z-B (Int)
// PB6  - TFT WR
// PB7  - TFT RS
// PB8  - TFT CS
// PB9  - TFT RST
// PB10 - EEPROM SCL
// PB11 - N/A
// PB12 -
// PB13 - Z ENA
// PB14 - Z DIR
// PB15 - Z PUL
//
// PC0  -
// PC1  -
// PC2  - Input Enc A (Int)
// PC3  - Input Enc B
// PC4  - PS/2 Keypad CLK (Int)
// PC5  - PS/2 Keypad DAT
// PC6  - Spindle Enc A (Timer)
// PC7  - Spindle Enc B (Timer)
// PC8  -
// PC9  -
// PC10 -
// PC11 -
// PC12 - EEPROM SDA
// PC13 - X ENA
// PC14 - X DIR
// PC15 - X PUL
//
// PH0  -
// PH1  -
//==============================================================================

void els_gpio_setup(void);
void els_gpio_mode_input(uint32_t port, uint32_t pins);
void els_gpio_mode_output(uint32_t port, uint32_t pins);

//==============================================================================
// Helpers
//==============================================================================

#define els_gpio_get(port, pins)   (GPIO_IDR(port) & pins)
#define els_gpio_set(port, pins)   GPIO_BSRR(port) = pins
#define els_gpio_clear(port, pins) GPIO_BSRR(port) = (pins << 16)
#define els_gpio_toggle(port, pins) do { \
                                      uint32_t data   = GPIO_ODR(port); \
                                      GPIO_BSRR(port) = ((data & pins) << 16) | (~data & pins); \
                                    } while(0)

#ifdef __cplusplus
}
#endif
