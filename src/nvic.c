#include <libopencm3/cm3/dwt.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/f4/timer.h>

#include "libopencm3/cm3/scb.h"

#include "nvic.h"

static vector_table_t els_vector_table __attribute__((aligned(512)));

void els_nvic_setup(void) {
  // copy vector table to RAM
  memcpy(&els_vector_table, &vector_table, sizeof(vector_table));

  // relocate
  SCB_VTOR = (uint32_t)&els_vector_table;
}

void els_nvic_irq_set_handler(uint8_t irq_number, vector_table_entry_t isr) {
  if (irq_number < NVIC_IRQ_COUNT)
    els_vector_table.irq[irq_number] = isr;
}
