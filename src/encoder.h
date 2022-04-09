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

void    els_encoder_setup(void);
void    els_encoder_start(void);
void    els_encoder_set_direction_debounce(uint32_t usec);
void    els_encoder_set_rotation_debounce(uint32_t usec);
void    els_encoder_stop(void);
void    els_encoder_reset(void);
void    els_encoder_incr(uint16_t);
void    els_encoder_decr(uint16_t);
int32_t els_encoder_read(void);
void    els_encoder_write(int32_t);
int16_t els_encoder_get_multiplier(void);
void    els_encoder_inc_multiplier(void);

#ifdef __cplusplus
}
#endif

