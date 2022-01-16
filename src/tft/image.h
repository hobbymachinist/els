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

#include "tft/tft.h"

typedef struct {
  uint16_t width;
  uint16_t height;
  const uint8_t *data;
} tft_image_t;

void tft_image_draw(const tft_device_t *, uint16_t x, uint16_t y, const tft_image_t *, tft_rgb_t);

#ifdef __cplusplus
}
#endif

