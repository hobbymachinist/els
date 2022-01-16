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

void els_eeprom_setup(void);
bool els_eeprom_erase(void);
bool els_eeprom_read(size_t offset, void *dst, size_t size);
bool els_eeprom_write(size_t offset, const void *src, size_t size);

#ifdef __cplusplus
}
#endif

