#include <libopencm3/cm3/dwt.h>
#include <libopencm3/cm3/scs.h>

#include "delay.h"
#include "rcc_config.h"

void els_delay_setup(void) {
  SCS_DEMCR |= SCS_DEMCR_TRCENA;
  // unlock cm7
  DWT_LAR = 0xC5ACCE55;
  DWT_CYCCNT = 0;
  DWT_CTRL |= DWT_CTRL_CYCCNTENA;
}

void els_delay_microseconds(uint32_t microseconds) {
  DWT_CYCCNT = 0;
  uint32_t end_ticks = microseconds * RCC_CLOCK_BASE;
  while (DWT_CYCCNT < end_ticks);
}

void els_delay_ticks(uint32_t ticks) {
  DWT_CYCCNT = 0;
  while (DWT_CYCCNT < ticks);
}

void els_delay_reset(void) {
  DWT_CYCCNT = 0;
}

uint32_t els_delay_elapsed_microseconds(void) {
  return (DWT_CYCCNT / RCC_CLOCK_BASE);
}
