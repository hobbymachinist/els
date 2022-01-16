#include "fonts.h"

const tft_font_t noto_sans_mono_bold_14 = {
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
         0xff, 0xff, 0xff, 0xe0, 0x7f, 0x03
       }
     },
     {
       .character = 0x22,
       .width     = 7,
       .height    = 5,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xee, 0xdd, 0xbb, 0x76, 0x06
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
         0x0d, 0x81, 0xb0, 0x26, 0x0c, 0xc7, 0xfe, 0xff, 0xc6, 0xc0,
         0xd8, 0xff, 0xdf, 0xf8, 0xcc, 0x19, 0x83, 0x20, 0x6c, 0x00
       }
     },
     {
       .character = 0x24,
       .width     = 8,
       .height    = 16,
       .xoffset   = 2,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x10, 0x7f, 0xfe, 0xd2, 0xd0, 0xd0, 0xf0, 0x7c, 0x1e, 0x17,
         0x17, 0x97, 0xfe, 0xfc, 0x10, 0x10
       }
     },
     {
       .character = 0x25,
       .width     = 11,
       .height    = 14,
       .xoffset   = 0,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x70, 0xdf, 0x33, 0x66, 0x65, 0x8d, 0xb1, 0xfc, 0x1d, 0x80,
         0x6e, 0x0f, 0xe3, 0x6c, 0x6d, 0x99, 0xb3, 0x3e, 0xc3, 0x02
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
         0x3c, 0x1f, 0x06, 0xe1, 0x98, 0x6c, 0x1e, 0x07, 0x9b, 0xf6,
         0xdf, 0xb3, 0xec, 0x73, 0xbc, 0xff, 0x9e, 0x07
       }
     },
     {
       .character = 0x27,
       .width     = 3,
       .height    = 5,
       .xoffset   = 4,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0x5b
       }
     },
     {
       .character = 0x28,
       .width     = 4,
       .height    = 17,
       .xoffset   = 4,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x37, 0x6e, 0xcc, 0xcc, 0xcc, 0xcc, 0xce, 0x67, 0x03
       }
     },
     {
       .character = 0x29,
       .width     = 5,
       .height    = 17,
       .xoffset   = 3,
       .yoffset   = 7,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc3, 0x18, 0xe3, 0x18, 0xe7, 0x39, 0xce, 0x63, 0x19, 0x8c,
         0x18
       }
     },
     {
       .character = 0x2a,
       .width     = 9,
       .height    = 9,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0c, 0x06, 0x22, 0x2f, 0xff, 0xf8, 0xe0, 0xd8, 0xee, 0x12,
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
       .width     = 3,
       .height    = 6,
       .xoffset   = 4,
       .yoffset   = 17,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x7f, 0xed, 0x02
       }
     },
     {
       .character = 0x2d,
       .width     = 5,
       .height    = 2,
       .xoffset   = 3,
       .yoffset   = 14,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0x03
       }
     },
     {
       .character = 0x2e,
       .width     = 3,
       .height    = 3,
       .xoffset   = 4,
       .yoffset   = 17,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0x01
       }
     },
     {
       .character = 0x2f,
       .width     = 7,
       .height    = 14,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x06, 0x1c, 0x30, 0x61, 0xc3, 0x06, 0x1c, 0x30, 0x61, 0xc3,
         0x0e, 0x18, 0x00
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
         0x3e, 0x3f, 0x9d, 0xdc, 0xfe, 0x7f, 0x7f, 0xb7, 0xf3, 0xf9,
         0xf9, 0xf8, 0xee, 0xf7, 0xf1, 0x3c
       }
     },
     {
       .character = 0x31,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1c, 0x1e, 0x3f, 0x0d, 0x84, 0xc0, 0x60, 0x30, 0x18, 0x0c,
         0x06, 0x03, 0x01, 0x87, 0xfb, 0x3f
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
         0x3e, 0x7f, 0x99, 0xe8, 0x70, 0x38, 0x1c, 0x1c, 0x1e, 0x1e,
         0x1e, 0x0e, 0x0e, 0x0f, 0xff, 0x3f
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
         0x3e, 0x7f, 0x99, 0xe0, 0x70, 0x30, 0x38, 0xf8, 0x7e, 0x03,
         0x81, 0xc0, 0xf8, 0xff, 0xf3, 0x3c
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
         0x07, 0x07, 0x83, 0xc3, 0xe3, 0xf1, 0xb9, 0xdc, 0xce, 0xc7,
         0x7f, 0xff, 0xe0, 0xe0, 0x70, 0x0e
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
         0x7f, 0x3f, 0x9c, 0x0e, 0x06, 0x03, 0xf1, 0xfc, 0x8f, 0x03,
         0x81, 0xc0, 0xfd, 0xef, 0xf3, 0x38
       }
     },
     {
       .character = 0x36,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0f, 0x1f, 0x9e, 0x2e, 0x0e, 0x07, 0xfb, 0xff, 0xe7, 0xe3,
         0xf1, 0xf8, 0xee, 0x77, 0xf0, 0x3c
       }
     },
     {
       .character = 0x37,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xc0, 0xe0, 0x70, 0x70, 0x38, 0x38, 0x1c, 0x0c,
         0x0e, 0x07, 0x07, 0x03, 0x83, 0x20
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
         0x3e, 0x3f, 0x9d, 0xec, 0x76, 0x33, 0xf8, 0xf8, 0xfe, 0x77,
         0xf1, 0xf8, 0xfe, 0x77, 0xf1, 0x3c
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
         0x3e, 0x3f, 0xbd, 0xdc, 0x7e, 0x3f, 0x1f, 0xde, 0xfb, 0x3b,
         0x81, 0xc0, 0xd1, 0xe7, 0xe3, 0x30
       }
     },
     {
       .character = 0x3a,
       .width     = 3,
       .height    = 11,
       .xoffset   = 4,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0x80, 0x00, 0xff, 0x01
       }
     },
     {
       .character = 0x3b,
       .width     = 4,
       .height    = 14,
       .xoffset   = 4,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x6f, 0x60, 0x00, 0x00, 0x6e, 0xec, 0xcc
       }
     },
     {
       .character = 0x3c,
       .width     = 8,
       .height    = 9,
       .xoffset   = 2,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x03, 0x0f, 0x3c, 0xf0, 0xe0, 0x78, 0x1e, 0x07, 0x01
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
       .width     = 8,
       .height    = 9,
       .xoffset   = 2,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x80, 0xe0, 0x78, 0x1e, 0x0f, 0x3e, 0xf8, 0xe0, 0x80
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
         0x3e, 0x7f, 0x99, 0xe0, 0x70, 0x38, 0x38, 0x38, 0x38, 0x18,
         0x0c, 0x00, 0x03, 0x03, 0xc0, 0x30
       }
     },
     {
       .character = 0x40,
       .width     = 11,
       .height    = 16,
       .xoffset   = 0,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0f, 0x07, 0xf0, 0xc3, 0x30, 0x36, 0xf7, 0xbe, 0xf6, 0xde,
         0xdb, 0xdb, 0x7b, 0x6f, 0x7f, 0xe7, 0xe6, 0x00, 0xe0, 0x0f,
         0xe0, 0xfc
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
         0x0e, 0x03, 0xe0, 0x7c, 0x0d, 0x81, 0xb8, 0x77, 0x0e, 0x61,
         0x8c, 0x3f, 0xcf, 0xf9, 0xc3, 0x30, 0x66, 0x0f, 0xc1, 0x03
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
         0xfe, 0x7f, 0xf8, 0xfc, 0x7e, 0x3f, 0x1f, 0xfd, 0xff, 0xe3,
         0xf0, 0xf8, 0x7c, 0x7f, 0xff, 0x3c
       }
     },
     {
       .character = 0x43,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0f, 0x9f, 0xde, 0x6e, 0x0e, 0x07, 0x03, 0x81, 0xc0, 0xe0,
         0x70, 0x38, 0x0f, 0x33, 0xf8, 0x3f
       }
     },
     {
       .character = 0x44,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xfc, 0x3f, 0xce, 0x7b, 0x8e, 0xe1, 0xb8, 0x7e, 0x1f, 0x87,
         0xe1, 0xf8, 0x6e, 0x3b, 0x9e, 0xff, 0x3f, 0x00
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
         0xff, 0xff, 0xf8, 0x1c, 0x0e, 0x07, 0x03, 0xff, 0xff, 0xe0,
         0x70, 0x38, 0x1c, 0x0f, 0xff, 0x3f
       }
     },
     {
       .character = 0x46,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xf8, 0x1c, 0x0e, 0x07, 0x03, 0xff, 0xff, 0xe0,
         0x70, 0x38, 0x1c, 0x0e, 0x07, 0x00
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
         0x1f, 0x3f, 0xde, 0x7c, 0x0e, 0x06, 0x03, 0x1f, 0x8f, 0xc1,
         0xf0, 0xf8, 0x6e, 0x37, 0xf8, 0x3f
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
         0xe1, 0xf0, 0xf8, 0x7c, 0x3e, 0x1f, 0x0f, 0xff, 0xff, 0xe1,
         0xf0, 0xf8, 0x7c, 0x3e, 0x1f, 0x03
       }
     },
     {
       .character = 0x49,
       .width     = 8,
       .height    = 14,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38,
         0x38, 0x38, 0xff, 0xff
       }
     },
     {
       .character = 0x4a,
       .width     = 7,
       .height    = 14,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0e, 0x1c, 0x38, 0x70, 0xe1, 0xc3, 0x87, 0x0e, 0x1c, 0x3e,
         0xff, 0xdf, 0x00
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
         0xc3, 0xf0, 0xec, 0x73, 0x38, 0xce, 0x37, 0x0d, 0x83, 0xf0,
         0xee, 0x33, 0x8c, 0x73, 0x1c, 0xc3, 0xb0, 0x0e
       }
     },
     {
       .character = 0x4c,
       .width     = 8,
       .height    = 14,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
         0xc0, 0xff, 0xff, 0xff
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
         0xe3, 0xf9, 0xfc, 0xfe, 0xff, 0x7f, 0xbf, 0xdf, 0xaf, 0xdd,
         0xee, 0xf7, 0x78, 0x3c, 0x1e, 0x03
       }
     },
     {
       .character = 0x4e,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xf1, 0xf8, 0xfc, 0x7f, 0x3d, 0x9e, 0xcf, 0x77, 0x9b, 0xcd,
         0xe6, 0xf1, 0xf8, 0xfc, 0x7e, 0x0f
       }
     },
     {
       .character = 0x4f,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3e, 0x1f, 0xcf, 0x7b, 0x8e, 0xe1, 0xb0, 0x7c, 0x1f, 0x07,
         0xc1, 0xf8, 0x6e, 0x3b, 0xde, 0x7f, 0x0f, 0x08
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
         0xfe, 0x7f, 0xf8, 0xfc, 0x3e, 0x1f, 0x0f, 0x8f, 0xff, 0xfe,
         0x70, 0x38, 0x1c, 0x0e, 0x07, 0x00
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
         0x3e, 0x1f, 0xcf, 0x7b, 0x8e, 0xe1, 0xb0, 0x7c, 0x1f, 0x07,
         0xc1, 0xf8, 0x6e, 0x3b, 0xde, 0x7f, 0x0f, 0x80, 0x70, 0x0c,
         0x03, 0x02
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
         0xfe, 0x3f, 0xce, 0x7b, 0x8e, 0xe3, 0xb8, 0xef, 0xf3, 0xf8,
         0xee, 0x39, 0xce, 0x73, 0x8e, 0xe3, 0xb8, 0x07
       }
     },
     {
       .character = 0x53,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3f, 0x3f, 0xdc, 0xcc, 0x06, 0x03, 0xc0, 0xf8, 0x3e, 0x07,
         0x81, 0xc0, 0xfc, 0xff, 0xf3, 0x3c
       }
     },
     {
       .character = 0x54,
       .width     = 10,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xf1, 0xc0, 0x70, 0x1c, 0x07, 0x01, 0xc0, 0x70,
         0x1c, 0x07, 0x01, 0xc0, 0x70, 0x1c, 0x07, 0x00
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
         0xe1, 0xf0, 0xf8, 0x7c, 0x3e, 0x1f, 0x0f, 0x87, 0xc3, 0xe1,
         0xf0, 0xf8, 0xfe, 0xf7, 0xf1, 0x3c
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
         0xe0, 0xec, 0x1d, 0xc3, 0x38, 0xe7, 0x1c, 0x63, 0x0c, 0x61,
         0xcc, 0x1b, 0x83, 0x60, 0x6c, 0x0f, 0x80, 0xf0, 0x1c, 0x00
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
         0xe0, 0x7c, 0x0f, 0x83, 0xf7, 0x76, 0xee, 0xdd, 0xdb, 0xbb,
         0x5f, 0x6b, 0xcf, 0x79, 0xef, 0x3d, 0xe7, 0x9c, 0xf3, 0x02
       }
     },
     {
       .character = 0x58,
       .width     = 11,
       .height    = 14,
       .xoffset   = 0,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x70, 0xee, 0x38, 0xe6, 0x0d, 0xc1, 0xf0, 0x1e, 0x03, 0x80,
         0x70, 0x1f, 0x03, 0x60, 0xee, 0x18, 0xc7, 0x1d, 0xc1, 0x03
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
         0xe0, 0xee, 0x38, 0xc7, 0x1c, 0xc3, 0xb8, 0x36, 0x07, 0xc0,
         0x70, 0x0e, 0x01, 0xc0, 0x38, 0x07, 0x00, 0xe0, 0x1c, 0x00
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
         0x7f, 0xbf, 0xc0, 0xe0, 0xe0, 0x60, 0x70, 0x30, 0x38, 0x38,
         0x1c, 0x1c, 0x0e, 0x0f, 0xff, 0x3f
       }
     },
     {
       .character = 0x5b,
       .width     = 5,
       .height    = 18,
       .xoffset   = 4,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xf9, 0xce, 0x73, 0x9c, 0xe7, 0x39, 0xce, 0x73, 0x9c,
         0xff, 0x03
       }
     },
     {
       .character = 0x5c,
       .width     = 7,
       .height    = 14,
       .xoffset   = 2,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xc1, 0xc1, 0x83, 0x03, 0x06, 0x0e, 0x0c, 0x18, 0x38, 0x30,
         0x60, 0xe0, 0x03
       }
     },
     {
       .character = 0x5d,
       .width     = 5,
       .height    = 18,
       .xoffset   = 2,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xc6, 0x31, 0x8c, 0x63, 0x18, 0xc6, 0x31, 0x8c, 0x63,
         0xff, 0x03
       }
     },
     {
       .character = 0x5e,
       .width     = 9,
       .height    = 9,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x08, 0x0e, 0x07, 0x06, 0xc3, 0x63, 0x99, 0x8d, 0xc3, 0xc1,
         0x01
       }
     },
     {
       .character = 0x5f,
       .width     = 11,
       .height    = 2,
       .xoffset   = 0,
       .yoffset   = 22,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0x3f
       }
     },
     {
       .character = 0x60,
       .width     = 4,
       .height    = 3,
       .xoffset   = 3,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xe6, 0x03
       }
     },
     {
       .character = 0x61,
       .width     = 9,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3e, 0x3f, 0x98, 0xe0, 0x73, 0xfb, 0xff, 0x8f, 0xc7, 0xe7,
         0xfe, 0xde, 0x03
       }
     },
     {
       .character = 0x62,
       .width     = 9,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xe0, 0x70, 0x38, 0x1c, 0x0e, 0xf7, 0xfb, 0xcf, 0xc7, 0xe1,
         0xf0, 0xf8, 0x7c, 0x7f, 0x3f, 0xfb, 0x3c
       }
     },
     {
       .character = 0x63,
       .width     = 9,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1f, 0x9f, 0x9e, 0x4c, 0x0e, 0x07, 0x03, 0x81, 0xc0, 0x79,
         0x3f, 0x87, 0x06
       }
     },
     {
       .character = 0x64,
       .width     = 9,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x01, 0x80, 0xc0, 0x60, 0x33, 0xdb, 0xff, 0xcf, 0xc7, 0xe1,
         0xe0, 0xf8, 0x7c, 0x7f, 0x3b, 0xec, 0x73
       }
     },
     {
       .character = 0x65,
       .width     = 9,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3e, 0x3f, 0x9c, 0xfc, 0x3f, 0xff, 0xff, 0x81, 0xc0, 0x71,
         0xbf, 0xc7, 0x06
       }
     },
     {
       .character = 0x66,
       .width     = 10,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0f, 0xc7, 0xe1, 0xc4, 0x70, 0x7f, 0xbf, 0xe1, 0xc0, 0x70,
         0x1c, 0x07, 0x01, 0xc0, 0x70, 0x1c, 0x07, 0x01, 0x30
       }
     },
     {
       .character = 0x67,
       .width     = 9,
       .height    = 16,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x39, 0xbe, 0xfd, 0xfc, 0x7e, 0x1e, 0x0f, 0x87, 0xc7, 0xf3,
         0xbf, 0xce, 0xe0, 0x70, 0x3f, 0x3f, 0xfc, 0xfc
       }
     },
     {
       .character = 0x68,
       .width     = 9,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xe0, 0x70, 0x38, 0x1c, 0x0e, 0xf7, 0xff, 0xcf, 0xc7, 0xe1,
         0xf0, 0xf8, 0x7c, 0x3e, 0x1f, 0x0f, 0x43
       }
     },
     {
       .character = 0x69,
       .width     = 9,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0c, 0x0f, 0x03, 0x00, 0x07, 0xc3, 0xe0, 0x30, 0x18, 0x0c,
         0x06, 0x03, 0x01, 0x80, 0xc7, 0xff, 0x7f
       }
     },
     {
       .character = 0x6a,
       .width     = 7,
       .height    = 20,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x0e, 0x1c, 0x38, 0x07, 0xef, 0xc3, 0x87, 0x0e, 0x1c, 0x38,
         0x70, 0xe1, 0xc3, 0x87, 0x0f, 0xff, 0xf7, 0x0c
       }
     },
     {
       .character = 0x6b,
       .width     = 10,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xe0, 0x38, 0x0e, 0x03, 0x80, 0xe3, 0xf9, 0xce, 0x73, 0xb8,
         0xfc, 0x3f, 0x8f, 0xe3, 0x9c, 0xe3, 0x38, 0xee, 0x07
       }
     },
     {
       .character = 0x6c,
       .width     = 9,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x7c, 0x3e, 0x07, 0x03, 0x81, 0xc0, 0xe0, 0x70, 0x38, 0x1c,
         0x0e, 0x07, 0x03, 0x81, 0xc7, 0xff, 0x7f
       }
     },
     {
       .character = 0x6d,
       .width     = 10,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xbb, 0xbf, 0xed, 0xdf, 0x77, 0xdd, 0xf7, 0x7d, 0xdf, 0x77,
         0xdd, 0xf7, 0x7d, 0x37
       }
     },
     {
       .character = 0x6e,
       .width     = 9,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xcf, 0x7f, 0xfc, 0xfc, 0x7e, 0x1f, 0x0f, 0x87, 0xc3, 0xe1,
         0xf0, 0xf8, 0x03
       }
     },
     {
       .character = 0x6f,
       .width     = 9,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3e, 0x3f, 0xbc, 0xfc, 0x7e, 0x1e, 0x0f, 0x87, 0xc7, 0xf3,
         0xbf, 0x8f, 0x04
       }
     },
     {
       .character = 0x70,
       .width     = 9,
       .height    = 16,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xcf, 0x7f, 0xbc, 0xfc, 0x7e, 0x1f, 0x0f, 0x87, 0xc7, 0xf3,
         0xff, 0xbb, 0x9c, 0x0e, 0x07, 0x03, 0x81, 0xc0
       }
     },
     {
       .character = 0x71,
       .width     = 9,
       .height    = 16,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3d, 0xbf, 0xfd, 0xfc, 0x7e, 0x1f, 0x0f, 0x87, 0xc7, 0xf3,
         0xbf, 0xce, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03
       }
     },
     {
       .character = 0x72,
       .width     = 10,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xfb, 0xff, 0xe3, 0xe4, 0xf0, 0x38, 0x0e, 0x03, 0x80, 0xe0,
         0x38, 0x3f, 0xcf, 0x3c
       }
     },
     {
       .character = 0x73,
       .width     = 9,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x3f, 0x3f, 0xdc, 0xce, 0x07, 0xc1, 0xf8, 0x3c, 0x07, 0xe3,
         0xff, 0x9f, 0x04
       }
     },
     {
       .character = 0x74,
       .width     = 9,
       .height    = 14,
       .xoffset   = 1,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x18, 0x0c, 0x06, 0x1f, 0xff, 0xf9, 0xc0, 0xe0, 0x70, 0x38,
         0x1c, 0x0e, 0x07, 0x91, 0xf8, 0x1f
       }
     },
     {
       .character = 0x75,
       .width     = 9,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xe3, 0xf1, 0xf8, 0xfc, 0x7e, 0x3f, 0x1f, 0x8f, 0xc7, 0xe7,
         0xfe, 0xde, 0x03
       }
     },
     {
       .character = 0x76,
       .width     = 11,
       .height    = 11,
       .xoffset   = 0,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xe0, 0xee, 0x19, 0xc7, 0x18, 0xe3, 0x98, 0x77, 0x06, 0xe0,
         0xd8, 0x1f, 0x01, 0xe0, 0x38, 0x00
       }
     },
     {
       .character = 0x77,
       .width     = 11,
       .height    = 11,
       .xoffset   = 0,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xe0, 0x7c, 0x0f, 0xb9, 0xb7, 0x76, 0xee, 0xd5, 0x9a, 0xf3,
         0xde, 0x7b, 0xcf, 0x79, 0xe7, 0x00
       }
     },
     {
       .character = 0x78,
       .width     = 11,
       .height    = 11,
       .xoffset   = 0,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x71, 0xef, 0x38, 0xee, 0x0f, 0xc1, 0xf0, 0x1c, 0x07, 0xc0,
         0xfc, 0x3b, 0x8e, 0x3b, 0xc7, 0x01
       }
     },
     {
       .character = 0x79,
       .width     = 11,
       .height    = 16,
       .xoffset   = 0,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xe0, 0xee, 0x19, 0xc7, 0x18, 0xe3, 0x98, 0x77, 0x06, 0xe0,
         0xf8, 0x1f, 0x01, 0xe0, 0x38, 0x07, 0x00, 0xe0, 0xf8, 0x1f,
         0x03, 0x80
       }
     },
     {
       .character = 0x7a,
       .width     = 9,
       .height    = 11,
       .xoffset   = 1,
       .yoffset   = 9,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xc1, 0xc0, 0xe0, 0xe0, 0xe0, 0xe0, 0x70, 0x70,
         0x7f, 0xff, 0x07
       }
     },
     {
       .character = 0x7b,
       .width     = 6,
       .height    = 17,
       .xoffset   = 3,
       .yoffset   = 6,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x1c, 0xf3, 0x8c, 0x30, 0xc3, 0x1c, 0xe3, 0x87, 0x0c, 0x30,
         0xc3, 0x0f, 0x0f
       }
     },
     {
       .character = 0x7c,
       .width     = 2,
       .height    = 20,
       .xoffset   = 5,
       .yoffset   = 5,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff, 0xff, 0xff
       }
     },
     {
       .character = 0x7d,
       .width     = 6,
       .height    = 17,
       .xoffset   = 3,
       .yoffset   = 7,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0xe3, 0x87, 0x1c, 0x71, 0xc7, 0x0f, 0x3d, 0xc7, 0x1c, 0x71,
         0xc7, 0x38, 0x30
       }
     },
     {
       .character = 0x7e,
       .width     = 9,
       .height    = 3,
       .xoffset   = 1,
       .yoffset   = 12,
       .xadvance  = 11,
       .data      = (const uint8_t[]){
         0x7c, 0xff, 0xe3, 0x06
       }
     }
  }
};
