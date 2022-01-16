#include "fonts.h"

const tft_font_t noto_sans_mono_bold_arrows_24 = {
  .size = 24,
  .yoffset = 18,
  .glyph_count = 4,
  .glyphs = (const tft_font_glyph_t[]){
     {
       .character = 0x41,
       .width     = 15,
       .height    = 7,
       .xoffset   = 2,
       .yoffset   = 26,
       .xadvance  = 19,
       .data      = (const uint8_t[]){
         0x10, 0x00, 0x60, 0x01, 0xc0, 0x07, 0xff, 0xf7, 0x00, 0x06,
         0x00, 0x04, 0x00, 0x00
       }
     },
     {
       .character = 0x42,
       .width     = 7,
       .height    = 19,
       .xoffset   = 6,
       .yoffset   = 17,
       .xadvance  = 19,
       .data      = (const uint8_t[]){
         0x10, 0x71, 0xf7, 0xf1, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
         0x81, 0x02, 0x04, 0x08, 0x10, 0x20, 0x08
       }
     },
     {
       .character = 0x43,
       .width     = 15,
       .height    = 7,
       .xoffset   = 2,
       .yoffset   = 26,
       .xadvance  = 19,
       .data      = (const uint8_t[]){
         0x00, 0x10, 0x00, 0x30, 0x00, 0x77, 0xff, 0xf0, 0x01, 0xc0,
         0x03, 0x00, 0x04, 0x00
       }
     },
     {
       .character = 0x44,
       .width     = 7,
       .height    = 19,
       .xoffset   = 6,
       .yoffset   = 17,
       .xadvance  = 19,
       .data      = (const uint8_t[]){
         0x10, 0x20, 0x40, 0x81, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
         0x81, 0x02, 0x04, 0x7f, 0x7c, 0x70, 0x08
       }
     }
  }
};
