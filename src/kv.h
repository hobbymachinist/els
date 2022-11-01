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

// version
#define ELS_KV_VERSION      "ELSKV1"

// data block max size
#define ELS_KV_BLOCK_SIZE   (8)

typedef enum {
  ELS_KV_X_PULSES_PER_MM      = 1,
  ELS_KV_Z_PULSES_PER_MM      = 2,
  ELS_KV_X_BACKLASH_UM        = 3,
  ELS_KV_Z_BACKLASH_UM        = 4,
  ELS_KV_X_BACKLASH_MM_S      = 5,
  ELS_KV_Z_BACKLASH_MM_S      = 6,
  ELS_KV_SPINDLE_ENCODER_PPR  = 7,
  ELS_KV_X_RETRACT_JOG_MM_S   = 8,
  ELS_KV_Z_RETRACT_JOG_MM_S   = 9,
  ELS_KV_X_CLOSED_LOOP        = 10,
  ELS_KV_Z_CLOSED_LOOP        = 11,
  ELS_KV_X_DRO_INVERT         = 12,
  ELS_KV_Z_DRO_INVERT         = 13,
  ELS_KV_X_JOG_MM_S           = 14,
  ELS_KV_Z_JOG_MM_S           = 15
} els_kv_t;

void els_kv_setup(void);
void els_kv_read(els_kv_t, void *dst, size_t size);
void els_kv_write(els_kv_t, const void *src, size_t size);

#ifdef __cplusplus
}
#endif

