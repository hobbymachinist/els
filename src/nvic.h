#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#include "libopencm3/cm3/vector.h"

void els_nvic_setup(void);
void els_nvic_irq_set_handler(uint8_t irq, vector_table_entry_t isr);

#ifdef __cplusplus
}
#endif

