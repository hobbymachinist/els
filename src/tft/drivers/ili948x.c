#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "tft/tft.h"
#include "tft/tft_platform.h"

#define swap16(a, b) do { \
                       a = a ^ b; \
                       b = a ^ b; \
                       a = a ^ b; \
                     } while (0)

//
// timing assumptions are based on main clock running at 180MHz
//
static void tft_write_bus(const tft_device_t *tft, uint8_t data) {
  tft_gpio_clear(tft->ili948x.data.port, tft->ili948x.data.pin);
  tft_gpio_set(tft->ili948x.data.port, data);

  // write strobe.
  //
  // datasheet for 9481 says twrl = 25ns, twrh = 30ns, but twrl=10n seems enough.
  //
  tft_gpio_clear(tft->ili948x.wr.port, tft->ili948x.wr.pin);
  __asm("nop");
  __asm("nop");
  //
  // datasheet for 9486 & 9488 say twrl = 15ns, twrh = 15ns. can't get to work when
  // twrl < 20ns.
  //
  #if defined(TFT_ILI9486) || defined(TFT_ILI9488)
  __asm("nop");
  __asm("nop");
  __asm("nop");
  #endif
  // twrh ~ 15ms, BX LR is about 4 cycles ~ 20ns seems enough for ILI9481.
  tft_gpio_set(tft->ili948x.wr.port, tft->ili948x.wr.pin);
  #if defined(TFT_ILI9486) || defined(TFT_ILI9488)
  __asm("nop");
  __asm("nop");
  __asm("nop");
  #endif
}

static uint8_t tft_read_bus(const tft_device_t *tft) {
  uint8_t data;

  tft_gpio_set(tft->ili948x.rd.port, tft->ili948x.rd.pin);
  tft_gpio_clear(tft->ili948x.rd.port, tft->ili948x.rd.pin);
  tft_delay_microseconds(1);
  data = tft_gpio_get(tft->ili948x.data.port, tft->ili948x.data.pin) & 0xFF;
  tft_gpio_set(tft->ili948x.rd.port, tft->ili948x.rd.pin);
  return data;
}

static uint8_t tft_read_data8(const tft_device_t *tft) {
  return tft_read_bus(tft);
}

static uint16_t tft_read_data16(const tft_device_t *tft) {
  uint16_t msb, lsb;
  msb = tft_read_data8(tft);
  lsb = tft_read_data8(tft);
  return (msb << 8) | lsb;
}

static void tft_write_data8(const tft_device_t *tft, uint8_t data) {
  tft_write_bus(tft, data);
}

static void tft_write_data16(const tft_device_t *tft, uint16_t data) {
  tft_write_bus(tft, data >> 8);
  tft_write_bus(tft, data & 0xff);
}

static void tft_write_comm8(const tft_device_t *tft, uint8_t comm) {
  tft_gpio_clear(tft->ili948x.rs.port, tft->ili948x.rs.pin);
  tft_write_bus(tft, comm);
  tft_gpio_set(tft->ili948x.rs.port, tft->ili948x.rs.pin);
}

static void tft_write_comm16(const tft_device_t *tft, uint16_t comm) {
  tft_gpio_clear(tft->ili948x.rs.port, tft->ili948x.rs.pin);
  tft_write_bus(tft, comm >> 8);
  tft_write_bus(tft, comm & 0xff);
  tft_gpio_set(tft->ili948x.rs.port, tft->ili948x.rs.pin);
}

static void tft_write_comm_data(const tft_device_t *tft, uint16_t comm, uint16_t data) {
  tft_gpio_clear(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  tft_write_comm16(tft, comm);
  tft_write_data16(tft, data);
  tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);
}

