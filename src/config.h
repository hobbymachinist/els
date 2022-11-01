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

  uint32_t x_jog_mm_s;
  uint32_t z_jog_mm_s;
  uint32_t x_retract_jog_mm_s;
  uint32_t z_retract_jog_mm_s;

  // boolean
  bool     x_closed_loop;
  bool     z_closed_loop;
  bool     x_dro_invert;
  bool     z_dro_invert;

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

