#include "fonts.h"

static const tft_font_glyph_t * find_glyph(const tft_font_t *font, char c) {
  for (size_t i = 0; i < font->glyph_count; i++) {
    if (font->glyphs[i].character == c)
      return &font->glyphs[i];
  }

  return NULL;
}

void tft_font_write_bg(const tft_device_t * tft, uint16_t x, uint16_t y,
                    const char *text,
                    const tft_font_t *font, tft_rgb_t color, tft_rgb_t bgcolor) {

  tft_font_write_bg_xadv(tft, x, y, text, font, color, bgcolor, font->glyphs[0].xadvance);
}


void tft_font_write_bg_xadv(const tft_device_t * tft, uint16_t x, uint16_t y,
                    const char *text,
                    const tft_font_t *font, tft_rgb_t color, tft_rgb_t bgcolor,
                    uint16_t xadvance) {

  uint16_t xcurr = x, xpos, ypos, rgb16 = tft_rgb_to_rgb16(color), bgrgb16 = tft_rgb_to_rgb16(bgcolor);

  tft_flood_start(tft);
  while(*text) {
    const tft_font_glyph_t *g = find_glyph(font, *text);
    if (g) {
      xpos   = xcurr + g->xoffset;
      ypos   = y + g->yoffset;
      xcurr += g->xadvance;

      size_t pixels = g->width * g->height;
      size_t bytes = pixels / 8, trailing_bits = pixels - (bytes * 8);

      tft_set_window(tft, xpos, ypos, g->width, g->height);

      uint8_t data, mask;
      for (size_t i = 0; i < bytes; i++) {
        data = g->data[i];
        tft_flood_write_rgb16(tft, data & 0x80 ? rgb16 : bgrgb16);
        data <<= 1;
        tft_flood_write_rgb16(tft, data & 0x80 ? rgb16 : bgrgb16);
        data <<= 1;
        tft_flood_write_rgb16(tft, data & 0x80 ? rgb16 : bgrgb16);
        data <<= 1;
        tft_flood_write_rgb16(tft, data & 0x80 ? rgb16 : bgrgb16);
        data <<= 1;
        tft_flood_write_rgb16(tft, data & 0x80 ? rgb16 : bgrgb16);
        data <<= 1;
        tft_flood_write_rgb16(tft, data & 0x80 ? rgb16 : bgrgb16);
        data <<= 1;
        tft_flood_write_rgb16(tft, data & 0x80 ? rgb16 : bgrgb16);
        data <<= 1;
        tft_flood_write_rgb16(tft, data & 0x80 ? rgb16 : bgrgb16);
      }

      if (trailing_bits > 0) {
        data = g->data[bytes];
        mask = 1 << (trailing_bits - 1);
        for (size_t i = 0; i < trailing_bits; i++) {
          tft_flood_write_rgb16(tft, data & mask ? rgb16 : bgrgb16);
          data <<= 1;
        }
      }
    }
    else if (*text == ' ') {
      tft_set_window(tft, xcurr, y + font->yoffset, xadvance, font->size);
      for (size_t n = 0; n < xadvance * font->size; n++)
        tft_flood_write_rgb16(tft, bgrgb16);
      xcurr += xadvance;
    }
    text++;
  };
  tft_flood_finish(tft);
}

void tft_font_write(const tft_device_t * tft, uint16_t x, uint16_t y,
                    const char *text,
                    const tft_font_t *font, tft_rgb_t color) {

  uint16_t xcurr = x, xpos, ypos, rgb16 = tft_rgb_to_rgb16(color);

  while(*text) {
    const tft_font_glyph_t *g = find_glyph(font, *text);
    if (g) {
      xpos   = xcurr + g->xoffset;
      ypos   = y + g->yoffset;
      xcurr += g->xadvance;

      size_t pixels = g->width * g->height;
      size_t bytes = pixels / 8, trailing_bits = pixels - (bytes * 8);
      uint8_t data = 0, mask = 0x80;

      size_t bitpos = 0, bytepos = 0;

      for (uint16_t yp = ypos; yp < ypos + g->height; yp++) {
        for (uint16_t xp = xpos; xp < xpos + g->width; xp++) {
          if (bitpos % 8 == 0) {
            mask = bytepos > bytes ? (1 << trailing_bits) : 0x80;
            data = g->data[bytepos++];
          }

          if (data & mask)
            tft_pixel_set(tft, xp, yp, color);

          data <<= 1;
          bitpos++;
        }
      }
    }
    else if (*text == ' ') {
      xcurr += font->glyphs[0].xadvance;
    }
    text++;
  };
}

