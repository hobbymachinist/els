#include "fonts.h"

const tft_font_t noto_sans_mono_bold_20 = {
  .size = 20,
  .yoffset = 9,
  .glyph_count = 95,
  .glyphs = (const tft_font_glyph_t[]){
     {
       .character = 0x20,
       .width     = 1,
       .height    = 1,
       .xoffset   = 0,
       .yoffset   = 28,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x00
       }
     },
     {
       .character = 0x21,
       .width     = 5,
       .height    = 19,
       .xoffset   = 6,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf7, 0xbd, 0xef, 0x7b, 0xde, 0xf7, 0xbd, 0xef, 0x01, 0xdf,
         0xff, 0x6e
       }
     },
     {
       .character = 0x22,
       .width     = 10,
       .height    = 7,
       .xoffset   = 3,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x73, 0xdc, 0xe7, 0x39, 0xce, 0x73, 0x9c, 0xe7, 0x0e
       }
     },
     {
       .character = 0x23,
       .width     = 15,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x07, 0x38, 0x0e, 0x60, 0x18, 0xc0, 0x33, 0x80, 0x67, 0x0f,
         0xff, 0xdf, 0xff, 0xbf, 0xff, 0x0e, 0x60, 0x18, 0xc0, 0x33,
         0x87, 0xff, 0xef, 0xff, 0xdf, 0xff, 0x87, 0x30, 0x0c, 0x60,
         0x19, 0xc0, 0x33, 0x80, 0xe7, 0x00
       }
     },
     {
       .character = 0x24,
       .width     = 12,
       .height    = 22,
       .xoffset   = 2,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x06, 0x00, 0x60, 0x1f, 0xc7, 0xff, 0x7f, 0xef, 0x66, 0xf6,
         0x0f, 0x60, 0xfe, 0x07, 0xe0, 0x3f, 0x81, 0xfe, 0x07, 0xe0,
         0x7f, 0x06, 0xf0, 0x6f, 0xc6, 0xff, 0xfe, 0xff, 0xc7, 0xf8,
         0x06, 0x00, 0x60
       }
     },
     {
       .character = 0x25,
       .width     = 16,
       .height    = 19,
       .xoffset   = 0,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x3c, 0x1c, 0x7e, 0x18, 0xff, 0x38, 0xe7, 0x30, 0xe7, 0x70,
         0xe7, 0x60, 0xff, 0xe0, 0x7e, 0xc0, 0x3d, 0xc0, 0x01, 0x80,
         0x03, 0xbc, 0x03, 0x7e, 0x07, 0xff, 0x06, 0xe7, 0x0e, 0xe7,
         0x0c, 0xe7, 0x1c, 0xff, 0x18, 0x7e, 0x38, 0x3c
       }
     },
     {
       .character = 0x26,
       .width     = 15,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0f, 0x00, 0x3f, 0x00, 0xff, 0x01, 0xce, 0x03, 0x9c, 0x07,
         0x38, 0x0e, 0xe0, 0x0f, 0xc0, 0x1f, 0x00, 0x3c, 0x78, 0xfc,
         0xf3, 0xfd, 0xcf, 0x3f, 0x9e, 0x3f, 0x3c, 0x7c, 0x7c, 0xf8,
         0xff, 0xf8, 0xff, 0xf0, 0xfc, 0x1e
       }
     },
     {
       .character = 0x27,
       .width     = 4,
       .height    = 7,
       .xoffset   = 6,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xf6, 0x06
       }
     },
     {
       .character = 0x28,
       .width     = 8,
       .height    = 23,
       .xoffset   = 5,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0e, 0x1e, 0x3c, 0x3c, 0x78, 0x78, 0x78, 0xf0, 0xf0, 0xf0,
         0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x78, 0x78, 0x78, 0x3c,
         0x3c, 0x1e, 0x0e
       }
     },
     {
       .character = 0x29,
       .width     = 8,
       .height    = 23,
       .xoffset   = 4,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x70, 0x78, 0x3c, 0x3c, 0x1e, 0x1e, 0x1e, 0x0f, 0x0f, 0x0f,
         0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x1e, 0x1e, 0x1e, 0x3c,
         0x3c, 0x78, 0x70
       }
     },
     {
       .character = 0x2a,
       .width     = 12,
       .height    = 12,
       .xoffset   = 2,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x7f, 0xf7, 0xff, 0x7f,
         0xf0, 0xf8, 0x1f, 0xc1, 0xfc, 0x3d, 0xe3, 0xde
       }
     },
     {
       .character = 0x2b,
       .width     = 12,
       .height    = 13,
       .xoffset   = 2,
       .yoffset   = 13,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x0f, 0xff, 0xff,
         0xff, 0xff, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00
       }
     },
     {
       .character = 0x2c,
       .width     = 5,
       .height    = 8,
       .xoffset   = 5,
       .yoffset   = 25,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x3b, 0xde, 0xf7, 0x3b, 0xdc
       }
     },
     {
       .character = 0x2d,
       .width     = 8,
       .height    = 3,
       .xoffset   = 4,
       .yoffset   = 20,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff
       }
     },
     {
       .character = 0x2e,
       .width     = 5,
       .height    = 5,
       .xoffset   = 6,
       .yoffset   = 24,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x77, 0xff, 0xf7, 0x00
       }
     },
     {
       .character = 0x2f,
       .width     = 10,
       .height    = 19,
       .xoffset   = 3,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x01, 0xc0, 0xf0, 0x38, 0x0e, 0x07, 0x81, 0xc0, 0xf0, 0x38,
         0x0e, 0x07, 0x81, 0xc0, 0x70, 0x3c, 0x0e, 0x07, 0x81, 0xc0,
         0x70, 0x3c, 0x0e, 0x00
       }
     },
     {
       .character = 0x30,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0f, 0x80, 0xff, 0x87, 0xfe, 0x1e, 0x3c, 0x78, 0xf3, 0xc3,
         0xef, 0x1f, 0xbc, 0xfe, 0xf3, 0x7b, 0xdd, 0xef, 0x67, 0xbf,
         0x1e, 0xfc, 0x7b, 0xe1, 0xe7, 0x8f, 0x1e, 0x3c, 0x3f, 0xf0,
         0xff, 0x80, 0xf8, 0x00
       }
     },
     {
       .character = 0x31,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x03, 0xc0, 0x3f, 0x03, 0xfc, 0x0f, 0xf0, 0x3f, 0xc0, 0xcf,
         0x00, 0x3c, 0x00, 0xf0, 0x03, 0xc0, 0x0f, 0x00, 0x3c, 0x00,
         0xf0, 0x03, 0xc0, 0x0f, 0x00, 0x3c, 0x00, 0xf0, 0x3f, 0xf8,
         0xff, 0xe3, 0xff, 0x02
       }
     },
     {
       .character = 0x32,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0f, 0xe0, 0xff, 0xc3, 0xff, 0x8e, 0x1f, 0x00, 0x3c, 0x00,
         0xf0, 0x03, 0xc0, 0x1f, 0x00, 0x78, 0x03, 0xe0, 0x1f, 0x00,
         0xf8, 0x07, 0xc0, 0x3e, 0x01, 0xf0, 0x0f, 0x00, 0x7f, 0xfd,
         0xff, 0xf7, 0xff, 0x03
       }
     },
     {
       .character = 0x33,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0f, 0xc0, 0xff, 0xc3, 0xff, 0x8c, 0x3e, 0x00, 0x78, 0x01,
         0xe0, 0x0f, 0x03, 0xf8, 0x0f, 0xc0, 0x3f, 0xc0, 0x0f, 0x00,
         0x1e, 0x00, 0x78, 0x01, 0xe0, 0x07, 0x98, 0x3e, 0x7f, 0xf1,
         0xff, 0x83, 0xf8, 0x00
       }
     },
     {
       .character = 0x34,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x00, 0xf0, 0x07, 0xc0, 0x3f, 0x00, 0xfc, 0x07, 0xf0, 0x3b,
         0xc0, 0xef, 0x07, 0x3c, 0x3c, 0xf0, 0xe3, 0xc7, 0x8f, 0x1c,
         0x3c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x3c, 0x00, 0xf0,
         0x03, 0xc0, 0x0f, 0x00
       }
     },
     {
       .character = 0x35,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x7f, 0xe1, 0xff, 0x8f, 0xfe, 0x3e, 0x00, 0xf0, 0x03, 0xc0,
         0x0f, 0x00, 0x3f, 0xe0, 0xff, 0xe3, 0xff, 0x80, 0x1f, 0x00,
         0x3c, 0x00, 0xf0, 0x03, 0xc0, 0x0f, 0x30, 0x78, 0xff, 0xe3,
         0xff, 0x07, 0xfc, 0x00
       }
     },
     {
       .character = 0x36,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x03, 0xf0, 0x3f, 0xc3, 0xff, 0x0f, 0x80, 0x78, 0x01, 0xc0,
         0x0f, 0x3c, 0x3f, 0xfc, 0xff, 0xf3, 0xe3, 0xef, 0x07, 0xbc,
         0x1e, 0xf0, 0x7b, 0xc1, 0xe7, 0x07, 0x9e, 0x3c, 0x3f, 0xf0,
         0x7f, 0x80, 0xfc, 0x00
       }
     },
     {
       .character = 0x37,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xfb, 0xff, 0xef, 0xff, 0x80, 0x1e, 0x00, 0xf0, 0x03,
         0xc0, 0x1e, 0x00, 0x78, 0x03, 0xc0, 0x0f, 0x00, 0x3c, 0x01,
         0xe0, 0x07, 0x80, 0x3c, 0x00, 0xf0, 0x07, 0x80, 0x1e, 0x00,
         0x78, 0x03, 0xc0, 0x00
       }
     },
     {
       .character = 0x38,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x1f, 0x81, 0xff, 0x8f, 0xff, 0x3e, 0x7c, 0xf0, 0xf3, 0xc3,
         0xc7, 0x9f, 0x1f, 0xf8, 0x1f, 0xc0, 0x7f, 0x07, 0xfe, 0x1e,
         0x7c, 0xf0, 0xfb, 0xc1, 0xef, 0x07, 0xbe, 0x3e, 0x7f, 0xf1,
         0xff, 0x81, 0xf8, 0x00
       }
     },
     {
       .character = 0x39,
       .width     = 13,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x1f, 0x81, 0xfe, 0x1f, 0xf9, 0xf1, 0xef, 0x07, 0x78, 0x3f,
         0xc1, 0xfe, 0x0f, 0xf8, 0xfb, 0xff, 0xdf, 0xfe, 0x3c, 0xf0,
         0x07, 0x80, 0x78, 0x03, 0xc0, 0x7c, 0x7f, 0xe3, 0xfc, 0x1f,
         0x40
       }
     },
     {
       .character = 0x3a,
       .width     = 5,
       .height    = 15,
       .xoffset   = 6,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x77, 0xff, 0xf7, 0x00, 0x00, 0x00, 0x1d, 0xff, 0xfd, 0x06
       }
     },
     {
       .character = 0x3b,
       .width     = 6,
       .height    = 20,
       .xoffset   = 5,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x39, 0xf7, 0xdf, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79,
         0xe7, 0x9e, 0x71, 0xcf, 0x38
       }
     },
     {
       .character = 0x3c,
       .width     = 12,
       .height    = 13,
       .xoffset   = 2,
       .yoffset   = 13,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x00, 0x30, 0x0f, 0x03, 0xf0, 0xfc, 0x3f, 0x0f, 0xc0, 0xf0,
         0x0f, 0xc0, 0x3f, 0x80, 0xfe, 0x03, 0xf0, 0x0f, 0x00, 0x03
       }
     },
     {
       .character = 0x3d,
       .width     = 12,
       .height    = 9,
       .xoffset   = 2,
       .yoffset   = 15,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0xff,
         0xff, 0xff, 0xff, 0x0f
       }
     },
     {
       .character = 0x3e,
       .width     = 12,
       .height    = 13,
       .xoffset   = 2,
       .yoffset   = 13,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xc0, 0x0f, 0x00, 0xfc, 0x03, 0xf0, 0x0f, 0xc0, 0x3f, 0x01,
         0xf0, 0x7e, 0x1f, 0x87, 0xe0, 0xf8, 0x0e, 0x00, 0x80, 0x00
       }
     },
     {
       .character = 0x3f,
       .width     = 12,
       .height    = 19,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x1f, 0x87, 0xfe, 0x7f, 0xf6, 0x1f, 0x00, 0xf0, 0x0f, 0x01,
         0xf0, 0x3e, 0x0f, 0xc1, 0xf0, 0x1e, 0x01, 0xe0, 0x00, 0x00,
         0x00, 0x0e, 0x01, 0xf0, 0x1f, 0x01, 0xf0, 0x0e, 0x00
       }
     },
     {
       .character = 0x40,
       .width     = 16,
       .height    = 21,
       .xoffset   = 0,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x03, 0xe0, 0x0f, 0xf8, 0x1e, 0x1c, 0x38, 0x0e, 0x78, 0x0e,
         0x73, 0xe6, 0x77, 0xf7, 0xe7, 0xf7, 0xef, 0x77, 0xee, 0x77,
         0xee, 0x77, 0xee, 0x77, 0xee, 0x77, 0xef, 0xfe, 0xe7, 0xfe,
         0x73, 0x9c, 0x70, 0x00, 0x38, 0x00, 0x3c, 0x08, 0x1f, 0xf8,
         0x07, 0xf0
       }
     },
     {
       .character = 0x41,
       .width     = 16,
       .height    = 19,
       .xoffset   = 0,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x0f, 0xf0,
         0x0e, 0x70, 0x0e, 0x70, 0x1e, 0x78, 0x1e, 0x78, 0x1c, 0x38,
         0x1c, 0x38, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x78, 0x1c,
         0x78, 0x1e, 0x78, 0x1e, 0x70, 0x0e, 0xf0, 0x0f
       }
     },
     {
       .character = 0x42,
       .width     = 13,
       .height    = 19,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0x87, 0xff, 0x3f, 0xfd, 0xe1, 0xef, 0x0f, 0x78, 0x7b,
         0xc3, 0xdf, 0xfc, 0xff, 0xc7, 0xff, 0xbc, 0x3d, 0xe0, 0xff,
         0x07, 0xf8, 0x3f, 0xc1, 0xfe, 0x1f, 0xff, 0xf7, 0xff, 0x3f,
         0x78
       }
     },
     {
       .character = 0x43,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x03, 0xf8, 0x3f, 0xe3, 0xff, 0x8f, 0x80, 0x78, 0x01, 0xe0,
         0x0f, 0x00, 0x3c, 0x00, 0xf0, 0x03, 0xc0, 0x0f, 0x00, 0x3c,
         0x00, 0xf0, 0x03, 0xe0, 0x07, 0x80, 0x1f, 0x00, 0x3f, 0xf8,
         0x7f, 0xe0, 0x7f, 0x02
       }
     },
     {
       .character = 0x44,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0x83, 0xff, 0x8f, 0xff, 0x3c, 0x3e, 0xf0, 0x7b, 0xc1,
         0xff, 0x03, 0xfc, 0x0f, 0xf0, 0x3f, 0xc0, 0xff, 0x03, 0xfc,
         0x0f, 0xf0, 0x3f, 0xc1, 0xef, 0x07, 0xbc, 0x3c, 0xff, 0xf3,
         0xff, 0x8f, 0xf8, 0x00
       }
     },
     {
       .character = 0x45,
       .width     = 12,
       .height    = 19,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xf0, 0x0f, 0x00, 0xf0,
         0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0f, 0x00, 0xf0, 0x0f,
         0x00, 0xf0, 0x0f, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f
       }
     },
     {
       .character = 0x46,
       .width     = 12,
       .height    = 19,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xf0, 0x0f, 0x00, 0xf0,
         0x0f, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xf0, 0x0f,
         0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x00
       }
     },
     {
       .character = 0x47,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x07, 0xfc, 0x7f, 0xf3, 0xff, 0xcf, 0x80, 0x78, 0x01, 0xe0,
         0x0f, 0x00, 0x3c, 0x00, 0xf0, 0xff, 0xc3, 0xff, 0x0f, 0xfc,
         0x0f, 0xf0, 0x3f, 0xe0, 0xf7, 0x83, 0xdf, 0x0f, 0x3f, 0xfc,
         0x7f, 0xf0, 0x7f, 0x02
       }
     },
     {
       .character = 0x48,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x3f, 0xc0, 0xff, 0x03, 0xfc, 0x0f, 0xf0, 0x3f, 0xc0,
         0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xfc,
         0x0f, 0xf0, 0x3f, 0xc0, 0xff, 0x03, 0xfc, 0x0f, 0xf0, 0x3f,
         0xc0, 0xff, 0x03, 0x03
       }
     },
     {
       .character = 0x49,
       .width     = 11,
       .height    = 19,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff, 0xff, 0x87, 0x80, 0xf0, 0x1e, 0x03, 0xc0,
         0x78, 0x0f, 0x01, 0xe0, 0x3c, 0x07, 0x80, 0xf0, 0x1e, 0x03,
         0xc0, 0x78, 0xff, 0xff, 0xff, 0xff, 0x01
       }
     },
     {
       .character = 0x4a,
       .width     = 11,
       .height    = 19,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x01, 0xe0, 0x3c, 0x07, 0x80, 0xf0, 0x1e, 0x03, 0xc0, 0x78,
         0x0f, 0x01, 0xe0, 0x3c, 0x07, 0x80, 0xf0, 0x1e, 0x03, 0xc0,
         0x78, 0x1e, 0xff, 0xdf, 0xf3, 0xfc, 0x00
       }
     },
     {
       .character = 0x4b,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x3f, 0xc1, 0xef, 0x0f, 0x3c, 0x3c, 0xf1, 0xe3, 0xcf,
         0x0f, 0x7c, 0x3d, 0xe0, 0xff, 0x03, 0xfe, 0x0f, 0xf8, 0x3e,
         0xf0, 0xf3, 0xc3, 0xc7, 0x8f, 0x1e, 0x3c, 0x3c, 0xf0, 0xfb,
         0xc1, 0xef, 0x07, 0x03
       }
     },
     {
       .character = 0x4c,
       .width     = 12,
       .height    = 19,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0,
         0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f,
         0x00, 0xf0, 0x0f, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f
       }
     },
     {
       .character = 0x4d,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xfc, 0x7f, 0xf1, 0xff, 0xc7, 0xff, 0x1f, 0xfe, 0x7f, 0xfb,
         0xff, 0xef, 0xff, 0xb7, 0xf6, 0xdf, 0xdb, 0x7f, 0x7d, 0xfd,
         0xf7, 0xf7, 0xdf, 0xce, 0x7f, 0x01, 0xfc, 0x07, 0xf0, 0x1f,
         0xc0, 0x7f, 0x01, 0x03
       }
     },
     {
       .character = 0x4e,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xfc, 0x3f, 0xf0, 0xff, 0xc3, 0xff, 0x8f, 0xfe, 0x3f, 0xd8,
         0xff, 0x73, 0xfd, 0xcf, 0xf7, 0x3f, 0xcc, 0xff, 0x3b, 0xfc,
         0xef, 0xf3, 0xbf, 0xc6, 0xff, 0x1f, 0xfc, 0x7f, 0xf0, 0xff,
         0xc3, 0xff, 0x0f, 0x03
       }
     },
     {
       .character = 0x4f,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0f, 0xc0, 0x7f, 0x83, 0xff, 0x1f, 0x3e, 0x78, 0x7b, 0xc0,
         0xff, 0x03, 0xfc, 0x0f, 0xf0, 0x3f, 0xc0, 0xff, 0x03, 0xfc,
         0x0f, 0xf0, 0x3f, 0xc0, 0xf7, 0x87, 0x9f, 0x3e, 0x3f, 0xf0,
         0x7f, 0x80, 0xfc, 0x00
       }
     },
     {
       .character = 0x50,
       .width     = 13,
       .height    = 19,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xc7, 0xff, 0x3f, 0xfd, 0xe1, 0xff, 0x07, 0xf8, 0x3f,
         0xc1, 0xfe, 0x0f, 0xf0, 0xff, 0xff, 0xbf, 0xf9, 0xff, 0x8f,
         0x00, 0x78, 0x03, 0xc0, 0x1e, 0x00, 0xf0, 0x07, 0x80, 0x3c,
         0x00
       }
     },
     {
       .character = 0x51,
       .width     = 14,
       .height    = 24,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0f, 0xc0, 0x7f, 0x83, 0xff, 0x1f, 0x3e, 0x78, 0x7b, 0xc0,
         0xff, 0x03, 0xfc, 0x0f, 0xf0, 0x3f, 0xc0, 0xff, 0x03, 0xfc,
         0x0f, 0xf0, 0x3f, 0xc0, 0xf7, 0x87, 0x9f, 0x3e, 0x3f, 0xf0,
         0x7f, 0x80, 0xfe, 0x00, 0x38, 0x00, 0xf0, 0x03, 0xc0, 0x07,
         0x80, 0x1e
       }
     },
     {
       .character = 0x52,
       .width     = 13,
       .height    = 19,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0x07, 0xfe, 0x3f, 0xf9, 0xe3, 0xef, 0x0f, 0x78, 0x7b,
         0xc3, 0xde, 0x1e, 0xf1, 0xe7, 0xff, 0x3f, 0xf1, 0xff, 0x0f,
         0x38, 0x79, 0xe3, 0xc7, 0x9e, 0x3c, 0xf0, 0xf7, 0x87, 0xfc,
         0x0f
       }
     },
     {
       .character = 0x53,
       .width     = 13,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0f, 0xc1, 0xff, 0x8f, 0xfc, 0xf8, 0x67, 0x80, 0x3c, 0x01,
         0xf0, 0x07, 0xc0, 0x3f, 0x80, 0x7c, 0x03, 0xf8, 0x07, 0xc0,
         0x1f, 0x00, 0x78, 0x03, 0xcc, 0x3e, 0x7f, 0xe3, 0xff, 0x07,
         0x70
       }
     },
     {
       .character = 0x54,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xe0, 0x07, 0x80, 0x1e,
         0x00, 0x78, 0x01, 0xe0, 0x07, 0x80, 0x1e, 0x00, 0x78, 0x01,
         0xe0, 0x07, 0x80, 0x1e, 0x00, 0x78, 0x01, 0xe0, 0x07, 0x80,
         0x1e, 0x00, 0x78, 0x00
       }
     },
     {
       .character = 0x55,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x3f, 0xc0, 0xff, 0x03, 0xfc, 0x0f, 0xf0, 0x3f, 0xc0,
         0xff, 0x03, 0xfc, 0x0f, 0xf0, 0x3f, 0xc0, 0xff, 0x03, 0xfc,
         0x0f, 0xf0, 0x3f, 0xc0, 0xff, 0x03, 0xde, 0x1e, 0x7f, 0xf8,
         0xff, 0xc0, 0xfc, 0x00
       }
     },
     {
       .character = 0x56,
       .width     = 16,
       .height    = 19,
       .xoffset   = 0,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x0f, 0xf8, 0x1f, 0xf8, 0x1f, 0x78, 0x1e, 0x38, 0x1c,
         0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x1c, 0x38, 0x1e, 0x78,
         0x1e, 0x78, 0x0e, 0x70, 0x0e, 0x70, 0x0f, 0x70, 0x07, 0xe0,
         0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x03, 0xc0
       }
     },
     {
       .character = 0x57,
       .width     = 15,
       .height    = 19,
       .xoffset   = 0,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xe0, 0x0f, 0xc0, 0x1f, 0x80, 0x3f, 0x00, 0x7f, 0x39, 0xfe,
         0xfb, 0xfd, 0xf7, 0xbb, 0xee, 0x77, 0xdc, 0xed, 0xb9, 0xdb,
         0x73, 0xb7, 0xe7, 0x6f, 0xcf, 0xdf, 0x9f, 0xbf, 0x3f, 0x7e,
         0x7c, 0x7c, 0xf8, 0xf9, 0xf1, 0x1e
       }
     },
     {
       .character = 0x58,
       .width     = 14,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x3d, 0xe1, 0xe7, 0x87, 0x8f, 0x3c, 0x1c, 0xe0, 0x7f,
         0x80, 0xfc, 0x03, 0xf0, 0x07, 0x80, 0x1e, 0x00, 0xfc, 0x03,
         0xf0, 0x1f, 0xe0, 0x73, 0x83, 0xcf, 0x0e, 0x1c, 0x78, 0x7b,
         0xc0, 0xff, 0x03, 0x03
       }
     },
     {
       .character = 0x59,
       .width     = 16,
       .height    = 19,
       .xoffset   = 0,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x78, 0x1e, 0x78, 0x1e, 0x3c, 0x3c, 0x3c, 0x3c, 0x1c, 0x38,
         0x1e, 0x78, 0x0e, 0x70, 0x0f, 0xf0, 0x07, 0xf0, 0x07, 0xe0,
         0x07, 0xe0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0,
         0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0
       }
     },
     {
       .character = 0x5a,
       .width     = 12,
       .height    = 19,
       .xoffset   = 2,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x7f, 0xf7, 0xff, 0x7f, 0xf0, 0x1e, 0x01, 0xe0, 0x3c, 0x03,
         0xc0, 0x78, 0x07, 0x80, 0xf0, 0x1f, 0x01, 0xe0, 0x3c, 0x03,
         0xc0, 0x78, 0x07, 0x80, 0xff, 0xff, 0xff, 0xff, 0x0f
       }
     },
     {
       .character = 0x5b,
       .width     = 7,
       .height    = 24,
       .xoffset   = 6,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
         0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7, 0x8f, 0x1f, 0xff,
         0xff
       }
     },
     {
       .character = 0x5c,
       .width     = 10,
       .height    = 19,
       .xoffset   = 3,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xe0, 0x3c, 0x07, 0x01, 0xc0, 0x78, 0x0e, 0x03, 0xc0, 0x70,
         0x1c, 0x07, 0x80, 0xe0, 0x38, 0x0f, 0x01, 0xc0, 0x78, 0x0e,
         0x03, 0x80, 0xf0, 0x07
       }
     },
     {
       .character = 0x5d,
       .width     = 7,
       .height    = 24,
       .xoffset   = 3,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xf8, 0xf1, 0xe3, 0xc7, 0x8f, 0x1e, 0x3c, 0x78,
         0xf1, 0xe3, 0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xff, 0xff,
         0xff
       }
     },
     {
       .character = 0x5e,
       .width     = 14,
       .height    = 12,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x03, 0x00, 0x1e, 0x00, 0x78, 0x03, 0xf0, 0x0f, 0xc0, 0x77,
         0x81, 0xce, 0x0e, 0x3c, 0x38, 0x71, 0xc0, 0xe7, 0x03, 0xb8,
         0x07
       }
     },
     {
       .character = 0x5f,
       .width     = 16,
       .height    = 3,
       .xoffset   = 0,
       .yoffset   = 31,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff
       }
     },
     {
       .character = 0x60,
       .width     = 6,
       .height    = 5,
       .xoffset   = 4,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf3, 0xc7, 0x8f, 0x07
       }
     },
     {
       .character = 0x61,
       .width     = 13,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x3f, 0xc1, 0xff, 0x8f, 0xfc, 0x01, 0xf0, 0x07, 0x8f, 0xfd,
         0xff, 0xff, 0xff, 0xf8, 0x7f, 0x83, 0xfc, 0x1f, 0xe1, 0xff,
         0xff, 0xbf, 0xfc, 0xf8, 0x07
       }
     },
     {
       .character = 0x62,
       .width     = 13,
       .height    = 21,
       .xoffset   = 1,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x07, 0x80, 0x3c, 0x01, 0xe0, 0x0f, 0x00, 0x78, 0x03,
         0xcf, 0x1f, 0xfc, 0xff, 0xf7, 0xc7, 0xbc, 0x1f, 0xe0, 0xff,
         0x07, 0xf8, 0x3f, 0xc1, 0xfe, 0x0f, 0xf0, 0x7f, 0xc7, 0xbf,
         0xfd, 0xff, 0xce, 0x3c, 0x00
       }
     },
     {
       .character = 0x63,
       .width     = 12,
       .height    = 15,
       .xoffset   = 2,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0f, 0xf3, 0xff, 0x7f, 0xf7, 0xc0, 0xf8, 0x0f, 0x00, 0xf0,
         0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf8, 0x07, 0xc0, 0x7f, 0xf3,
         0xff, 0x0f, 0x0f
       }
     },
     {
       .character = 0x64,
       .width     = 14,
       .height    = 21,
       .xoffset   = 1,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x00, 0x3c, 0x00, 0xf0, 0x03, 0xc0, 0x0f, 0x00, 0x3c, 0x00,
         0xf1, 0xf3, 0xcf, 0xff, 0x7f, 0xfd, 0xe1, 0xff, 0x87, 0xfc,
         0x0f, 0xf0, 0x3f, 0xc0, 0xff, 0x03, 0xfc, 0x0f, 0xf0, 0x3d,
         0xe1, 0xf7, 0xff, 0xcf, 0xef, 0x1f, 0x0f
       }
     },
     {
       .character = 0x65,
       .width     = 14,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0f, 0xc0, 0xff, 0xc7, 0xff, 0x9e, 0x1e, 0xf8, 0x3f, 0xc0,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0x80, 0x1f,
         0x03, 0x3f, 0xfc, 0x7f, 0xf0, 0x7f, 0x00
       }
     },
     {
       .character = 0x66,
       .width     = 13,
       .height    = 21,
       .xoffset   = 2,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x03, 0xf8, 0x3f, 0xc3, 0xfe, 0x1f, 0x00, 0xf0, 0x07, 0x83,
         0xff, 0xdf, 0xfe, 0xff, 0xf0, 0x78, 0x03, 0xc0, 0x1e, 0x00,
         0xf0, 0x07, 0x80, 0x3c, 0x01, 0xe0, 0x0f, 0x00, 0x78, 0x03,
         0xc0, 0x1e, 0x00, 0xf0, 0x00
       }
     },
     {
       .character = 0x67,
       .width     = 14,
       .height    = 21,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x1f, 0x1c, 0xfe, 0xf7, 0xff, 0xde, 0x1f, 0xf8, 0x3f, 0xc0,
         0xff, 0x03, 0xfc, 0x0f, 0xf0, 0x3f, 0xc0, 0xff, 0x83, 0xde,
         0x1f, 0x7f, 0xfc, 0xfe, 0xf1, 0xf3, 0xc0, 0x0f, 0x00, 0x7d,
         0x81, 0xe7, 0xff, 0x9f, 0xfc, 0x3f, 0x30
       }
     },
     {
       .character = 0x68,
       .width     = 13,
       .height    = 21,
       .xoffset   = 1,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x07, 0x80, 0x3c, 0x01, 0xe0, 0x0f, 0x00, 0x78, 0x03,
         0xcf, 0x9e, 0xfe, 0xff, 0xf7, 0xc7, 0xfc, 0x1f, 0xe0, 0xff,
         0x07, 0xf8, 0x3f, 0xc1, 0xfe, 0x0f, 0xf0, 0x7f, 0x83, 0xfc,
         0x1f, 0xe0, 0xff, 0x07, 0x01
       }
     },
     {
       .character = 0x69,
       .width     = 13,
       .height    = 21,
       .xoffset   = 2,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x07, 0x80, 0x3c, 0x01, 0xe0, 0x0f, 0x00, 0x00, 0x00, 0x01,
         0xfe, 0x0f, 0xf0, 0x7f, 0x80, 0x3c, 0x01, 0xe0, 0x0f, 0x00,
         0x78, 0x03, 0xc0, 0x1e, 0x00, 0xf0, 0x07, 0x80, 0x3c, 0x3f,
         0xff, 0xff, 0xff, 0xff, 0x01
       }
     },
     {
       .character = 0x6a,
       .width     = 11,
       .height    = 27,
       .xoffset   = 1,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x01, 0xc0, 0x7c, 0x0f, 0x80, 0xe0, 0x00, 0x00, 0x0f, 0xf9,
         0xff, 0x3f, 0xe0, 0x3c, 0x07, 0x80, 0xf0, 0x1e, 0x03, 0xc0,
         0x78, 0x0f, 0x01, 0xe0, 0x3c, 0x07, 0x80, 0xf0, 0x1e, 0x03,
         0xc0, 0x78, 0x1f, 0xff, 0xdf, 0xf3, 0xfc, 0x00
       }
     },
     {
       .character = 0x6b,
       .width     = 14,
       .height    = 21,
       .xoffset   = 2,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x03, 0xc0, 0x0f, 0x00, 0x3c, 0x00, 0xf0, 0x03, 0xc0,
         0x0f, 0x07, 0xbc, 0x3e, 0xf1, 0xf3, 0xcf, 0x8f, 0x3c, 0x3d,
         0xe0, 0xff, 0x83, 0xfe, 0x0f, 0xfc, 0x3e, 0x78, 0xf1, 0xe3,
         0xc3, 0xcf, 0x0f, 0xbc, 0x1e, 0xf0, 0x0f
       }
     },
     {
       .character = 0x6c,
       .width     = 13,
       .height    = 21,
       .xoffset   = 2,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x7f, 0x83, 0xfc, 0x1f, 0xe0, 0x0f, 0x00, 0x78, 0x03, 0xc0,
         0x1e, 0x00, 0xf0, 0x07, 0x80, 0x3c, 0x01, 0xe0, 0x0f, 0x00,
         0x78, 0x03, 0xc0, 0x1e, 0x00, 0xf0, 0x07, 0x80, 0x3c, 0x3f,
         0xff, 0xff, 0xff, 0xff, 0x01
       }
     },
     {
       .character = 0x6d,
       .width     = 14,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xe7, 0x3b, 0xfd, 0xff, 0xff, 0xfd, 0xef, 0xf7, 0xbf, 0xde,
         0xff, 0x7b, 0xfd, 0xef, 0xf7, 0xbf, 0xde, 0xff, 0x7b, 0xfd,
         0xef, 0xf7, 0xbf, 0xde, 0xff, 0x7b, 0x03
       }
     },
     {
       .character = 0x6e,
       .width     = 13,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xe3, 0xe7, 0xff, 0xbf, 0xfd, 0xf1, 0xff, 0x87, 0xf8, 0x3f,
         0xc1, 0xfe, 0x0f, 0xf0, 0x7f, 0x83, 0xfc, 0x1f, 0xe0, 0xff,
         0x07, 0xf8, 0x3f, 0xc1, 0x07
       }
     },
     {
       .character = 0x6f,
       .width     = 14,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0f, 0xc0, 0xff, 0xc7, 0xff, 0x9e, 0x1e, 0xf8, 0x7f, 0xc0,
         0xff, 0x03, 0xfc, 0x0f, 0xf0, 0x3f, 0xc0, 0xff, 0x87, 0xde,
         0x1e, 0x7f, 0xf8, 0xff, 0xc0, 0xfc, 0x00
       }
     },
     {
       .character = 0x70,
       .width     = 13,
       .height    = 21,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xe3, 0xc7, 0xff, 0x3f, 0xfd, 0xf1, 0xef, 0x07, 0xf8, 0x3f,
         0xc1, 0xfe, 0x0f, 0xf0, 0x7f, 0x83, 0xfc, 0x1f, 0xf1, 0xef,
         0xff, 0x7f, 0xf3, 0xcf, 0x1e, 0x00, 0xf0, 0x07, 0x80, 0x3c,
         0x01, 0xe0, 0x0f, 0x00, 0x00
       }
     },
     {
       .character = 0x71,
       .width     = 14,
       .height    = 21,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x1f, 0x1c, 0xff, 0xf7, 0xff, 0xde, 0x1f, 0xf8, 0x7f, 0xc0,
         0xff, 0x03, 0xfc, 0x0f, 0xf0, 0x3f, 0xc0, 0xff, 0x87, 0xde,
         0x1f, 0x7f, 0xfc, 0xff, 0xf1, 0xf3, 0xc0, 0x0f, 0x00, 0x3c,
         0x00, 0xf0, 0x03, 0xc0, 0x0f, 0x00, 0x0f
       }
     },
     {
       .character = 0x72,
       .width     = 14,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x7e, 0x3d, 0xf9, 0xf7, 0xff, 0x87, 0xfe, 0x0f, 0x80, 0x3e,
         0x00, 0xf0, 0x03, 0xc0, 0x0f, 0x00, 0x3c, 0x00, 0xf0, 0x03,
         0xc0, 0x7f, 0xe3, 0xff, 0x8f, 0xfe, 0x00
       }
     },
     {
       .character = 0x73,
       .width     = 12,
       .height    = 15,
       .xoffset   = 2,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x1f, 0xe7, 0xff, 0xff, 0xff, 0x06, 0xf0, 0x0f, 0xc0, 0x7f,
         0x83, 0xfe, 0x0f, 0xe0, 0x3f, 0x00, 0xfc, 0x0f, 0xff, 0xff,
         0xfe, 0x7f, 0x08
       }
     },
     {
       .character = 0x74,
       .width     = 13,
       .height    = 19,
       .xoffset   = 1,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x0e, 0x00, 0x70, 0x03, 0x80, 0x3c, 0x07, 0xff, 0xff, 0xff,
         0xff, 0xe3, 0xc0, 0x1e, 0x00, 0xf0, 0x07, 0x80, 0x3c, 0x01,
         0xe0, 0x0f, 0x00, 0x78, 0x03, 0xe0, 0x0f, 0xf8, 0x7f, 0xc0,
         0x7f
       }
     },
     {
       .character = 0x75,
       .width     = 13,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x7f, 0x83, 0xfc, 0x1f, 0xe0, 0xff, 0x07, 0xf8, 0x3f,
         0xc1, 0xfe, 0x0f, 0xf0, 0x7f, 0x83, 0xfc, 0x3f, 0xf1, 0xf7,
         0xff, 0xbf, 0xfc, 0xf8, 0x07
       }
     },
     {
       .character = 0x76,
       .width     = 14,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x3f, 0xc0, 0xff, 0x03, 0xde, 0x1e, 0x78, 0x78, 0xe1,
         0xc3, 0xcf, 0x0f, 0x3c, 0x1c, 0xe0, 0x7b, 0x81, 0xfe, 0x03,
         0xf0, 0x0f, 0xc0, 0x3f, 0x00, 0x78, 0x00
       }
     },
     {
       .character = 0x77,
       .width     = 16,
       .height    = 15,
       .xoffset   = 0,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x70, 0x07, 0x70, 0x07, 0x70, 0x07, 0x71, 0xc7, 0x71, 0xc7,
         0x73, 0xe7, 0x73, 0xe7, 0x73, 0xe7, 0x73, 0xe7, 0x73, 0xe7,
         0x73, 0x67, 0x7e, 0x3f, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e
       }
     },
     {
       .character = 0x78,
       .width     = 14,
       .height    = 15,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf8, 0x3d, 0xe1, 0xe7, 0xcf, 0x8f, 0x3c, 0x1f, 0xe0, 0x7f,
         0x80, 0xfc, 0x01, 0xf0, 0x0f, 0xc0, 0x7f, 0x81, 0xff, 0x0f,
         0x3c, 0x7c, 0xf9, 0xe1, 0xef, 0x03, 0x03
       }
     },
     {
       .character = 0x79,
       .width     = 15,
       .height    = 21,
       .xoffset   = 1,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf0, 0x1e, 0xf0, 0x79, 0xe0, 0xf3, 0xc1, 0xe3, 0xc7, 0x87,
         0x8f, 0x0f, 0x1e, 0x0f, 0x38, 0x1e, 0xf0, 0x1d, 0xc0, 0x3b,
         0x80, 0x7f, 0x00, 0x7c, 0x00, 0xf8, 0x00, 0xf0, 0x03, 0xc0,
         0x07, 0x80, 0x1e, 0x01, 0xfc, 0x03, 0xf0, 0x07, 0x80, 0x00
       }
     },
     {
       .character = 0x7a,
       .width     = 13,
       .height    = 15,
       .xoffset   = 2,
       .yoffset   = 14,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xf7, 0xff, 0xbf, 0xfc, 0x03, 0xc0, 0x3c, 0x03, 0xe0,
         0x1e, 0x01, 0xe0, 0x1e, 0x01, 0xf0, 0x0f, 0x00, 0xf0, 0x0f,
         0xff, 0xff, 0xff, 0xff, 0x07
       }
     },
     {
       .character = 0x7b,
       .width     = 9,
       .height    = 24,
       .xoffset   = 4,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x07, 0x87, 0xc7, 0xe3, 0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e,
         0x1f, 0x3f, 0x1e, 0x0f, 0xc1, 0xf0, 0x78, 0x3c, 0x1e, 0x0f,
         0x07, 0x83, 0xc1, 0xe0, 0xfc, 0x3e, 0x0f
       }
     },
     {
       .character = 0x7c,
       .width     = 3,
       .height    = 27,
       .xoffset   = 7,
       .yoffset   = 8,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
         0x01
       }
     },
     {
       .character = 0x7d,
       .width     = 10,
       .height    = 24,
       .xoffset   = 3,
       .yoffset   = 10,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0xf8, 0x3f, 0x0f, 0xe0, 0x78, 0x1e, 0x07, 0x81, 0xe0, 0x78,
         0x1e, 0x07, 0xc0, 0xfc, 0x0f, 0x0f, 0xc7, 0xc1, 0xe0, 0x78,
         0x1e, 0x07, 0x81, 0xe0, 0x78, 0x1e, 0x3f, 0x8f, 0xc3, 0xe0
       }
     },
     {
       .character = 0x7e,
       .width     = 12,
       .height    = 4,
       .xoffset   = 2,
       .yoffset   = 18,
       .xadvance  = 16,
       .data      = (const uint8_t[]){
         0x7c, 0x1f, 0xff, 0xff, 0xf8, 0x3e
       }
     }
  }
};
