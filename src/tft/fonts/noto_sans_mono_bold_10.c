#include "fonts.h"

const tft_font_t noto_sans_mono_bold_10 = {
  .size = 10,
  .yoffset = 4,
  .glyph_count = 95,
  .glyphs = (const tft_font_glyph_t[]){
     {
       .character = 0x20,
       .width     = 1,
       .height    = 1,
       .xoffset   = 0,
       .yoffset   = 14,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x00
       }
     },
     {
       .character = 0x21,
       .width     = 2,
       .height    = 10,
       .xoffset   = 3,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0x0f
       }
     },
     {
       .character = 0x22,
       .width     = 4,
       .height    = 3,
       .xoffset   = 2,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xf9, 0x09
       }
     },
     {
       .character = 0x23,
       .width     = 8,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x16, 0x14, 0x7f, 0x7f, 0x24, 0xfe, 0xfe, 0x2c, 0x28, 0x68
       }
     },
     {
       .character = 0x24,
       .width     = 6,
       .height    = 12,
       .xoffset   = 1,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x11, 0xff, 0xf4, 0xd1, 0xc1, 0xc7, 0x9f, 0xe7, 0x04
       }
     },
     {
       .character = 0x25,
       .width     = 8,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x66, 0xb4, 0xbc, 0x68, 0x18, 0x16, 0x3d, 0x29, 0x2d, 0x46
       }
     },
     {
       .character = 0x26,
       .width     = 8,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x38, 0x78, 0x6c, 0x68, 0x38, 0x72, 0x5e, 0xce, 0x7e, 0x7f
       }
     },
     {
       .character = 0x27,
       .width     = 2,
       .height    = 3,
       .xoffset   = 3,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x3f
       }
     },
     {
       .character = 0x28,
       .width     = 3,
       .height    = 12,
       .xoffset   = 3,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x6b, 0x49, 0x24, 0x9b, 0x03
       }
     },
     {
       .character = 0x29,
       .width     = 3,
       .height    = 12,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xcd, 0x92, 0x49, 0x2d, 0x06
       }
     },
     {
       .character = 0x2a,
       .width     = 6,
       .height    = 6,
       .xoffset   = 1,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x30, 0xcf, 0xce, 0x79, 0x02
       }
     },
     {
       .character = 0x2b,
       .width     = 6,
       .height    = 7,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x30, 0xc3, 0x3f, 0xfc, 0xc3, 0x00
       }
     },
     {
       .character = 0x2c,
       .width     = 2,
       .height    = 4,
       .xoffset   = 3,
       .yoffset   = 13,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xfe
       }
     },
     {
       .character = 0x2d,
       .width     = 4,
       .height    = 2,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff
       }
     },
     {
       .character = 0x2e,
       .width     = 2,
       .height    = 3,
       .xoffset   = 3,
       .yoffset   = 12,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x3f
       }
     },
     {
       .character = 0x2f,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x0c, 0x61, 0x84, 0x30, 0xc2, 0x18, 0x63, 0x00
       }
     },
     {
       .character = 0x30,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7b, 0xfc, 0xf7, 0xff, 0xbe, 0xf3, 0xfd, 0x0e
       }
     },
     {
       .character = 0x31,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x33, 0xcf, 0x0c, 0x30, 0xc3, 0x0c, 0x7b, 0x0f
       }
     },
     {
       .character = 0x32,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7b, 0xf8, 0xc3, 0x0c, 0x63, 0x18, 0xff, 0x0f
       }
     },
     {
       .character = 0x33,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7b, 0xf0, 0xc3, 0x79, 0xf0, 0xc3, 0xff, 0x0e
       }
     },
     {
       .character = 0x34,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x18, 0x63, 0x9e, 0x5b, 0x6f, 0xff, 0x18, 0x06
       }
     },
     {
       .character = 0x35,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7b, 0xec, 0x30, 0xf8, 0x70, 0xc3, 0xff, 0x0c
       }
     },
     {
       .character = 0x36,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x3d, 0xfc, 0x36, 0xff, 0x3c, 0xf3, 0xfd, 0x0e
       }
     },
     {
       .character = 0x37,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff, 0xf0, 0xc6, 0x18, 0x63, 0x0c, 0x61, 0x08
       }
     },
     {
       .character = 0x38,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7b, 0xfc, 0xf3, 0x79, 0xec, 0xf3, 0xfd, 0x0e
       }
     },
     {
       .character = 0x39,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7b, 0xfc, 0xf3, 0xfd, 0xf0, 0xc3, 0xfb, 0x0c
       }
     },
     {
       .character = 0x3a,
       .width     = 2,
       .height    = 8,
       .xoffset   = 3,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xfc, 0x3f
       }
     },
     {
       .character = 0x3b,
       .width     = 2,
       .height    = 10,
       .xoffset   = 3,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xfc, 0x0f, 0x0e
       }
     },
     {
       .character = 0x3c,
       .width     = 6,
       .height    = 7,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x04, 0x77, 0x38, 0x38, 0x30, 0x01
       }
     },
     {
       .character = 0x3d,
       .width     = 6,
       .height    = 5,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff, 0xf0, 0x3f, 0x3f
       }
     },
     {
       .character = 0x3e,
       .width     = 6,
       .height    = 7,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x83, 0x83, 0x87, 0x73, 0x08, 0x00
       }
     },
     {
       .character = 0x3f,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7b, 0xf0, 0xc3, 0x38, 0xc0, 0x0c, 0x70, 0x0c
       }
     },
     {
       .character = 0x40,
       .width     = 8,
       .height    = 11,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x3c, 0x62, 0x5b, 0xdd, 0xb5, 0xa5, 0xad, 0xbf, 0xd6, 0x60,
         0x3e
       }
     },
     {
       .character = 0x41,
       .width     = 8,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x18, 0x3c, 0x3c, 0x3c, 0x24, 0x66, 0x7e, 0x7e, 0x42, 0xc3
       }
     },
     {
       .character = 0x42,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xfb, 0xfc, 0xf3, 0xfb, 0xfc, 0xf3, 0xff, 0x0e
       }
     },
     {
       .character = 0x43,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x3d, 0xfc, 0x30, 0xc3, 0x0c, 0x30, 0x7c, 0x0f
       }
     },
     {
       .character = 0x44,
       .width     = 7,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xf9, 0xfb, 0x36, 0x2c, 0x78, 0xf3, 0x66, 0xfd, 0x38
       }
     },
     {
       .character = 0x45,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff, 0xfc, 0x30, 0xff, 0xfc, 0x30, 0xff, 0x0f
       }
     },
     {
       .character = 0x46,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff, 0xfc, 0x30, 0xff, 0xfc, 0x30, 0xc3, 0x00
       }
     },
     {
       .character = 0x47,
       .width     = 7,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x1e, 0xfd, 0x83, 0x0c, 0xf9, 0xd0, 0xb1, 0x7e, 0x1f
       }
     },
     {
       .character = 0x48,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xcf, 0x3c, 0xf3, 0xff, 0xfc, 0xf3, 0xcf, 0x03
       }
     },
     {
       .character = 0x49,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xfd, 0xe3, 0x0c, 0x30, 0xc3, 0x0c, 0x7b, 0x0f
       }
     },
     {
       .character = 0x4a,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x0c, 0x30, 0xc3, 0x0c, 0x30, 0xc6, 0xfb, 0x0c
       }
     },
     {
       .character = 0x4b,
       .width     = 7,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xcf, 0xbb, 0x67, 0x8f, 0x1e, 0x36, 0x6c, 0xcd, 0x26
       }
     },
     {
       .character = 0x4c,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xc3, 0x0c, 0x30, 0xc3, 0x0c, 0x30, 0xff, 0x0f
       }
     },
     {
       .character = 0x4d,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xcf, 0x3c, 0xff, 0xfe, 0xdb, 0x6d, 0x86, 0x01
       }
     },
     {
       .character = 0x4e,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xc7, 0x9e, 0x69, 0xb6, 0xd9, 0x67, 0x9e, 0x03
       }
     },
     {
       .character = 0x4f,
       .width     = 8,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x3c, 0x7e, 0x66, 0x42, 0xc3, 0xc3, 0x42, 0x66, 0x7e, 0x3c
       }
     },
     {
       .character = 0x50,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xfb, 0xfc, 0xf3, 0xff, 0xec, 0x30, 0xc3, 0x00
       }
     },
     {
       .character = 0x51,
       .width     = 8,
       .height    = 12,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x3c, 0x7e, 0x66, 0x42, 0xc3, 0xc3, 0x42, 0x66, 0x7e, 0x3c,
         0x0c, 0x06
       }
     },
     {
       .character = 0x52,
       .width     = 7,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xf9, 0xfb, 0x36, 0x6f, 0xdf, 0x36, 0x64, 0xcd, 0x23
       }
     },
     {
       .character = 0x53,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7f, 0xfc, 0x30, 0x70, 0xf0, 0xc3, 0xff, 0x0e
       }
     },
     {
       .character = 0x54,
       .width     = 8,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18
       }
     },
     {
       .character = 0x55,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xfd, 0x0e
       }
     },
     {
       .character = 0x56,
       .width     = 8,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xc3, 0x42, 0x66, 0x66, 0x66, 0x24, 0x3c, 0x3c, 0x18, 0x18
       }
     },
     {
       .character = 0x57,
       .width     = 8,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xc3, 0xc3, 0xdb, 0xdb, 0xdb, 0x5a, 0x7e, 0x7e, 0x66, 0x66
       }
     },
     {
       .character = 0x58,
       .width     = 8,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xe7, 0x66, 0x3c, 0x3c, 0x18, 0x18, 0x3c, 0x3c, 0x66, 0xc7
       }
     },
     {
       .character = 0x59,
       .width     = 8,
       .height    = 10,
       .xoffset   = 0,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xe3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x18, 0x18, 0x18, 0x18
       }
     },
     {
       .character = 0x5a,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff, 0xf1, 0x86, 0x30, 0xc6, 0x18, 0xff, 0x0f
       }
     },
     {
       .character = 0x5b,
       .width     = 3,
       .height    = 13,
       .xoffset   = 3,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff, 0x6d, 0xb6, 0xdb, 0x3f
       }
     },
     {
       .character = 0x5c,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xc1, 0x86, 0x08, 0x30, 0xc1, 0x06, 0x18, 0x03
       }
     },
     {
       .character = 0x5d,
       .width     = 3,
       .height    = 13,
       .xoffset   = 2,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xfd, 0xb6, 0xdb, 0x6d, 0x7f
       }
     },
     {
       .character = 0x5e,
       .width     = 6,
       .height    = 7,
       .xoffset   = 1,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x20, 0xc3, 0x1e, 0x4b, 0x38, 0x01
       }
     },
     {
       .character = 0x5f,
       .width     = 8,
       .height    = 2,
       .xoffset   = 0,
       .yoffset   = 16,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff, 0xff
       }
     },
     {
       .character = 0x60,
       .width     = 3,
       .height    = 3,
       .xoffset   = 2,
       .yoffset   = 3,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xc9, 0x01
       }
     },
     {
       .character = 0x61,
       .width     = 6,
       .height    = 8,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7b, 0xf0, 0xdf, 0xff, 0x3f, 0xfd
       }
     },
     {
       .character = 0x62,
       .width     = 6,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xc3, 0x0c, 0x3e, 0xff, 0x3c, 0xf3, 0xcf, 0xfb, 0x02
       }
     },
     {
       .character = 0x63,
       .width     = 6,
       .height    = 8,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x3d, 0xfc, 0x30, 0xc3, 0x0f, 0xdf
       }
     },
     {
       .character = 0x64,
       .width     = 6,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x0c, 0x30, 0xdf, 0xff, 0x3c, 0xf1, 0xcf, 0xf7, 0x01
       }
     },
     {
       .character = 0x65,
       .width     = 6,
       .height    = 8,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7b, 0xfc, 0xff, 0xff, 0x0f, 0xdf
       }
     },
     {
       .character = 0x66,
       .width     = 7,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x1e, 0x78, 0xc7, 0xef, 0xc6, 0x0c, 0x18, 0x30, 0x60, 0x18
       }
     },
     {
       .character = 0x67,
       .width     = 6,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x77, 0xfc, 0xf3, 0xcf, 0x3f, 0xdf, 0x0f, 0xff, 0x02
       }
     },
     {
       .character = 0x68,
       .width     = 6,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xc3, 0x0c, 0x3e, 0xff, 0x3c, 0xf3, 0xcf, 0x3c, 0x03
       }
     },
     {
       .character = 0x69,
       .width     = 6,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x30, 0xc0, 0x3c, 0xf0, 0xc3, 0x0c, 0x31, 0xef, 0x03
       }
     },
     {
       .character = 0x6a,
       .width     = 5,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x19, 0xc0, 0xf7, 0x8c, 0x63, 0x18, 0xc6, 0x3f, 0x1e
       }
     },
     {
       .character = 0x6b,
       .width     = 7,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xc1, 0x83, 0x06, 0x7d, 0x9f, 0x3c, 0x7c, 0xd9, 0x9b, 0x07
       }
     },
     {
       .character = 0x6c,
       .width     = 6,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xf1, 0xc3, 0x0c, 0x30, 0xc3, 0x0c, 0x31, 0xef, 0x03
       }
     },
     {
       .character = 0x6d,
       .width     = 8,
       .height    = 8,
       .xoffset   = 0,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xf6, 0xff, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb
       }
     },
     {
       .character = 0x6e,
       .width     = 6,
       .height    = 8,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xbb, 0xfc, 0xf3, 0xcf, 0x3c, 0xf3
       }
     },
     {
       .character = 0x6f,
       .width     = 6,
       .height    = 8,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7b, 0xfc, 0xf3, 0xcf, 0x3f, 0xde
       }
     },
     {
       .character = 0x70,
       .width     = 6,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xbb, 0xfc, 0xf3, 0xcf, 0x3f, 0xfe, 0xc3, 0x0c, 0x00
       }
     },
     {
       .character = 0x71,
       .width     = 6,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x77, 0xfc, 0xf3, 0xcf, 0x3f, 0xdf, 0x0c, 0x30, 0x03
       }
     },
     {
       .character = 0x72,
       .width     = 7,
       .height    = 8,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xee, 0xf9, 0xc3, 0x06, 0x0c, 0x3e, 0x7c
       }
     },
     {
       .character = 0x73,
       .width     = 6,
       .height    = 8,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x7f, 0xfc, 0x1c, 0x3c, 0x3f, 0xfe
       }
     },
     {
       .character = 0x74,
       .width     = 6,
       .height    = 10,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x20, 0x8f, 0xff, 0x61, 0x86, 0x18, 0x3c, 0x0f
       }
     },
     {
       .character = 0x75,
       .width     = 6,
       .height    = 8,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xcf, 0x3c, 0xf3, 0xcf, 0x3f, 0xdd
       }
     },
     {
       .character = 0x76,
       .width     = 8,
       .height    = 8,
       .xoffset   = 0,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xc3, 0x66, 0x66, 0x66, 0x3c, 0x3c, 0x3c, 0x18
       }
     },
     {
       .character = 0x77,
       .width     = 8,
       .height    = 8,
       .xoffset   = 0,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xc3, 0xdb, 0xdb, 0xdb, 0x5a, 0x7e, 0x66, 0x66
       }
     },
     {
       .character = 0x78,
       .width     = 8,
       .height    = 8,
       .xoffset   = 0,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xe7, 0x66, 0x3c, 0x1c, 0x3c, 0x3c, 0x66, 0xe7
       }
     },
     {
       .character = 0x79,
       .width     = 8,
       .height    = 11,
       .xoffset   = 0,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xe3, 0x66, 0x66, 0x26, 0x3c, 0x3c, 0x18, 0x18, 0x18, 0x70,
         0x60
       }
     },
     {
       .character = 0x7a,
       .width     = 6,
       .height    = 8,
       .xoffset   = 1,
       .yoffset   = 7,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff, 0xf1, 0x8c, 0x71, 0x8f, 0xff
       }
     },
     {
       .character = 0x7b,
       .width     = 4,
       .height    = 13,
       .xoffset   = 2,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x37, 0x66, 0x6e, 0xc6, 0x66, 0x67, 0x01
       }
     },
     {
       .character = 0x7c,
       .width     = 2,
       .height    = 14,
       .xoffset   = 3,
       .yoffset   = 4,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff, 0x0f
       }
     },
     {
       .character = 0x7d,
       .width     = 4,
       .height    = 13,
       .xoffset   = 2,
       .yoffset   = 5,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x8e, 0x66, 0x66, 0x37, 0x66, 0x6e, 0x0c
       }
     },
     {
       .character = 0x7e,
       .width     = 6,
       .height    = 3,
       .xoffset   = 1,
       .yoffset   = 8,
       .xadvance  = 8,
       .data      = (const uint8_t[]){
         0x67, 0xf9, 0x03
       }
     }
  }
};
