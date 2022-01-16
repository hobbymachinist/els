#include "fonts.h"

const tft_font_t inconsolata_lgc_bold_14 = {
  .size = 14,
  .yoffset = 6,
  .glyph_count = 95,
  .glyphs = (const tft_font_glyph_t[]){
     {
       .character = 0x20,
       .width     = 1,
       .height    = 1,
       .xoffset   = 0,
       .yoffset   = 19,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x00
       }
     },
     {
       .character = 0x21,
       .width     = 3,
       .height    = 14,
       .xoffset   = 4,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xdf, 0xff, 0xb6, 0x40, 0x7f, 0x03
       }
     },
     {
       .character = 0x22,
       .width     = 6,
       .height    = 6,
       .xoffset   = 3,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xcf, 0x3c, 0xf3, 0xcf, 0x03
       }
     },
     {
       .character = 0x23,
       .width     = 11,
       .height    = 14,
       .xoffset   = 0,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x09, 0x81, 0x30, 0x66, 0x0c, 0xc7, 0xfe, 0xf3, 0xc6, 0x40,
         0xd8, 0x1f, 0xcf, 0xfb, 0xcc, 0x19, 0x83, 0x30, 0x66, 0x00
       }
     },
     {
       .character = 0x24,
       .width     = 9,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0c, 0x06, 0x0f, 0xcd, 0xb6, 0xc3, 0x61, 0xf0, 0x3e, 0x0f,
         0x86, 0xd3, 0x7d, 0xb7, 0xf0, 0xf0, 0x18
       }
     },
     {
       .character = 0x25,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x71, 0xbe, 0x69, 0xb2, 0x6c, 0xf6, 0x1b, 0x00, 0xc0, 0x60,
         0x1b, 0x8d, 0xf3, 0x4d, 0x93, 0x67, 0xf0, 0x0e
       }
     },
     {
       .character = 0x26,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3c, 0x1f, 0x86, 0x61, 0x98, 0x66, 0x0f, 0x03, 0x81, 0xf2,
         0xcc, 0xf1, 0xec, 0x3b, 0x1e, 0xff, 0xde, 0x02
       }
     },
     {
       .character = 0x27,
       .width     = 2,
       .height    = 6,
       .xoffset   = 5,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0x0f
       }
     },
     {
       .character = 0x28,
       .width     = 7,
       .height    = 18,
       .xoffset   = 3,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x04, 0x3c, 0xe3, 0x86, 0x1c, 0x30, 0x60, 0xc1, 0x83, 0x06,
         0x06, 0x0c, 0x1c, 0x1c, 0x1c, 0x06
       }
     },
     {
       .character = 0x29,
       .width     = 7,
       .height    = 18,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x40, 0xe0, 0xe0, 0xe0, 0xc1, 0xc1, 0x83, 0x06, 0x0c, 0x18,
         0x30, 0xe1, 0x87, 0x1c, 0x70, 0x30
       }
     },
     {
       .character = 0x2a,
       .width     = 9,
       .height    = 9,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0c, 0x06, 0x32, 0x7f, 0xf1, 0xc0, 0xf0, 0xd8, 0xc6, 0x20,
         0x00
       }
     },
     {
       .character = 0x2b,
       .width     = 9,
       .height    = 9,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0c, 0x06, 0x03, 0x1f, 0xff, 0xf8, 0x60, 0x30, 0x18, 0x0c,
         0x00
       }
     },
     {
       .character = 0x2c,
       .width     = 4,
       .height    = 6,
       .xoffset   = 3,
       .yoffset   = 18,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x67, 0x73, 0x64
       }
     },
     {
       .character = 0x2d,
       .width     = 9,
       .height    = 2,
       .xoffset   = 1,
       .yoffset   = 12,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0x03
       }
     },
     {
       .character = 0x2e,
       .width     = 3,
       .height    = 3,
       .xoffset   = 4,
       .yoffset   = 18,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0x00
       }
     },
     {
       .character = 0x2f,
       .width     = 9,
       .height    = 16,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x01, 0x01, 0xc0, 0xc0, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18,
         0x0c, 0x0c, 0x06, 0x06, 0x03, 0x03, 0x00, 0x00
       }
     },
     {
       .character = 0x30,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1c, 0x1f, 0x18, 0xcc, 0x7c, 0x3e, 0x3f, 0x37, 0xb3, 0xd1,
         0xf8, 0xd8, 0x6e, 0x63, 0xf0, 0x38
       }
     },
     {
       .character = 0x31,
       .width     = 5,
       .height    = 14,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1b, 0xfe, 0x31, 0x8c, 0x63, 0x18, 0xc6, 0x31, 0x23
       }
     },
     {
       .character = 0x32,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1e, 0x3f, 0x98, 0xe0, 0x30, 0x18, 0x1c, 0x0c, 0x0c, 0x0c,
         0x0c, 0x0c, 0x0c, 0x07, 0xff, 0x3f
       }
     },
     {
       .character = 0x33,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3c, 0x3f, 0x98, 0xc0, 0x60, 0x30, 0x38, 0x78, 0x3e, 0x03,
         0x00, 0xc0, 0x6c, 0x67, 0xf1, 0x38
       }
     },
     {
       .character = 0x34,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x07, 0x03, 0x83, 0xc3, 0xe1, 0xf1, 0xb9, 0x9c, 0xce, 0xff,
         0xff, 0xc1, 0xc0, 0xe0, 0x70, 0x0e
       }
     },
     {
       .character = 0x35,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x7f, 0xbf, 0xd8, 0x0c, 0x07, 0xc3, 0xf9, 0x8e, 0x03, 0x01,
         0x80, 0xd0, 0x7c, 0x77, 0xf1, 0x3c
       }
     },
     {
       .character = 0x36,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1f, 0x0f, 0xe7, 0x19, 0x80, 0xe0, 0x3f, 0x8f, 0xfb, 0x87,
         0xc0, 0xf0, 0x3c, 0x0d, 0x86, 0x7f, 0x87, 0x08
       }
     },
     {
       .character = 0x37,
       .width     = 8,
       .height    = 14,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0x06, 0x06, 0x0c, 0x0c, 0x1c, 0x18, 0x18, 0x30,
         0x30, 0x30, 0x60, 0x60
       }
     },
     {
       .character = 0x38,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1e, 0x1f, 0x98, 0x6c, 0x36, 0x19, 0x98, 0x78, 0x66, 0x61,
         0xe0, 0x70, 0x2c, 0x33, 0xf0, 0x3c
       }
     },
     {
       .character = 0x39,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1c, 0x3f, 0x98, 0xdc, 0x3c, 0x1f, 0x0d, 0x8e, 0xff, 0x3d,
         0x80, 0xc0, 0xc8, 0x67, 0xe1, 0x38
       }
     },
     {
       .character = 0x3a,
       .width     = 3,
       .height    = 10,
       .xoffset   = 4,
       .yoffset   = 11,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0x00, 0x07, 0x3e
       }
     },
     {
       .character = 0x3b,
       .width     = 4,
       .height    = 13,
       .xoffset   = 3,
       .yoffset   = 11,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x67, 0x70, 0x00, 0x06, 0x77, 0x36, 0x04
       }
     },
     {
       .character = 0x3c,
       .width     = 9,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 8,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x01, 0x83, 0xc7, 0xcf, 0x8e, 0x07, 0x01, 0xe0, 0x3c, 0x07,
         0x81, 0xc0, 0x01
       }
     },
     {
       .character = 0x3d,
       .width     = 9,
       .height    = 6,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0x3f
       }
     },
     {
       .character = 0x3e,
       .width     = 10,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 8,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc0, 0x3c, 0x03, 0xc0, 0x3c, 0x03, 0xc0, 0xf0, 0xf0, 0xf0,
         0x78, 0x38, 0x08, 0x00
       }
     },
     {
       .character = 0x3f,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3e, 0x3f, 0xb8, 0xe8, 0x30, 0x38, 0x1c, 0x1c, 0x1c, 0x0c,
         0x00, 0x00, 0x01, 0x81, 0xc0, 0x18
       }
     },
     {
       .character = 0x40,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0c, 0x0f, 0xc6, 0x1b, 0x06, 0xc7, 0xb7, 0xf9, 0x8e, 0x63,
         0x99, 0xf7, 0xfc, 0xed, 0x80, 0x3f, 0x87, 0x0c
       }
     },
     {
       .character = 0x41,
       .width     = 11,
       .height    = 14,
       .xoffset   = 0,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x04, 0x00, 0x80, 0x38, 0x07, 0x01, 0xe0, 0x36, 0x06, 0xc1,
         0x88, 0x3f, 0x87, 0xf1, 0x83, 0x30, 0x66, 0x0d, 0x80, 0x03
       }
     },
     {
       .character = 0x42,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xfe, 0x7f, 0xb0, 0xf8, 0x3c, 0x3f, 0xfb, 0xfd, 0x87, 0xc1,
         0xe0, 0xf0, 0x78, 0x7f, 0xf7, 0x3c
       }
     },
     {
       .character = 0x43,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1e, 0x0f, 0xe7, 0x1b, 0x80, 0xc0, 0x30, 0x0c, 0x03, 0x00,
         0xc0, 0x30, 0x0e, 0x01, 0xc6, 0x3f, 0x87, 0x08
       }
     },
     {
       .character = 0x44,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xfc, 0x7f, 0xb0, 0xd8, 0x3c, 0x1e, 0x0f, 0x07, 0x83, 0xc1,
         0xe0, 0xf0, 0x78, 0x6f, 0xf7, 0x38
       }
     },
     {
       .character = 0x45,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xf0, 0x18, 0x0c, 0x06, 0x03, 0xfd, 0xfe, 0xc0,
         0x60, 0x30, 0x18, 0x0f, 0xff, 0x3f
       }
     },
     {
       .character = 0x46,
       .width     = 8,
       .height    = 14,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xc0, 0xc0, 0xc0, 0xfe, 0xfe, 0xc0, 0xc0, 0xc0,
         0xc0, 0xc0, 0xc0, 0xc0
       }
     },
     {
       .character = 0x47,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1e, 0x1f, 0x9c, 0xf8, 0x1c, 0x06, 0x03, 0x01, 0x8f, 0xc7,
         0xe0, 0xf8, 0x6c, 0x37, 0xf8, 0x3c
       }
     },
     {
       .character = 0x48,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0x0f, 0xff, 0xff, 0xc1,
         0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0x03
       }
     },
     {
       .character = 0x49,
       .width     = 7,
       .height    = 14,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xfc, 0xc1, 0x83, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc1,
         0x8f, 0xff, 0x03
       }
     },
     {
       .character = 0x4a,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1f, 0x8f, 0xc1, 0x80, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06,
         0x03, 0x01, 0x98, 0xcf, 0xc3, 0x30
       }
     },
     {
       .character = 0x4b,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc1, 0xb0, 0xcc, 0x63, 0x38, 0xdc, 0x3e, 0x0f, 0x83, 0x60,
         0xcc, 0x33, 0x8c, 0x63, 0x0c, 0xc3, 0xb0, 0x07
       }
     },
     {
       .character = 0x4c,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x01, 0x80, 0xc0,
         0x60, 0x30, 0x18, 0x0f, 0xff, 0x3f
       }
     },
     {
       .character = 0x4d,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc1, 0xe0, 0xf8, 0xfc, 0x7e, 0x3f, 0xbf, 0x77, 0xbb, 0xc9,
         0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0x03
       }
     },
     {
       .character = 0x4e,
       .width     = 9,
       .height    = 13,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc1, 0xf0, 0xfc, 0x7e, 0x3d, 0x9e, 0xcf, 0x37, 0x9b, 0xc7,
         0xe3, 0xf0, 0xf8, 0x3c, 0x03
       }
     },
     {
       .character = 0x4f,
       .width     = 11,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3f, 0x0f, 0xf1, 0x86, 0x60, 0x6c, 0x0d, 0x81, 0xb0, 0x36,
         0x06, 0xc0, 0xd8, 0x1b, 0x03, 0x30, 0xc7, 0xf8, 0x7e, 0x00
       }
     },
     {
       .character = 0x50,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xfe, 0x7f, 0xb0, 0xf8, 0x3c, 0x1e, 0x1f, 0xfd, 0xfc, 0xc0,
         0x60, 0x30, 0x18, 0x0c, 0x06, 0x00
       }
     },
     {
       .character = 0x51,
       .width     = 10,
       .height    = 17,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1e, 0x1f, 0xe6, 0x1b, 0x03, 0xc0, 0xf0, 0x3c, 0x0f, 0x03,
         0xc0, 0xf0, 0x3c, 0x0d, 0x86, 0x7f, 0x8f, 0xc0, 0xc0, 0x3e,
         0x07, 0x02
       }
     },
     {
       .character = 0x52,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xfe, 0x3f, 0xcc, 0x3b, 0x06, 0xc1, 0xb0, 0xef, 0xf3, 0xf8,
         0xc6, 0x31, 0x8c, 0x33, 0x0c, 0xc1, 0xb0, 0x06
       }
     },
     {
       .character = 0x53,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1e, 0x1f, 0xee, 0x1b, 0x84, 0xe0, 0x1c, 0x03, 0xc0, 0x3c,
         0x03, 0x80, 0x74, 0x1f, 0x06, 0xff, 0x0f, 0x08
       }
     },
     {
       .character = 0x54,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xc6, 0x03, 0x01, 0x80, 0xc0, 0x60, 0x30, 0x18,
         0x0c, 0x06, 0x03, 0x01, 0x80, 0x30
       }
     },
     {
       .character = 0x55,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0x0f, 0x07, 0x83, 0xc1,
         0xe0, 0xf0, 0x7c, 0x77, 0xf1, 0x3c
       }
     },
     {
       .character = 0x56,
       .width     = 11,
       .height    = 14,
       .xoffset   = 0,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x60, 0x6c, 0x19, 0x83, 0x18, 0x63, 0x18, 0x63, 0x06, 0x60,
         0xc8, 0x1b, 0x01, 0xe0, 0x38, 0x07, 0x00, 0x60, 0x08, 0x00
       }
     },
     {
       .character = 0x57,
       .width     = 11,
       .height    = 14,
       .xoffset   = 0,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc0, 0x78, 0x8d, 0x99, 0xb3, 0x36, 0xe4, 0xdc, 0x9a, 0xf3,
         0x5e, 0x3b, 0xc7, 0x38, 0xc6, 0x18, 0xc3, 0x18, 0x61, 0x00
       }
     },
     {
       .character = 0x58,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc1, 0xb8, 0xe6, 0x30, 0xdc, 0x36, 0x07, 0x01, 0xc0, 0x70,
         0x1e, 0x0d, 0x87, 0x31, 0x8c, 0xe1, 0xb0, 0x06
       }
     },
     {
       .character = 0x59,
       .width     = 11,
       .height    = 14,
       .xoffset   = 0,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x60, 0xec, 0x18, 0xc3, 0x18, 0xc1, 0x98, 0x36, 0x03, 0xc0,
         0x70, 0x06, 0x00, 0xc0, 0x18, 0x03, 0x00, 0x60, 0x0c, 0x00
       }
     },
     {
       .character = 0x5a,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xc0, 0xc0, 0x60, 0x60, 0x70, 0x30, 0x30, 0x18,
         0x18, 0x1c, 0x0c, 0x0f, 0xff, 0x3f
       }
     },
     {
       .character = 0x5b,
       .width     = 7,
       .height    = 16,
       .xoffset   = 3,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xfd, 0xfb, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc1, 0x83, 0x06,
         0x0c, 0x18, 0x3f, 0xff
       }
     },
     {
       .character = 0x5c,
       .width     = 9,
       .height    = 16,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x40, 0x70, 0x18, 0x0e, 0x03, 0x01, 0xc0, 0x60, 0x38, 0x0c,
         0x07, 0x01, 0x80, 0xe0, 0x30, 0x18, 0x06, 0x00
       }
     },
     {
       .character = 0x5d,
       .width     = 6,
       .height    = 16,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xf0, 0xc3, 0x0c, 0x30, 0xc3, 0x0c, 0x30, 0xc3, 0x0c,
         0x3f, 0xff
       }
     },
     {
       .character = 0x5e,
       .width     = 7,
       .height    = 6,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x10, 0x70, 0xf3, 0x6c, 0x78, 0x03
       }
     },
     {
       .character = 0x5f,
       .width     = 10,
       .height    = 2,
       .xoffset   = 1,
       .yoffset   = 20,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0x0f
       }
     },
     {
       .character = 0x60,
       .width     = 3,
       .height    = 9,
       .xoffset   = 3,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x12, 0x6e, 0xd9, 0x01
       }
     },
     {
       .character = 0x61,
       .width     = 9,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3e, 0x3f, 0x98, 0xe0, 0x33, 0xfb, 0xff, 0x07, 0x87, 0xff,
         0xbe, 0x03
       }
     },
     {
       .character = 0x62,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc0, 0x60, 0x30, 0x18, 0x0d, 0xe7, 0xfb, 0x8f, 0x83, 0xc1,
         0xe0, 0xf0, 0x7c, 0x7f, 0xf6, 0x3c
       }
     },
     {
       .character = 0x63,
       .width     = 9,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1f, 0x1f, 0xd8, 0x7c, 0x0c, 0x06, 0x03, 0x80, 0xe3, 0x3f,
         0x8f, 0x02
       }
     },
     {
       .character = 0x64,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x01, 0x80, 0xc0, 0x60, 0x33, 0xdb, 0xff, 0x8f, 0x83, 0xc1,
         0xe0, 0xf0, 0x7c, 0x77, 0xf9, 0x3b
       }
     },
     {
       .character = 0x65,
       .width     = 9,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1e, 0x3f, 0x98, 0xf8, 0x3f, 0xff, 0xff, 0x01, 0xc2, 0x7f,
         0x8f, 0x00
       }
     },
     {
       .character = 0x66,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0f, 0x87, 0xf1, 0x88, 0xe0, 0x30, 0x3f, 0xcf, 0xf0, 0xc0,
         0x30, 0x0c, 0x03, 0x00, 0xc0, 0x30, 0x0c, 0x00
       }
     },
     {
       .character = 0x67,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3d, 0xdf, 0xfc, 0x33, 0x0c, 0xe7, 0x1f, 0x87, 0xc1, 0x00,
         0x7f, 0x1f, 0xec, 0x1b, 0x06, 0xff, 0x9f, 0x08
       }
     },
     {
       .character = 0x68,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xe0, 0x70, 0x38, 0x1c, 0x0e, 0xe7, 0xfb, 0xcf, 0xc3, 0xe1,
         0xf0, 0xf8, 0x7c, 0x3e, 0x1f, 0x03
       }
     },
     {
       .character = 0x69,
       .width     = 7,
       .height    = 14,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x30, 0xf0, 0xc0, 0x0f, 0x9f, 0x06, 0x0c, 0x18, 0x30, 0x60,
         0xcf, 0xff, 0x03
       }
     },
     {
       .character = 0x6a,
       .width     = 8,
       .height    = 18,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0c, 0x1e, 0x0c, 0x00, 0x3e, 0x3e, 0x06, 0x06, 0x06, 0x06,
         0x06, 0x06, 0x06, 0x06, 0x06, 0xc6, 0xfe, 0x78
       }
     },
     {
       .character = 0x6b,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc0, 0x30, 0x0c, 0x03, 0x00, 0xc1, 0xb1, 0xcc, 0xe3, 0x70,
         0xf8, 0x3b, 0x0c, 0x63, 0x1c, 0xc3, 0xb0, 0x07
       }
     },
     {
       .character = 0x6c,
       .width     = 8,
       .height    = 14,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xf8, 0xf8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
         0x18, 0x18, 0xff, 0xff
       }
     },
     {
       .character = 0x6d,
       .width     = 10,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xfb, 0xbf, 0xec, 0xcf, 0x33, 0xcc, 0xf3, 0x3c, 0xcf, 0x33,
         0xcc, 0xf3, 0x03
       }
     },
     {
       .character = 0x6e,
       .width     = 9,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xee, 0x7f, 0xbc, 0xfc, 0x3e, 0x1f, 0x0f, 0x87, 0xc3, 0xe1,
         0xf0, 0x03
       }
     },
     {
       .character = 0x6f,
       .width     = 9,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1e, 0x3f, 0xb8, 0xf8, 0x3c, 0x1e, 0x0f, 0x07, 0xc7, 0x7f,
         0x0f, 0x00
       }
     },
     {
       .character = 0x70,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xde, 0x7f, 0xb8, 0xf8, 0x3c, 0x1e, 0x0f, 0x07, 0xc7, 0xff,
         0x6f, 0x30, 0x18, 0x0c, 0x06, 0x00
       }
     },
     {
       .character = 0x71,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3f, 0xbf, 0xf8, 0xf8, 0x3c, 0x1e, 0x0f, 0x07, 0xc7, 0x7f,
         0x9e, 0xc0, 0x60, 0x30, 0x18, 0x03
       }
     },
     {
       .character = 0x72,
       .width     = 8,
       .height    = 10,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xde, 0xff, 0xf1, 0xe0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0
       }
     },
     {
       .character = 0x73,
       .width     = 9,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3e, 0x3f, 0xd8, 0x4e, 0x03, 0xe0, 0x79, 0x07, 0x83, 0xff,
         0x1f, 0x00
       }
     },
     {
       .character = 0x74,
       .width     = 9,
       .height    = 13,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x18, 0x0c, 0x06, 0x1f, 0xef, 0xf0, 0xc0, 0x60, 0x30, 0x18,
         0x0c, 0x06, 0x23, 0xf0, 0x1e
       }
     },
     {
       .character = 0x75,
       .width     = 9,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0x0f, 0x07, 0xc7, 0x7f,
         0x9e, 0x03
       }
     },
     {
       .character = 0x76,
       .width     = 9,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc1, 0xe0, 0xd8, 0x6c, 0x67, 0x31, 0x90, 0xd8, 0x38, 0x1c,
         0x06, 0x00
       }
     },
     {
       .character = 0x77,
       .width     = 11,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc0, 0x68, 0xcd, 0x99, 0xb7, 0x26, 0xec, 0xd7, 0x8a, 0xf1,
         0xce, 0x39, 0x86, 0x0c
       }
     },
     {
       .character = 0x78,
       .width     = 10,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc1, 0x98, 0xc3, 0x60, 0xf8, 0x1c, 0x07, 0x03, 0x61, 0xdc,
         0x63, 0x30, 0x06
       }
     },
     {
       .character = 0x79,
       .width     = 11,
       .height    = 14,
       .xoffset   = 0,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x60, 0xce, 0x18, 0xc3, 0x18, 0xc1, 0x98, 0x33, 0x06, 0xc0,
         0x78, 0x0e, 0x00, 0xc0, 0x18, 0x26, 0x07, 0xc0, 0xf0, 0x00
       }
     },
     {
       .character = 0x7a,
       .width     = 9,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xc1, 0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xff,
         0xff, 0x03
       }
     },
     {
       .character = 0x7b,
       .width     = 8,
       .height    = 18,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x07, 0x1f, 0x18, 0x30, 0x38, 0x38, 0x30, 0xf0, 0xe0, 0x30,
         0x30, 0x30, 0x30, 0x30, 0x30, 0x18, 0x1f, 0x0f
       }
     },
     {
       .character = 0x7c,
       .width     = 2,
       .height    = 18,
       .xoffset   = 5,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff, 0xff, 0x0f
       }
     },
     {
       .character = 0x7d,
       .width     = 8,
       .height    = 18,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xf0, 0xf8, 0x1c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0f, 0x07, 0x0c,
         0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x1c, 0xf8, 0xf0
       }
     },
     {
       .character = 0x7e,
       .width     = 10,
       .height    = 3,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x78, 0xbf, 0xe8, 0x3c
       }
     }
  }
};
