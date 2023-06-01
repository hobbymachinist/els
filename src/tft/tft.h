#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// colorspaces
//
// ILI9481, ILI9486, ILI9488 16-bit colorspace
#if defined(TFT_ILI9481) || defined(TFT_ILI9486) || defined(TFT_ILI9488)
  #define ILI948x_RMAX 31
  #define ILI948x_GMAX 63
  #define ILI948x_BMAX 31

  #define TFT_COLOR_RED       (tft_rgb_t){.r = ILI948x_RMAX}
  #define TFT_COLOR_GREEN     (tft_rgb_t){.g = ILI948x_GMAX}
  #define TFT_COLOR_BLUE      (tft_rgb_t){.b = ILI948x_BMAX}
  #define TFT_COLOR_YELLOW    (tft_rgb_t){.r = ILI948x_BMAX, .g = ILI948x_GMAX}
  #define TFT_COLOR_CYAN      (tft_rgb_t){.g = ILI948x_GMAX, .b = ILI948x_BMAX}
  #define TFT_COLOR_MAGENTA   (tft_rgb_t){.r = ILI948x_RMAX, .b = ILI948x_BMAX}
  #define TFT_COLOR_WHITE     (tft_rgb_t){.r = ILI948x_RMAX, .g = ILI948x_GMAX, .b = ILI948x_BMAX}
  #define TFT_COLOR_ORANGE    (tft_rgb_t){.r = ILI948x_RMAX, .g = 20}
  #define TFT_COLOR_BLACK     (tft_rgb_t){.r = 0,  .g = 0, .b = 0}
  #define TFT_COLOR_CERULEAN  (tft_rgb_t){.r = 0,  .g = 30, .b = 20}
  #define TFT_COLOR_DIANNE    (tft_rgb_t){.r = 2,  .g = 18, .b = 12}
  #define TFT_COLOR_FROG      (tft_rgb_t){.r = 2,  .g = 25, .b = 2}
  #define TFT_COLOR_GRAY      (tft_rgb_t){.r = 7,  .g = 14, .b = 7}
  #define TFT_COLOR_LITEGRAY  (tft_rgb_t){.r = 14, .g = 28, .b = 14}
  #define TFT_COLOR_FOREST    (tft_rgb_t){.r = 1,  .g = 25, .b = 4}
  #define TFT_COLOR_TURF      (tft_rgb_t){.r = 5,  .g = 13, .b = 3}
  #define TFT_COLOR_BLUE1     (tft_rgb_t){.r = 0,  .g = 4,  .b = 5}
  #define TFT_COLOR_BLUE2     (tft_rgb_t){.r = 0,  .g = 16, .b = 8}

  #define TFT_COLOR_BGCOLOR1  (tft_rgb_t){.r = 10, .g = 40, .b = 30}
  #define TFT_COLOR_BGCOLOR2  (tft_rgb_t){.r = 4,  .g = 30, .b = 8}
  #define TFT_COLOR_BGCOLOR3  (tft_rgb_t){.r = 27, .g = 10}
#endif

typedef struct {
  uint32_t port;
  uint32_t pin;
} tft_gpio_t;

typedef enum {
  ROTATE_NONE = 0,
  ROTATE_90   = 1,
  ROTATE_180  = 2,
  ROTATE_270  = 3
} tft_rotation_t;

typedef struct {
  uint8_t r, g, b;
} tft_rgb_t;

typedef union {
  struct {
    tft_gpio_t cs;
    tft_gpio_t rs;
    tft_gpio_t wr;
    tft_gpio_t rd;
    tft_gpio_t rst;
    tft_gpio_t data; // pin is ignored, uses pin 0 - pin 7.
  } ili948x;
} tft_device_t;

// ------------------------------------------------------------------------------------------------------------------
// init & config, device specific implemented by driver.
// ------------------------------------------------------------------------------------------------------------------
void tft_init(const tft_device_t *);
void tft_rotate(const tft_device_t*, tft_rotation_t);
void tft_invert(const tft_device_t*, bool);
void tft_set_window(const tft_device_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

// ------------------------------------------------------------------------------------------------------------------
// drawing, device specific implemented by driver.
// ------------------------------------------------------------------------------------------------------------------
void tft_pixel_set(const tft_device_t*, uint16_t x, uint16_t y, tft_rgb_t);
void tft_pixel_get(const tft_device_t*, uint16_t x, uint16_t y, tft_rgb_t*);
void tft_rectangle(const tft_device_t *, uint16_t x, uint16_t y, uint16_t w, uint16_t h, tft_rgb_t color);
void tft_filled_rectangle(const tft_device_t *, uint16_t x, uint16_t y, uint16_t w, uint16_t h, tft_rgb_t color);
void tft_line(const tft_device_t *, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, tft_rgb_t color);
void tft_circle_rgb(const tft_device_t *, uint16_t x, uint16_t y, uint16_t r, tft_rgb_t color);
void tft_filled_circle(const tft_device_t *, uint16_t x, uint16_t y, uint16_t r, tft_rgb_t color);

void tft_triangle(const tft_device_t*,
  uint16_t x1, uint16_t y1,
  uint16_t x2, uint16_t y2,
  uint16_t x3, uint16_t y3,
  tft_rgb_t color);

void tft_filled_triangle(const tft_device_t*,
  uint16_t x1, uint16_t y1,
  uint16_t x2, uint16_t y2,
  uint16_t x3, uint16_t y3,
  tft_rgb_t color);

// draws 3° arc segments.
void tft_filled_arc(const tft_device_t *tft,
  uint16_t x, uint16_t y, uint16_t rx, uint16_t ry,
  uint16_t start_angle, uint16_t segments, uint16_t width,
  tft_rgb_t color);

// ------------------------------------------------------------------------------------------------------------------
// fast flood fill, device specific implemented by driver.
// ------------------------------------------------------------------------------------------------------------------
void tft_flood_start(const tft_device_t *tft);
void tft_flood_finish(const tft_device_t *tft);
void tft_flood_write_rgb16(const tft_device_t *tft, uint16_t data);
// ------------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------------
// color space utils
// ------------------------------------------------------------------------------------------------------------------
uint16_t tft_rgb_to_rgb16(tft_rgb_t color);

// ------------------------------------------------------------------------------------------------------------------
// gpio utils - used by drivers, platform & bsp specific.
// ------------------------------------------------------------------------------------------------------------------
void     tft_gpio_set(uint32_t port, uint32_t pins);
void     tft_gpio_clear(uint32_t port, uint32_t pins);
uint32_t tft_gpio_get(uint32_t port, uint32_t pins);
void     tft_gpio_mode_input(uint32_t port, uint32_t pins);
void     tft_gpio_mode_output(uint32_t port, uint32_t pins);

#ifdef __cplusplus
}
#endif

