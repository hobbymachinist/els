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

#include "keypad.h"

#include "constants.h"
#include "delay.h"
#include "gpio.h"
#include "ringbuffer.h"

//==============================================================================
// Internal State
//==============================================================================
uint8_t buffer[64];
bool    locked;
ringbuffer_t ringbuffer;

volatile uint8_t byte = 0;
volatile uint8_t bits = 0;
volatile uint8_t bytes = 0;

//------------------------------------------------------------------------------
// PS/2
//
// http://www.lucadavidian.com/2017/11/15/interfacing-ps2-keyboard-to-a-microcontroller/
//
// A frame consists of:
//
// 1. start bit (always low)
// 2. 8 data bits, LSB first
// 3. a odd parity bit
// 4. a stop bit (always high)
//
//------------------------------------------------------------------------------
void ELS_KEYPAD_ISR(void) {
#if ELS_KEYPAD_CLK_PIN > GPIO4
  if (exti_get_flag_status(ELS_KEYPAD_EXTI)) {
#endif

  exti_reset_request(ELS_KEYPAD_EXTI);
  if (bits > 0 && bits < 9) {
    byte >>= 1;
    els_delay_microseconds(10);
    if (gpio_get(ELS_KEYPAD_DAT_PORT, ELS_KEYPAD_DAT_PIN))
      byte |= 0x80;
  }

  // D-8203 PS/2 keypad
  //
  // https://www.aliexpress.com/item/1005002340320709.html
  //
  // make & break codes come in a triplet, which is non-compliant for the keypad.
  //
  // 1. press => scan code, 0xf0
  // 2. release => scan code
  if (bits++ == 10) {
    bytes++;
    if (bytes > 2) {
      bytes = 0;
      ringbuffer_putc(&ringbuffer, byte);
    }

    // ignore reset & power-on ack.
    if (byte == 0xaa) {
      printf("kp ok\n");
      bytes = 0;
    }

    // process next byte.
    byte = bits = 0;
  }

#if ELS_KEYPAD_CLK_PIN > GPIO4
  }
#endif
}

//==============================================================================
// API
//==============================================================================
void els_keypad_setup(void) {
  ringbuffer_init(&ringbuffer, sizeof(buffer), buffer);

  // PS/2 keypad.
  gpio_mode_setup(ELS_KEYPAD_CLK_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ELS_KEYPAD_CLK_PIN);
  gpio_mode_setup(ELS_KEYPAD_DAT_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ELS_KEYPAD_DAT_PIN);

  exti_select_source(ELS_KEYPAD_EXTI, ELS_KEYPAD_CLK_PORT);
  exti_set_trigger(ELS_KEYPAD_EXTI, EXTI_TRIGGER_FALLING);
  exti_enable_request(ELS_KEYPAD_EXTI);

  nvic_set_priority(ELS_KEYPAD_IRQ, 4);
  nvic_enable_irq(ELS_KEYPAD_IRQ);
}

void els_keypad_lock(void) {
  locked = true;
}

void els_keypad_unlock(void) {
  locked = false;
}

bool els_keypad_locked(void) {
  return locked;
}

int els_keypad_read(void) {
  uint8_t data;
  if (locked)
    return ELS_KEY_EOF;

  if (ringbuffer_getc(&ringbuffer, &data)) {
    switch (data) {
      // top level function keys.
      case ELS_KEY_LOCK:
      case ELS_KEY_ENC_MULT:
      case ELS_KEY_FUN_TURN:
      case ELS_KEY_FUN_THREAD:
      case ELS_KEY_FUN_SELECT:
        els_keypad_write(data);
        return ELS_KEY_EOF;
      default:
        return data;
    }
  }
  else
    return ELS_KEY_EOF;
}

int els_keypad_peek(void) {
  uint8_t data;
  if (ringbuffer_peek(&ringbuffer, &data))
    return data;
  else
    return ELS_KEY_EOF;
}

void els_keypad_flush(void) {
  ringbuffer.read_ptr = ringbuffer.write_ptr;
}

void els_keypad_write(uint8_t c) {
  ringbuffer_putc(&ringbuffer, c);
}
