#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <libopencm3/cm3/dwt.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/exti.h>

#include "eeprom.h"
#include "delay.h"
#include "utils.h"

//----------------------------------------------------------------------------------------------------------------------
// EEPROM driver for FT24C64A Fremont Micro 8KB I2C EEPROM
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------------------------------------------------
// 0b1010 is default MSB value for I2C EEPROMs
// A0, A1 and A2 are unconnected, treated at V_IL
#define FT24C64A_ADDRESS    0b10100000
#define FT24C64A_PAGE_SIZE  32
#define FT24C64A_PAGE_COUNT 256


#define ELS_I2C_OK          0
#define ELS_I2C_TIMEOUT     1
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
// Internal API
//----------------------------------------------------------------------------------------------------------------------
static bool i2c_write(size_t address, const void *src, size_t size);
static bool i2c_read(size_t address, void *dst, size_t size);
static void i2c_clear_address_flag(void);
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
// API
//----------------------------------------------------------------------------------------------------------------------

void els_eeprom_setup(void) {
  // SCL
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10);
  gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO10);
  gpio_set_af(GPIOB, GPIO_AF4, GPIO10);

  // SDA
  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO12);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO12);
  gpio_set_af(GPIOC, GPIO_AF4, GPIO12);

  // Enable Clock
  rcc_periph_clock_enable(RCC_I2C2);

  // Disable I2C before config
  I2C_CR1(I2C2) &= ~I2C_CR1_PE;

  // reset
  I2C_CR1(I2C2) |= I2C_CR1_SWRST;
  I2C_CR1(I2C2) &= ~I2C_CR1_SWRST;

  // Settings are for 400KHz fast speed.

  // freqrange = APB1 clock frequency / 1e6
  I2C_CR2(I2C2) = 45;

  // trise = (freqrange * 300) / 1000 + 1
  I2C_TRISE(I2C2) = 14;

  // fs (true) = I2C_CCR_FS
  // duty cycle (div2) = 0
  // ccr = (45e6 - 1) / (400e3 * 3) + 1  = 38
  I2C_CCR(I2C2) = I2C_CCR_FS | (I2C_CCR_DUTY & I2C_CCR_DUTY_DIV2) | 38;

  // general call, no stretch
  I2C_CR1(I2C2) = 0;

  // Bit 14 should always be kept at 1 by software
  I2C_OAR1(I2C2) = (1 << 14);

  // Bus master, so nothing set here.
  I2C_OAR2(I2C2) = 0;

  // Enable I2C
  I2C_CR1(I2C2) |= I2C_CR1_PE;
}

bool els_eeprom_erase(void) {
  bool rv = true;
  uint8_t zeros[FT24C64A_PAGE_SIZE];

  memset(zeros, 0, FT24C64A_PAGE_SIZE);
  for (size_t n = 0; n < FT24C64A_PAGE_COUNT && rv; n++)
    rv = i2c_write(n * FT24C64A_PAGE_SIZE, zeros, sizeof(zeros));

  return rv;
}

bool els_eeprom_read(size_t address, void *dst, size_t size) {
  return i2c_read(address, dst, size);
}

bool els_eeprom_write(size_t address, const void *src, size_t size) {
  size_t n = 0, bytes, max_bytes, pending, offset;
  const uint8_t *data = src;

  while (n < size) {
    // offset within page.
    offset = address % FT24C64A_PAGE_SIZE;
    // pending
    pending = size - n;
    // write up to maximum bytes per page, but writes should be scoped to page.
    max_bytes = MIN(FT24C64A_PAGE_SIZE - offset, FT24C64A_PAGE_SIZE);
    // bytes to write
    bytes = MIN(pending, max_bytes);
    // do the actual write
    i2c_write(address, data + n, bytes);
    address += bytes;
    n += bytes;
  }
  return true;
}

//----------------------------------------------------------------------------------------------------------------------
// Internal API
//----------------------------------------------------------------------------------------------------------------------
#define check_until_timeout(condition, timeout_us)  do {                                                                \
                                                      status = ELS_I2C_OK;                                              \
                                                      uint32_t delay_us = 0;                                            \
                                                      while((condition) && delay_us < timeout_us) {                     \
                                                        els_delay_microseconds(10);                                     \
                                                        delay_us += 10;                                                 \
                                                      }                                                                 \
                                                      if ((condition))                                                  \
                                                        status = ELS_I2C_TIMEOUT;                                       \
                                                    } while (0)


