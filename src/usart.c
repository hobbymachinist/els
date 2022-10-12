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

#include "usart.h"

#include "constants.h"
#include "encoder.h"
#include "keypad.h"
#include "nvic.h"

//------------------------------------------------------------------------------
// Set to 1 if you would like to use UART input to inject keypad scancodes.
//
// See the scan code table below for mapping
//------------------------------------------------------------------------------
#define ELS_USART_ENABLE_KEYPAD_INPUT 0

static uint8_t usart_scancode_table[] = {
  0x1c, // A = Settings
  0x32, // B = Functions
  0x21, // C = F1
  0x23, // D = F2
  0x24, // E = F/P
  0x2b, // F = </>
  0x34, // G = Turn
  0x33, // H = Thread
  0x43, // I = Z/X
  0x3b, // J = 0
  0x42, // K = LO LIM
  0x4b, // L = HI LIM
  0x16, // M = Lock / Unlock
  0x1e, // N = Encoder Multiplier
  0x26, // O = STOP / EXIT
  0x25, // P = READY / OK
  0x2e, // Q = LO JOG
  0x36  // R = HI JOG
};

//------------------------------------------------------------------------------
// ISR
//------------------------------------------------------------------------------
static void els_usart_isr(void) __attribute__ ((interrupt ("IRQ")));

//------------------------------------------------------------------------------
// UART & printf plumbing
//------------------------------------------------------------------------------
void els_usart_setup(void) {
  // UART1 TX, RX
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);
  gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);

  usart_set_baudrate(USART1, 921600);
  usart_set_databits(USART1, 8);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
  usart_set_mode(USART1, USART_MODE_TX_RX);
  usart_set_parity(USART1, USART_PARITY_NONE);
  usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

  // enable the USART.
  usart_enable(USART1);

  // NOTE: key input via UART disabled by default.
  #if ELS_USART_ENABLE_KEYPAD_INPUT
    els_nvic_irq_set_handler(NVIC_USART1_IRQ, els_usart_isr);
    nvic_enable_irq(NVIC_USART1_IRQ);
    usart_enable_rx_interrupt(USART1);
  #endif
}

int _write(int file, char *ptr, int len);
int _write(int file, char *ptr, int len) {
  int i;

  if (file == 1) {
    for (i = 0; i < len; i++)
      usart_send_blocking(USART1, ptr[i]);
    return i;
  }

  errno = EIO;
  return -1;
}

//==============================================================================
// ISR
//==============================================================================
static void els_usart_isr(void) {
  volatile uint8_t byte;
  volatile uint32_t reg;
  volatile bool kp_process;
  static uint8_t kp_state = 0;
  do {
    reg = USART_SR(USART1);
    if (reg & USART_SR_RXNE) {
      byte = USART_DR(USART1);
      kp_process = false;
      switch (kp_state) {
        case 0:
          if (byte == 0x1B)
            kp_state = 1;
          else
            kp_process = true;
          break;
        case 1:
          if (byte == 0x5B)
            kp_state = 2;
          else if (byte != 0x1B) {
            kp_state = 0;
            kp_process = true;
          }
          break;
        case 2:
          // up
          if (byte == 0x41 || byte == 0x44)
            els_encoder_decr(1);
          else if (byte == 0x42 || byte == 0x43)
            els_encoder_incr(1);
          else
            kp_process = true;
          kp_state = 0;
          break;
      }

      if (kp_process) {
        if (byte == '0') {
          els_keypad_write(0x00);
        }
        else if (byte >= 'a' && byte <= 'r') {
          els_keypad_write(usart_scancode_table[byte - 'a']);
        }
        else if (byte >= 'A' && byte <= 'R') {
          els_keypad_write(usart_scancode_table[byte - 'A']);
        }
        else if (byte == '\n' || byte == '\r') {
          els_keypad_write(ELS_KEY_OK);
        }
        else if (byte == 0x7E) { // shift esc
          els_keypad_write(ELS_KEY_EXIT);
        }
      }
    }
  } while ((reg & USART_SR_RXNE) != 0);
}
