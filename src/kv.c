#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

#include "kv.h"

#include "eeprom.h"
#include "utils.h"

void els_kv_setup(void) {
  uint8_t sig[8];
  memset(sig, 0, sizeof(sig));
  els_eeprom_read(0, sig, 8);
  if (memcmp(sig, ELS_KV_VERSION, strlen(ELS_KV_VERSION)) != 0) {
    printf("init eeprom: erase\n");
    els_eeprom_erase();
    printf("init eeprom: set version\n");
    els_eeprom_write(0, ELS_KV_VERSION, strlen(ELS_KV_VERSION));
    printf("init eeprom: done\n");
  }
}

void els_kv_read(els_kv_t key, void *dst, size_t size) {
  els_eeprom_read(key * ELS_KV_BLOCK_SIZE, dst, size);
}

void els_kv_write(els_kv_t key, const void *src, size_t size) {
  els_eeprom_write(key * ELS_KV_BLOCK_SIZE, src, size);
}