#define check_busy_with_timeout(us)                 do {                                                                \
                                                      check_until_timeout(((I2C_SR2(I2C2) & I2C_SR2_BUSY)), us);        \
                                                      if (status != ELS_I2C_OK) {                                       \
                                                        els_printf("write: i2c busy");                                  \
                                                        return false;                                                   \
                                                      }                                                                 \
                                                    } while (0)


#define check_start_with_timeout(us)                do {                                                                \
                                                      check_until_timeout(((I2C_SR1(I2C2) & I2C_SR1_SB) == 0), us);     \
                                                      if (status != ELS_I2C_OK) {                                       \
                                                        els_printf("write: i2c start failed");                          \
                                                        return false;                                                   \
                                                      }                                                                 \
                                                    } while (0)


#define check_address_ack_with_timeout(us)          do {                                                                \
                                                      check_until_timeout(((I2C_SR1(I2C2) & I2C_SR1_ADDR) == 0), us);   \
                                                      if (status != ELS_I2C_OK) {                                       \
                                                        els_printf("write: i2c addr failed");                           \
                                                        I2C_CR1(I2C2) |= I2C_CR1_STOP;                                  \
                                                        I2C_SR1(I2C2) &= ~(I2C_SR1_AF & 0x0000FFFFU);                   \
                                                        return false;                                                   \
                                                      }                                                                 \
                                                    } while (0)


#define check_txe_with_timeout(us)                  do {                                                               \
                                                      check_until_timeout(((I2C_SR1(I2C2) & I2C_SR1_TxE) == 0), us);   \
                                                      if (status != ELS_I2C_OK) {                                      \
                                                        els_printf("write: i2c txe failed");                           \
                                                        I2C_CR1(I2C2) |= I2C_CR1_STOP;                                 \
                                                        I2C_SR1(I2C2) &= ~(I2C_SR1_AF & 0x0000FFFFU);                  \
                                                        return false;                                                  \
                                                      }                                                                \
                                                    } while (0)

#define check_rxne_with_timeout(us)                 do {                                                               \
                                                      check_until_timeout(((I2C_SR1(I2C2) & I2C_SR1_RxNE) == 0), us);  \
                                                      if (status != ELS_I2C_OK) {                                      \
                                                        els_printf("write: i2c rxne failed");                          \
                                                        I2C_CR1(I2C2) |= I2C_CR1_STOP;                                 \
                                                        I2C_SR1(I2C2) &= ~(I2C_SR1_AF & 0x0000FFFFU);                  \
                                                        return false;                                                  \
                                                      }                                                                \
                                                    } while (0)


#define check_btf_with_timeout(us)                  do {                                                               \
                                                      check_until_timeout(((I2C_SR1(I2C2) & I2C_SR1_BTF) == 0), us);   \
                                                      if (status != ELS_I2C_OK) {                                      \
                                                        els_printf("write: i2c btf failed");                           \
                                                        I2C_CR1(I2C2) |= I2C_CR1_STOP;                                 \
                                                        I2C_SR1(I2C2) &= ~(I2C_SR1_AF & 0x0000FFFFU);                  \
                                                        return false;                                                  \
                                                      }                                                                \
                                                    } while (0)


static bool i2c_write(size_t address, const void *src, size_t size) {
  const uint8_t *data = src;
  uint8_t status = ELS_I2C_OK;

  // Check if BUSY
  check_busy_with_timeout(5e3);

  // Reset POS
  I2C_CR1(I2C2) &= ~I2C_CR1_POS;

  // Send START
  I2C_CR1(I2C2) |= I2C_CR1_START;

  // Wait for START condition.
  check_start_with_timeout(5e3);

  I2C_DR(I2C2) = FT24C64A_ADDRESS;

  // Wait for address ACK
  check_address_ack_with_timeout(5e3);

  // Clearing ADDR condition sequence.
  i2c_clear_address_flag();

  // Wait till TXE is set
  check_txe_with_timeout(5e3);

  uint8_t address_hi = (address & 0xFF00) >> 8;
  uint8_t address_lo = (address & 0x00FF);

  I2C_DR(I2C2) = address_hi;
  check_txe_with_timeout(5e3);

  I2C_DR(I2C2) = address_lo;
  check_txe_with_timeout(5e3);

  size_t n = 0;
  while (n < size) {
    // Wait till TXE is set
    check_txe_with_timeout(5e3);
    I2C_DR(I2C2) = data[n++];
    // If byte transfer finished shove in next byte.
    if ((I2C_SR1(I2C2) & I2C_SR1_BTF) && n < size)
      I2C_DR(I2C2) = data[n++];
  }

  // Wait till byte transfer finished.
  check_btf_with_timeout(5e3);
  I2C_CR1(I2C2) |= I2C_CR1_STOP;

  // Minimum write cycle delay ~5ms
  els_delay_microseconds(8e3);
  return true;
}

