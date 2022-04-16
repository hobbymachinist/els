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
// ILI9481
#define ILI9481_RMAX 31
#define ILI9481_GMAX 63
#define ILI9481_BMAX 31

#define ILI9481_RED       (tft_rgb_t){.r = ILI9481_RMAX}
#define ILI9481_GREEN     (tft_rgb_t){.g = ILI9481_GMAX}
#define ILI9481_BLUE      (tft_rgb_t){.b = ILI9481_BMAX}
#define ILI9481_YELLOW    (tft_rgb_t){.r = ILI9481_BMAX, .g = ILI9481_GMAX}
#define ILI9481_CYAN      (tft_rgb_t){.g = ILI9481_GMAX, .b = ILI9481_BMAX}
#define ILI9481_MAGENTA   (tft_rgb_t){.r = ILI9481_RMAX, .b = ILI9481_BMAX}
#define ILI9481_WHITE     (tft_rgb_t){.r = ILI9481_RMAX, .g = ILI9481_GMAX, .b = ILI9481_BMAX}
#define ILI9481_ORANGE    (tft_rgb_t){.r = ILI9481_RMAX, .g = 20}
#define ILI9481_BLACK     (tft_rgb_t){.r = 0,  .g = 0, .b = 0}
#define ILI9481_CERULEAN  (tft_rgb_t){.r = 0,  .g = 30, .b = 20}
#define ILI9481_DIANNE    (tft_rgb_t){.r = 2,  .g = 18, .b = 12}
#define ILI9481_FROG      (tft_rgb_t){.r = 2,  .g = 25, .b = 2}
#define ILI9481_GRAY      (tft_rgb_t){.r = 7,  .g = 14, .b = 7}
#define ILI9481_LITEGRAY  (tft_rgb_t){.r = 14, .g = 28, .b = 14}
#define ILI9481_FOREST    (tft_rgb_t){.r = 1,  .g = 25, .b = 4}
#define ILI9481_TURF      (tft_rgb_t){.r = 5,  .g = 13, .b = 3}
#define ILI9481_BLUE1     (tft_rgb_t){.r = 0,  .g = 4,  .b = 5}
#define ILI9481_BLUE2     (tft_rgb_t){.r = 0,  .g = 16, .b = 8}

#define ILI9481_BGCOLOR1  (tft_rgb_t){.r = 10, .g = 40, .b = 30}
#define ILI9481_BGCOLOR2  (tft_rgb_t){.r = 4,  .g = 30, .b = 8}
#define ILI9481_BGCOLOR3  (tft_rgb_t){.r = 27, .g = 10}

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
  } ili9481;
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

