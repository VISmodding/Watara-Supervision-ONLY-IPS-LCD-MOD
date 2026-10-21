#pragma once

#include <stdint.h>
#include <stdbool.h>

#define LCD_WIDTH 320
#define LCD_HEIGHT 320

#define FRAMEBUFFER_WIDTH 160
#define FRAMEBUFFER_HEIGHT 160

#define PALETTE_COUNT 16

#define PALETTE_SIZE 4

#define LCD_PIN_D0 0 
#define LCD_PIN_WR 9 
#define LCD_PIN_RD 8 
#define LCD_PIN_DC 10
#define LCD_PIN_CS 11
#define LCD_PIN_RST 12

#define LCD_DATA_MASK 0xFFu 
#define LCD_WR_MASK (1u << LCD_PIN_WR)
#define LCD_RD_MASK (1u << LCD_PIN_RD)
#define LCD_DC_MASK (1u << LCD_PIN_DC)
#define LCD_CS_MASK (1u << LCD_PIN_CS)
#define LCD_RST_MASK (1u << LCD_PIN_RST)

extern volatile uint8_t sync;

void lcd_init(void);
void lcd_render_framebuffer(uint8_t buffer_index);
void lcd_set_palette(uint8_t index, uint16_t color);
void lcd_set_predefined_palette(uint8_t index);

uint8_t* get_framebuffer(uint8_t index);

void lcd_draw_pixel_xy(uint16_t x, uint16_t y, uint16_t color);
uint16_t RGB(uint8_t r, uint8_t g, uint8_t b);
