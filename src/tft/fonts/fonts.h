#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../tft.h"

typedef struct {
  const char character;
  const uint8_t width;
  const uint8_t height;
  const int8_t  xoffset;
  const int8_t  yoffset;
  const uint8_t xadvance;
  const uint8_t * const data;
} tft_font_glyph_t;

typedef struct {
  const uint16_t glyph_count;
  const uint8_t  size;
  const uint8_t  yoffset; // default for space
  const tft_font_glyph_t *glyphs;
} tft_font_t;

void tft_font_write(const tft_device_t *, uint16_t x, uint16_t y, const char *text,
                    const tft_font_t *, tft_rgb_t color);

void tft_font_write_bg(const tft_device_t *, uint16_t x, uint16_t y, const char *text,
                       const tft_font_t *, tft_rgb_t color, tft_rgb_t bgcolor);

void tft_font_write_bg_xadv(const tft_device_t * tft, uint16_t x, uint16_t y,
                    const char *text,
                    const tft_font_t *font, tft_rgb_t color, tft_rgb_t bgcolor, uint16_t xadvance);


#ifdef __cplusplus
}
#endif
