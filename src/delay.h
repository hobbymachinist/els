#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void     els_delay_setup(void);
void     els_delay_microseconds(uint32_t microseconds);
void     els_delay_milliseconds(uint32_t milliseconds);
void     els_delay_ticks(uint32_t ticks);
void     els_delay_reset(void);
uint32_t els_delay_elapsed_microseconds(void);

#ifdef __cplusplus
}
#endif

