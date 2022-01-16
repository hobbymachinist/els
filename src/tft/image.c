#include "tft/image.h"

void tft_image_draw(const tft_device_t *tft, uint16_t x, uint16_t y, const tft_image_t *img, tft_rgb_t color) {
  size_t offset = 0;
  uint16_t bits = 0, data = 0;
  uint16_t xpos = x, ypos = y;
  for (size_t h = 0; h < img->height; h++, ypos++) {
    xpos = x;
    for (size_t w = 0; w < img->width; w++, xpos++) {
      if (bits == 0) {
        bits = 8;
        data = img->data[offset++];
      }
      if (data & 0x80)
        tft_pixel_set(tft, xpos, ypos, color);

      data <<= 1;
      bits--;
    }
  }
}
