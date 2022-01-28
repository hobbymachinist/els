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

#include "delay.h"
#include "keypad.h"
#include "ringbuffer.h"
#include "gpio.h"

#include "constants.h"

//==============================================================================
// Internal State
//==============================================================================
uint8_t buffer[64];
bool    locked;
ringbuffer_t ringbuffer;

//==============================================================================
// ISR
//==============================================================================
static uint8_t usart_scancode_table[] = {
  0x1c, // A
  0x32, // B
  0x21, // C
  0x23, // D
  0x24, // E
  0x2b, // F
  0x34, // G
  0x33, // H
  0x43, // I
  0x3b, // J
  0x42, // K
  0x4b, // L
  0x16, // M
  0x1e, // N
  0x26, // O
  0x25, // P
  0x2e, // Q
  0x36  // R
};

void usart1_isr(void) {
  uint8_t byte;
  uint32_t reg;
  do {
    reg = USART_SR(USART1);
    if (reg & USART_SR_RXNE) {
      byte = USART_DR(USART1);
      if (byte == '0')
        ringbuffer_putc(&ringbuffer, 0x00);
      else if (byte >= 'a' && byte <= 'r')
        ringbuffer_putc(&ringbuffer, usart_scancode_table[byte - 'a']);
      else if (byte >= 'A' && byte <= 'R')
        ringbuffer_putc(&ringbuffer, usart_scancode_table[byte - 'A']);
    }
  } while ((reg & USART_SR_RXNE) != 0);
}

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

#define ELS_KEYPAD_ENABLE_USART_INPUT 0

//==============================================================================
// API
//==============================================================================
void els_keypad_setup(void) {
  ringbuffer_init(&ringbuffer, sizeof(buffer), buffer);

  // NOTE: key input via UART disabled by default.
  #if ELS_KEYPAD_ENABLE_USART_INPUT
    nvic_enable_irq(NVIC_USART1_IRQ);
    usart_enable_rx_interrupt(USART1);
  #endif

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
      case ELS_KEY_UNLOCK:
      case ELS_KEY_FUN_TURN:
      case ELS_KEY_FUN_THREAD:
      case ELS_KEY_FUN_SELECT:
        els_keypad_unread(data);
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

void els_keypad_unread(uint8_t c) {
  ringbuffer_putc(&ringbuffer, c);
}
