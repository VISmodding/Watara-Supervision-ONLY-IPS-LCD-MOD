#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

void menu_init(void);
void menu_update(void);
bool menu_is_visible(void);
void menu_draw_overlay(uint8_t buffer_index);

#endif