static uint16_t tft_read_register(const tft_device_t *tft, uint16_t reg, int index) {
  uint16_t ret = 0, high, low;

  tft_gpio_clear(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  tft_write_comm16(tft, reg);
  tft_delay_microseconds(1);
  tft_gpio_mode_input(tft->ili948x.data.port, tft->ili948x.data.pin);

  do {
    high = tft_read_data8(tft);
    low  = tft_read_data8(tft);
    ret  = (high << 8 | low);
  } while (--index >= 0);

  tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  tft_gpio_mode_output(tft->ili948x.data.port, tft->ili948x.data.pin);

  return ret;
}

static uint32_t tft_read_register32(const tft_device_t *tft, uint16_t reg) {
  uint32_t h = tft_read_register(tft, reg, 0);
  uint32_t l = tft_read_register(tft, reg, 1);

  return (h << 16) | l;
}

static uint32_t tft_read_register40(const tft_device_t *tft, uint16_t reg) {
  uint32_t h = tft_read_register(tft, reg, 0);
  uint16_t m = tft_read_register(tft, reg, 1);
  uint16_t l = tft_read_register(tft, reg, 2);

  return (h << 24) | (m << 8) | (l >> 8);
}

static void tft_init_ili9481(const tft_device_t *tft) {
  // ILI9481
  // uint32_t r40 = tft_read_register40(tft, 0xBF);
  // printf("ID = 0x%04x\n", (uint16_t)(r40 & 0xFFFF));

  // soft reset
  tft_write_comm8(tft, 0x01);
  tft_delay_milliseconds(25);

  // display off
  tft_write_comm8(tft, 0x28);

  // pixel format, 16bpp
  tft_write_comm8(tft, 0x3A);
  tft_write_data8(tft, 0x55);

  // unlock E0 F0
  tft_write_comm_data(tft, 0xB0, 0x0000);

  // Frame Memory, interface [02 00 00 00]
  tft_write_comm8(tft, 0xB3);
  tft_write_data8(tft, 0x02);
  tft_write_data8(tft, 0x00);
  tft_write_data8(tft, 0x00);
  tft_write_data8(tft, 0x00);

  // Frame mode [00]
  tft_write_comm8(tft, 0xB4);
  tft_write_data8(tft, 0x00);

  // Set Power [00 43 18] x1.00, x6, x3
  tft_write_comm8(tft, 0xD0);
  tft_write_data8(tft, 0x07);
  tft_write_data8(tft, 0x42);
  tft_write_data8(tft, 0x17);

  // Set VCOM  [00 00 00] x0.72, x1.02
  tft_write_comm8(tft, 0xD1);
  tft_write_data8(tft, 0x00);
  tft_write_data8(tft, 0x07);
  tft_write_data8(tft, 0x10);

  // Set Power for Normal Mode [01 22]
  tft_write_comm8(tft, 0xD2);
  tft_write_data8(tft, 0x01);
  tft_write_data8(tft, 0x02);

  // Set Power for Partial Mode [01 22]
  tft_write_comm8(tft, 0xD3);
  tft_write_data8(tft, 0x01);
  tft_write_data8(tft, 0x02);

  // Set Power for Idle Mode [01 22]
  tft_write_comm8(tft, 0xD4);
  tft_write_data8(tft, 0x01);
  tft_write_data8(tft, 0x02);

  // Panel Driving BGR for 1581 [10 3B 00 02 11]
  tft_write_comm8(tft, 0xC0);
  tft_write_data8(tft, 0x12);
  tft_write_data8(tft, 0x3B);
  tft_write_data8(tft, 0x00);
  tft_write_data8(tft, 0x02);
  tft_write_data8(tft, 0x11);

  // Display Timing Normal [10 10 88]
  tft_write_comm8(tft, 0xC1);
  tft_write_data8(tft, 0x10);
  tft_write_data8(tft, 0x10);
  tft_write_data8(tft, 0x88);

  // Frame Rate [03]
  tft_write_comm8(tft, 0xC5);
  tft_write_data8(tft, 0x03);

  // Interface Control [02]
  tft_write_comm8(tft, 0xC6);
  tft_write_data8(tft, 0x02);
  tft_write_comm8(tft, 0xC8);
  tft_write_data8(tft, 0x00);
  tft_write_data8(tft, 0x32);
  tft_write_data8(tft, 0x36);
  tft_write_data8(tft, 0x45);
  tft_write_data8(tft, 0x06);
  tft_write_data8(tft, 0x16);
  tft_write_data8(tft, 0x37);
  tft_write_data8(tft, 0x75);
  tft_write_data8(tft, 0x77);
  tft_write_data8(tft, 0x54);
  tft_write_data8(tft, 0x0C);
  tft_write_data8(tft, 0x00);

  // Panel Control [00]
  tft_write_comm8(tft, 0xCC);
  tft_write_data8(tft, 0x00);

  // sleep out
  tft_write_comm8(tft, 0x11);
  tft_delay_milliseconds(25);

  // display on
  tft_write_comm8(tft, 0x29);
}

static void tft_init_ili9488(const tft_device_t *tft) {
  // ILI9486, ILI9488
  // uint32_t r32 = tft_read_register32(tft, 0xD3);
  // printf("ID = 0x%04x\n", (uint16_t)(r32 & 0xFFFF));

  // soft reset
  tft_write_comm8(tft, 0x01);
  tft_delay_milliseconds(25);

  // display off
  tft_write_comm8(tft, 0x28);

  // power control 1, v_reg1_out = 5v, v_reg2_out = 5v
  //
  // controls reg. voltage for +ve and -ve gamma correction.
  //
  tft_write_comm8(tft, 0xC0);
  tft_write_data8(tft, 0x17);
  tft_write_data8(tft, 0x17);
  tft_delay_milliseconds(5);

  // power control 2, v_gh, v_gl
  tft_write_comm8(tft, 0xC1);
  tft_write_data8(tft, 0x43);
  tft_delay_milliseconds(5);

  // vcomm
  tft_write_comm8(tft, 0xC5);
  tft_write_data8(tft, 0x00);
  tft_write_data8(tft, 0x30);
  tft_write_data8(tft, 0x00);
  tft_write_data8(tft, 0x00);
  tft_delay_milliseconds(5);

  // pixel format, 16bpp
  tft_write_comm8(tft, 0x3A);
  tft_write_data8(tft, 0x55);

  // Uncomment and customise for your TFT if necessary.
  #if 0
  // PGAMCTRL (Positive Gamma Control)
  tft_write_comm8(tft, 0xE0);
  tft_write_data8(tft, 0x00);
  tft_write_data8(tft, 0x03);
  tft_write_data8(tft, 0x09);
  tft_write_data8(tft, 0x08);
  tft_write_data8(tft, 0x16);
  tft_write_data8(tft, 0x0A);
  tft_write_data8(tft, 0x3F);
  tft_write_data8(tft, 0x78);
  tft_write_data8(tft, 0x4C);
  tft_write_data8(tft, 0x09);
  tft_write_data8(tft, 0x0A);
  tft_write_data8(tft, 0x08);
  tft_write_data8(tft, 0x16);
  tft_write_data8(tft, 0x1A);
  tft_write_data8(tft, 0x0F);

  // NGAMCTRL (Negative Gamma Control)
  tft_write_comm8(tft, 0xE1);
  tft_write_data8(tft, 0x00);
  tft_write_data8(tft, 0x16);
  tft_write_data8(tft, 0x19);
  tft_write_data8(tft, 0x03);
  tft_write_data8(tft, 0x0F);
  tft_write_data8(tft, 0x05);
  tft_write_data8(tft, 0x32);
  tft_write_data8(tft, 0x45);
  tft_write_data8(tft, 0x46);
  tft_write_data8(tft, 0x04);
  tft_write_data8(tft, 0x0E);
  tft_write_data8(tft, 0x0D);
  tft_write_data8(tft, 0x35);
  tft_write_data8(tft, 0x37);
  tft_write_data8(tft, 0x0F);
  #endif

  // frame rate, 30fps
  tft_write_comm8(tft, 0xB1);
  tft_write_data8(tft, 0x10);

  // display inversion, 2-dot
  tft_write_comm8(tft, 0xB4);
  tft_write_data8(tft, 0x02);

  // sleep out
  tft_write_comm8(tft, 0x11);
  tft_delay_milliseconds(25);

  // display on
  tft_write_comm8(tft, 0x29);
}

void tft_init(const tft_device_t *tft) {
  // init pins
  tft_gpio_mode_output(tft->ili948x.rd.port, tft->ili948x.rd.pin);
  tft_gpio_mode_output(tft->ili948x.wr.port, tft->ili948x.wr.pin);
  tft_gpio_mode_output(tft->ili948x.rs.port, tft->ili948x.rs.pin);
  tft_gpio_mode_output(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  tft_gpio_mode_output(tft->ili948x.rst.port, tft->ili948x.rst.pin);

  // data out by default
  tft_gpio_mode_output(tft->ili948x.data.port, tft->ili948x.data.pin);

  // idle
  tft_gpio_set(tft->ili948x.rd.port, tft->ili948x.rd.pin);
  tft_gpio_set(tft->ili948x.wr.port, tft->ili948x.wr.pin);
  tft_gpio_set(tft->ili948x.rs.port, tft->ili948x.rs.pin);
  tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  tft_gpio_set(tft->ili948x.rst.port, tft->ili948x.rst.pin);
  tft_delay_milliseconds(1);

  tft_gpio_clear(tft->ili948x.rst.port, tft->ili948x.rst.pin);
  tft_delay_milliseconds(1);
  tft_gpio_set(tft->ili948x.rst.port, tft->ili948x.rst.pin);
  tft_delay_milliseconds(1);

  tft_gpio_clear(tft->ili948x.cs.port, tft->ili948x.cs.pin);

  #if defined(TFT_ILI9481)
  tft_init_ili9481(tft);
  #endif

  #if defined(TFT_ILI9486) || defined(TFT_ILI9488)
  tft_init_ili9488(tft);
  #endif

  tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);
}

void tft_set_window(const tft_device_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  // allows you to use clipping within a flood fill.
  bool reset = false;
  if (tft_gpio_get(tft->ili948x.cs.port, tft->ili948x.cs.pin)) {
    tft_gpio_clear(tft->ili948x.cs.port, tft->ili948x.cs.pin);
    reset = true;
  }

  tft_write_comm8(tft, 0x2A);
  tft_write_data16(tft, x);
  tft_write_data16(tft, x + w - 1);
  tft_write_comm8(tft, 0x2B);
  tft_write_data16(tft, y);
  tft_write_data16(tft, y + h - 1);
  tft_write_comm8(tft, 0x2C);

  if (reset) {
    tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  }
}


/****************************************************************************

ILITEK datasheet, v0.27 Page 78

B0 Vertical Flip
B1 Horizontal Flip
B2 Display data latch data order - fixed at 0
B3 RGB/BGR Order
B4 Vertical Order
B5 Page/Column Selection
B6 Column Address Order
B7 Page Address Order

****************************************************************************/

void tft_rotate(const tft_device_t *tft, tft_rotation_t rotation) {
  uint8_t data = 0;

  switch (rotation) {
    case ROTATE_NONE: // PORTRAIT:
        data = 0x02;  // B7 = 0, B6 = 0, B5 = 0, B4 = 0, B3 = 0, B2 = 0, B1 = 1, B0 = 0
        break;
    case ROTATE_90:   // LANDSCAPE: 90 degrees
        data = 0x20;  // B7 = 0, B6 = 0, B5 = 1, B4 = 0, B3 = 0, B2 = 0, B1 = 0, B0 = 0
        break;
    case ROTATE_180:  // PORTRAIT REVERSE: 180 degrees
        data = 0x01;  // B7 = 0, B6 = 0, B5 = 0, B4 = 0, B3 = 0, B2 = 0, B1 = 0, B0 = 1
        break;
    case ROTATE_270:  // LANDSCAPE REVERSE: 270 degrees
        data = 0x23;  // B7 = 0, B6 = 0, B5 = 1, B4 = 0, B3 = 0, B2 = 0, B1 = 1, B0 = 1
        break;
  }

  tft_gpio_clear(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  tft_write_comm8(tft, 0x36);
  tft_write_data8(tft, data);
  tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);
}

void tft_invert(const tft_device_t *tft, bool invert) {
  tft_gpio_clear(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  tft_write_comm8(tft, invert ? 0x21 : 20);
  tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);

  tft_write_comm_data(tft, 0x61, invert);
}

void tft_pixel_set(const tft_device_t *tft, uint16_t x, uint16_t y, tft_rgb_t color) {
  tft_gpio_clear(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  // write memory
  tft_write_comm8(tft, 0x2C);
  tft_set_window(tft, x, y, 1, 1);

  // 5:6:5 16-bit color space.
  tft_write_data16(
    tft,
    ((uint16_t)color.b & 0x1f) << 11 |
    ((uint16_t)color.g & 0x3f) << 5  |
    ((uint16_t)color.r & 0x1f)
  );

  tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);
}

void tft_pixel_get(const tft_device_t *tft, uint16_t x, uint16_t y, tft_rgb_t *color) {
  tft_gpio_clear(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  // read memory
  tft_set_window(tft, x, y, 1, 1);
  tft_write_comm8(tft, 0x2E);

  tft_gpio_mode_input(tft->ili948x.data.port, tft->ili948x.data.pin);
  // dummy read.
  tft_read_data8(tft);
  // 5:6:5 16-bit color space.
  uint16_t data = tft_read_data16(tft);
  tft_gpio_mode_output(tft->ili948x.data.port, tft->ili948x.data.pin);
  tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);

  color->r = (data >>  0) & 0x1f;
  color->g = (data >>  5) & 0x3f;
  color->b = (data >> 11) & 0x1f;
}

uint16_t tft_rgb_to_rgb16(tft_rgb_t color) {
  return ((uint16_t)color.b & 0x1f) << 11 |
         ((uint16_t)color.g & 0x3f) << 5  |
         ((uint16_t)color.r & 0x1f);
}

void tft_flood_start(const tft_device_t *tft) {
  tft_gpio_clear(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  // write memory
  tft_write_comm8(tft, 0x2C);
}

void tft_flood_write_rgb16(const tft_device_t *tft, uint16_t data) {
  tft_write_data16(tft, data);
}

void tft_flood_finish(const tft_device_t *tft) {
  tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);
}

void tft_rectangle(const tft_device_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, tft_rgb_t color) {
  tft_filled_rectangle(tft, x,     y,     w, 1, color);
  tft_filled_rectangle(tft, x,     y + h, w, 1, color);
  tft_filled_rectangle(tft, x,     y,     1, h, color);
  tft_filled_rectangle(tft, x + w, y,     1, h, color);
}

void tft_filled_rectangle(const tft_device_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, tft_rgb_t color) {
  // 5:6:5 16-bit color space.
  uint16_t cvalue = ((uint16_t)color.b & 0x1f) << 11 |
                    ((uint16_t)color.g & 0x3f) << 5  |
                    ((uint16_t)color.r & 0x1f);

  uint8_t cvalue_hi = cvalue >> 8,
          cvalue_lo = cvalue & 0xff;

  uint32_t count = w * h - 1;

  tft_gpio_clear(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  tft_set_window(tft, x, y, w, h);

  // write memory
  tft_write_comm8(tft, 0x2C);

  // first pixel.
  tft_write_bus(tft, cvalue_hi);
  tft_write_bus(tft, cvalue_lo);

  if (cvalue_hi == cvalue_lo) {
    // write strobe, data already set up above.
    for (uint32_t i = 0; i < count; i++) {
      // cvalue_hi
      //
      // wr strobe
      tft_gpio_clear(tft->ili948x.wr.port, tft->ili948x.wr.pin);
      //
      // datasheet for 9486 & 9488 say twrl = 15ns, twrh = 15ns and seem strict about it.
      //
      // ILI9481 somehow manages without it, which is a mystery.
      //
      #if defined(TFT_ILI9486) || defined(TFT_ILI9488)
      __asm("nop");
      __asm("nop");
      __asm("nop");
      __asm("nop");
      #endif
      tft_gpio_set(tft->ili948x.wr.port, tft->ili948x.wr.pin);

      // cvalue_lo
      tft_gpio_clear(tft->ili948x.wr.port, tft->ili948x.wr.pin);
      //
      // datasheet for 9486 & 9488 say twrl = 15ns, twrh = 15ns and seem strict about it.
      //
      // ILI9481 somehow manages without it, which is a mystery.
      //
      #if defined(TFT_ILI9486) || defined(TFT_ILI9488)
      __asm("nop");
      __asm("nop");
      __asm("nop");
      __asm("nop");
      #endif
      tft_gpio_set(tft->ili948x.wr.port, tft->ili948x.wr.pin);
    }
  }
  else {
    for (uint32_t i = 0; i < count; i++) {
      tft_write_bus(tft, cvalue_hi);
      tft_write_bus(tft, cvalue_lo);
    }
  }

  tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);
}

void tft_line(const tft_device_t *tft, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, tft_rgb_t color) {
  uint16_t cvalue = ((uint16_t)color.b & 0x1f) << 11 |
                    ((uint16_t)color.g & 0x3f) << 5  |
                    ((uint16_t)color.r & 0x1f);

  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if (steep) {
    swap16(x0, y0);
    swap16(x1, y1);
  }

  if (x0 > x1) {
    swap16(x0, x1);
    swap16(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  }
  else {
    ystep = -1;
  }

  tft_gpio_clear(tft->ili948x.cs.port, tft->ili948x.cs.pin);
  for (; x0 <= x1; x0++) {
    tft_write_comm8(tft, 0x2C);
    if (steep)
      tft_set_window(tft, y0, x0, 1, 1);
    else
      tft_set_window(tft, x0, y0, 1, 1);
    tft_write_data16(tft, cvalue);

    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
  tft_gpio_set(tft->ili948x.cs.port, tft->ili948x.cs.pin);
}

void tft_triangle(const tft_device_t *tft,
  uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
  tft_rgb_t color) {
  tft_line(tft, x1, y1, x2, y2, color);
  tft_line(tft, x2, y2, x3, y3, color);
  tft_line(tft, x3, y3, x1, y1, color);
}

// Pinched from Adafruit GFX
//
// Explained here: https://gabrielgambetta.com/computer-graphics-from-scratch/07-filled-triangles.html
//
void tft_filled_triangle(const tft_device_t *tft,
  uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
  tft_rgb_t color) {

  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap16(y0, y1);
    swap16(x0, x1);
  }
  if (y1 > y2) {
    swap16(y2, y1);
    swap16(x2, x1);
  }
  if (y0 > y1) {
    swap16(y0, y1);
    swap16(x0, x1);
  }

  // Handle awkward all-on-same-line case as its own thing
  if (y0 == y2) {
    a = b = x0;
    if (x1 < a)
      a = x1;
    else if (x1 > b)
      b = x1;
    if (x2 < a)
      a = x2;
    else if (x2 > b)
      b = x2;
    tft_line(tft, a, y0, b + 1, y0, color);
    return;
  }

  int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
  int32_t sa = 0, sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2)
    last = y1;                  // Include y1 scanline
  else
    last = y1 - 1;              // Skip it

  for (y = y0; y <= last; y++) {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
       a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
       b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
     */
    if (a > b)
      swap16(a, b);
    tft_line(tft, a, y, b + 1, y, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for (; y <= y2; y++) {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
       a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
       b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
     */
    if (a > b)
      swap16(a, b);
    tft_line(tft, a, y, b + 1, y, color);
  }
}

// Pinched from Adafruit GFX
//
void tft_circle_rgb(const tft_device_t * tft, uint16_t x0, uint16_t y0, uint16_t r, tft_rgb_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  tft_pixel_set(tft, x0, y0 + r, color);
  tft_pixel_set(tft, x0, y0 - r, color);
  tft_pixel_set(tft, x0 + r, y0, color);
  tft_pixel_set(tft, x0 - r, y0, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    tft_pixel_set(tft, x0 + x, y0 + y, color);
    tft_pixel_set(tft, x0 - x, y0 + y, color);
    tft_pixel_set(tft, x0 + x, y0 - y, color);
    tft_pixel_set(tft, x0 - x, y0 - y, color);
    tft_pixel_set(tft, x0 + y, y0 + x, color);
    tft_pixel_set(tft, x0 - y, y0 + x, color);
    tft_pixel_set(tft, x0 + y, y0 - x, color);
    tft_pixel_set(tft, x0 - y, y0 - x, color);
  }
}

// Pinched from Adafruit GFX
//
void tft_filled_circle(const tft_device_t * tft, uint16_t x0, uint16_t y0, uint16_t r, tft_rgb_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    tft_line(tft, x0, y0 - r, x0, y0 + r, color);
    tft_line(tft, x0 + x, y0 - y, x0 + x, y0 + y - 1, color);
    tft_line(tft, x0 + y, y0 - x, x0 + y, y0 + x - 1, color);
    tft_line(tft, x0 - x, y0 - y, x0 - x, y0 + y - 1, color);
    tft_line(tft, x0 - y, y0 - x, x0 - y, y0 + x - 1, color);
  }
}

// https://forum.arduino.cc/t/adafruit_gfx-fillarc/397741/6
void tft_filled_arc(const tft_device_t *tft,
  uint16_t x, uint16_t y, uint16_t rx, uint16_t ry,
  uint16_t start_angle, uint16_t segments, uint16_t width,
  tft_rgb_t color) {

  uint8_t seg = 3; // Segments are 3 degrees wide = 120 segments for 360 degrees
  uint8_t inc = 3; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Calculate first pair of coordinates for segment start
  float sx = cos(((start_angle - 90) * M_PI) / 180);
  float sy = sin(((start_angle - 90) * M_PI) / 180);
  uint16_t x0 = sx * (rx - width) + x;
  uint16_t y0 = sy * (ry - width) + y;
  uint16_t x1 = sx * rx + x;
  uint16_t y1 = sy * ry + y;

  // Draw colour blocks every inc degrees
  for (uint16_t i = start_angle; i < start_angle + seg * segments; i += inc) {
    // Calculate pair of coordinates for segment end
    float sx2 = cos(((i + seg - 90) * M_PI) / 180);
    float sy2 = sin(((i + seg - 90) * M_PI) / 180);
    uint16_t x2 = sx2 * (rx - width) + x;
    uint16_t y2 = sy2 * (ry - width) + y;
    uint16_t x3 = sx2 * rx + x;
    uint16_t y3 = sy2 * ry + y;

    tft_filled_triangle(tft, x0, y0, x1, y1, x2, y2, color);
    tft_filled_triangle(tft, x1, y1, x2, y2, x3, y3, color);

    // Copy segment end to segment start for next segment
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;
  }
}


