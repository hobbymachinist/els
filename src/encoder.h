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
int32_t els_encoder_read(void);
void    els_encoder_write(int32_t);

#ifdef __cplusplus
}
#endif

