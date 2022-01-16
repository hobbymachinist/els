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

typedef struct {
  uint32_t x_pulses_per_mm;
  uint32_t z_pulses_per_mm;

  uint32_t x_backlash_um;
  uint32_t z_backlash_um;

  uint32_t spindle_encoder_ppr;

  // computed
  uint32_t x_backlash_pulses;
  uint32_t z_backlash_pulses;
} els_config_t;

extern const els_config_t *els_config;

void els_config_setup(void);
void els_config_start(void);
void els_config_update(void);
void els_config_stop(void);

#ifdef __cplusplus
}
#endif

