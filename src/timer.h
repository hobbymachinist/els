#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void     els_timer_setup(void);
uint64_t els_timer_elapsed_microseconds(void);
uint32_t els_timer_elapsed_milliseconds(void);

#ifdef __cplusplus
}
#endif

