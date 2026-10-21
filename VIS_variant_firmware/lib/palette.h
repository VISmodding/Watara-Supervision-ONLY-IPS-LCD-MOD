#ifndef PALETTE_H
#define PALETTE_H

#include <stdint.h>

void palette_init(void);

void palette_apply(uint8_t index);

void palette_apply_splash(void);

void palette_apply_no_save(uint8_t index);

uint8_t palette_count(void);

const char* palette_name(uint8_t index);

uint8_t palette_current(void);

#endif // PALETTE_H