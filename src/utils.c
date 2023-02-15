#include "utils.h"
#include <stdio.h>

static int dround(double value) {
  int ivalue = (int)value;
  if (value - ivalue >= 0.5)
    return ivalue + 1;
  else
    return ivalue;
}


// Binary GCD
// https://lemire.me/blog/2013/12/26/fastest-way-to-compute-the-greatest-common-divisor/
uint32_t els_gcd(uint32_t u, uint32_t v) {
  uint32_t shift;
  if (u == 0)
    return v;
  if (v == 0)
    return u;
  shift = __builtin_ctz(u | v);
  u >>= __builtin_ctz(u);
  do {
    v >>= __builtin_ctz(v);
    if (u > v) {
      uint32_t t = v;
      v = u;
      u = t;
    }
    v = v - u;
  } while (v != 0);
  return u << shift;
}

size_t els_sprint_double1(char *text, size_t size, float value, const char *prefix) {
  if (value >= 0) {
    int ivalue = (int)value;
    int dec = dround((value - ivalue) * 100);
    // deal with floating point errors.
    if (dec > 99) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s %01d.%02d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "%01d.%02d", ivalue, dec);
  }
  else {
    int ivalue = -(int)value;
    int dec = dround((-value - ivalue) * 100);
    // deal with floating point errors.
    if (dec > 99) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s-%01d.%02d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "-%01d.%02d", ivalue, dec);
  }
}

size_t els_sprint_double2(char *text, size_t size, float value, const char *prefix) {
  if (value >= 0) {
    int ivalue = (int)value;
    int dec = dround((value - ivalue) * 100);
    // deal with floating point errors.
    if (dec > 99) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s %02d.%02d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "%02d.%02d", ivalue, dec);
  }
  else {
    int ivalue = -(int)value;
    int dec = dround((-value - ivalue) * 100);
    // deal with floating point errors.
    if (dec > 99) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s-%02d.%02d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "-%02d.%02d", ivalue, dec);
  }
}

size_t els_sprint_double3(char *text, size_t size, float value, const char *prefix) {
  if (value >= 0) {
    int ivalue = (int)value;
    int dec = dround((value - ivalue) * 100);
    // deal with floating point errors.
    if (dec > 99) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s %03d.%02d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "%03d.%02d", ivalue, dec);
  }
  else {
    int ivalue = -(int)value;
    int dec = dround((-value - ivalue) * 100);
    // deal with floating point errors.
    if (dec > 99) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s-%03d.%02d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "-%03d.%02d", ivalue, dec);
  }
}

size_t els_sprint_double33(char *text, size_t size, float value, const char *prefix) {
  if (value >= 0) {
    int ivalue = (int)value;
    int dec = dround((value - ivalue) * 1000);
    // deal with floating point errors.
    if (dec > 999) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s %03d.%03d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "%03d.%03d", ivalue, dec);
  }
  else {
    int ivalue = -(int)value;
    int dec = dround((-value - ivalue) * 1000);
    // deal with floating point errors.
    if (dec > 999) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s-%03d.%03d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "-%03d.%03d", ivalue, dec);
  }
}

size_t els_sprint_double23(char *text, size_t size, float value, const char *prefix) {
  if (value >= 0) {
    int ivalue = (int)value;
    int dec = dround((value - ivalue) * 1000);
    // deal with floating point errors.
    if (dec > 999) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s %02d.%03d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "%02d.%03d", ivalue, dec);
  }
  else {
    int ivalue = -(int)value;
    int dec = dround((-value - ivalue) * 1000);
    // deal with floating point errors.
    if (dec > 999) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s-%02d.%03d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "-%02d.%03d", ivalue, dec);
  }
}

size_t els_sprint_double13(char *text, size_t size, float value, const char *prefix) {
  if (value >= 0) {
    int ivalue = (int)value;
    int dec = dround((value - ivalue) * 1000);
    // deal with floating point errors.
    if (dec > 999) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s %01d.%03d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "%01d.%03d", ivalue, dec);
  }
  else {
    int ivalue = -(int)value;
    int dec = dround((-value - ivalue) * 1000);
    // deal with floating point errors.
    if (dec > 999) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s-%01d.%03d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "-%01d.%03d", ivalue, dec);
  }
}

size_t els_sprint_double24(char *text, size_t size, float value, const char *prefix) {
  if (value >= 0) {
    int ivalue = (int)value;
    int dec = dround((value - ivalue) * 10000);
    // deal with floating point errors.
    if (dec > 9999) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s %02d.%04d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "%02d.%04d", ivalue, dec);
  }
  else {
    int ivalue = -(int)value;
    int dec = dround((-value - ivalue) * 10000);
    // deal with floating point errors.
    if (dec > 9999) { dec = 0; ivalue++; }
    if (prefix)
      return snprintf(text, size, "%s-%02d.%04d", prefix, ivalue, dec);
    else
      return snprintf(text, size, "-%02d.%04d", ivalue, dec);
  }
}

