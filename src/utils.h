#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

//==============================================================================
// Utils
//==============================================================================
#define MAX(a, b)  ((a) >= (b) ? (a) : (b))
#define MIN(a, b)  ((a) <= (b) ? (a) : (b))
#define CEIL(a)    ((int)((a) + 0.5))
#define SQR(a)     ((a)*(a))

size_t els_sprint_double1(char *text, size_t size, float value, const char *prefix);
size_t els_sprint_double2(char *text, size_t size, float value, const char *prefix);
size_t els_sprint_double3(char *text, size_t size, float value, const char *prefix);
size_t els_sprint_double33(char *text, size_t size, float value, const char *prefix);
size_t els_sprint_double13(char *text, size_t size, float value, const char *prefix);
size_t els_sprint_double23(char *text, size_t size, float value, const char *prefix);
size_t els_sprint_double24(char *text, size_t size, float value, const char *prefix);
uint32_t els_gcd(uint32_t u, uint32_t v);

#ifdef ELS_DEBUG
  #define els_printf(...)  printf(__VA_ARGS__)
#else
  #define els_printf(...)
#endif

#ifdef __cplusplus
}
#endif
