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
  volatile int32_t xpos_um;
  volatile int32_t zpos_um;
} els_dro_t;

extern els_dro_t els_dro;

void els_dro_setup(void);
void els_dro_zero_x(void);
void els_dro_zero_z(void);

#ifdef __cplusplus
}
#endif