static bool i2c_read(size_t address, void *dst, size_t size) {
  uint8_t *data = (uint8_t *)dst;
  uint8_t status = ELS_I2C_OK;

  // Check if BUSY
  check_busy_with_timeout(5e3);

  // Reset POS
  I2C_CR1(I2C2) &= ~I2C_CR1_POS;

  // Enable ACK
  I2C_CR1(I2C2) |= I2C_CR1_ACK;

  // Send START
  I2C_CR1(I2C2) |= I2C_CR1_START;

  // Wait for START condition
  check_start_with_timeout(5e3);

  I2C_DR(I2C2) = FT24C64A_ADDRESS;

  // waiting for address ACK.
  check_address_ack_with_timeout(5e3);

  // Clearing ADDR condition sequence.
  i2c_clear_address_flag();

  // Wait till TXE is set
  check_txe_with_timeout(5e3);

  uint8_t address_hi = (address & 0xFF00) >> 8;
  uint8_t address_lo = (address & 0x00FF);

  I2C_DR(I2C2) = address_hi;
  check_txe_with_timeout(5e3);

  I2C_DR(I2C2) = address_lo;
  check_txe_with_timeout(5e3);

  // Send restart
  I2C_CR1(I2C2) |= I2C_CR1_START;

  // Wait for START condition.
  check_start_with_timeout(5e3);

  I2C_DR(I2C2) = (FT24C64A_ADDRESS | 1);
  // waiting for address ACK
  check_address_ack_with_timeout(5e3);

  size_t n = 0, pending = size;

  switch (size) {
    case 1:
      // Disable ACK
      I2C_CR1(I2C2) &= ~I2C_CR1_ACK;
      // Clearing ADDR condition sequence.
      i2c_clear_address_flag();
      // Send STOP
      I2C_CR1(I2C2) |= I2C_CR1_STOP;
      break;
    case 2:
      // Disable ACK
      I2C_CR1(I2C2) &= ~I2C_CR1_ACK;
      // Enable POS
      I2C_CR1(I2C2) |= I2C_CR1_POS;
      // Clearing ADDR condition sequence.
      i2c_clear_address_flag();
      break;
    default:
      // Enable ACK
      I2C_CR1(I2C2) |= I2C_CR1_ACK;
      // Clearing ADDR condition sequence.
      i2c_clear_address_flag();
      break;
  }

  while (pending > 0) {
    switch (pending) {
      case 1:
        check_rxne_with_timeout(5e3);
        data[n++] = I2C_DR(I2C2);
        pending--;
        break;
      case 2:
        check_btf_with_timeout(5e3);
        // Send STOP
        I2C_CR1(I2C2) |= I2C_CR1_STOP;
        data[n++] = I2C_DR(I2C2);
        pending--;
        data[n++] = I2C_DR(I2C2);
        pending--;
        break;
      case 3:
        // Wait for byte transfer
        check_btf_with_timeout(5e3);
        // Disable ACK
        I2C_CR1(I2C2) &= ~I2C_CR1_ACK;
        data[n++] = I2C_DR(I2C2);
        pending--;
        // Wait for byte transfer
        check_btf_with_timeout(5e3);
        // Send STOP
        I2C_CR1(I2C2) |= I2C_CR1_STOP;
        data[n++] = I2C_DR(I2C2);
        pending--;
        data[n++] = I2C_DR(I2C2);
        pending--;
        break;
      default:
        check_rxne_with_timeout(5e3);
        data[n++] = I2C_DR(I2C2);
        pending--;
        if ((I2C_SR1(I2C2) & I2C_SR1_BTF)) {
          data[n++] = I2C_DR(I2C2);
          pending--;
        }
        break;
    }
  }
  return true;
}

static void i2c_clear_address_flag(void) {
  volatile uint32_t reg;
  // Clearing ADDR condition sequence.
  reg = I2C_SR1(I2C2);
  reg = I2C_SR2(I2C2);
  (void)reg;
}
