#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <stdbool.h>


void settings_init(void);

uint8_t settings_get_palette(void);

uint8_t settings_get_brightness(void);

void settings_set_palette(uint8_t p);

void settings_set_brightness(uint8_t b);

void settings_save_now(void);

bool settings_save_if_changed(void);

#endif