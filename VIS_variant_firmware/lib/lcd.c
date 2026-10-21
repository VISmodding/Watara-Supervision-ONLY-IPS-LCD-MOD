#include "pico/stdlib.h"
#include <stdio.h>
#include "lcd.h"

static uint16_t lcd_palette[256];
extern volatile uint8_t sync;
extern volatile uint8_t render_buffer_index;

/* ===== Color conversion ===== */
static inline uint16_t rgb565_pack(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((uint16_t)(b & 0xF8) << 8) |
                     ((uint16_t)(g & 0xFC) << 3) |
                     ((uint16_t)(r >> 3)));
}

uint16_t RGB(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)~rgb565_pack(r, g, b);
}

uint16_t GRAY(uint8_t v)
{
    return (uint16_t)~rgb565_pack(v, v, v);
}

/* ===== LCD low level ===== */
static inline void wr_pulse(void)
{
    gpio_put(LCD_PIN_WR, 0);
    __asm volatile("nop; nop; nop; nop;");
    gpio_put(LCD_PIN_WR, 1);
}

static inline uint8_t reverse_bits(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

static inline void bus_write(uint8_t v)
{
    // added reverse_bits(v) % gpio_put_masked(LCD_DATA_MASK, 0xFF); for debug
    gpio_put_masked(LCD_DATA_MASK, reverse_bits(v));
}

static inline void lcd_cmd(uint8_t c)
{
    gpio_put(LCD_PIN_DC, 0);
    bus_write(c);
    wr_pulse();
}

static inline void lcd_data8(uint8_t d)
{
    gpio_put(LCD_PIN_DC, 1);
    bus_write(d);
    wr_pulse();
}

static inline void lcd_data16(uint16_t d)
{
    lcd_data8(d >> 8);
    lcd_data8(d & 0xFF);
}

void lcd_write_pixel(uint16_t c)
{
    bus_write((uint8_t)(c >> 8));
    wr_pulse();
    bus_write((uint8_t)(c & 0xFF));
    wr_pulse();
}

 void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    lcd_cmd(0x2A);
    lcd_data16(x0);
    lcd_data16(x1);

    lcd_cmd(0x2B);
    lcd_data16(y0);
    lcd_data16(y1);

    lcd_cmd(0x2C);
}

void lcd_init(void)
{
    for (int g = 0; g <= 7; ++g)
    {
        gpio_init(g);
        gpio_set_dir(g, GPIO_OUT);
        gpio_put(g, 0);
    }

    gpio_init(LCD_PIN_WR);
    gpio_set_dir(LCD_PIN_WR, GPIO_OUT);
    gpio_put(LCD_PIN_WR, 1);

    gpio_init(LCD_PIN_RD);
    gpio_set_dir(LCD_PIN_RD, GPIO_OUT);
    gpio_put(LCD_PIN_RD, 1);

    gpio_init(LCD_PIN_DC);
    gpio_set_dir(LCD_PIN_DC, GPIO_OUT);
    gpio_put(LCD_PIN_DC, 0);

    gpio_init(LCD_PIN_CS);
    gpio_set_dir(LCD_PIN_CS, GPIO_OUT);
    gpio_put(LCD_PIN_CS, 0);

    gpio_init(LCD_PIN_RST);
    gpio_set_dir(LCD_PIN_RST, GPIO_OUT);
    gpio_put(LCD_PIN_RST, 1);

    // Reset sequence from EastRising ER-TFT3.92-1 8080-8bit demo.
    gpio_put(LCD_PIN_RST, 1);
    sleep_ms(5);
    gpio_put(LCD_PIN_RST, 0);
    sleep_ms(10);
    gpio_put(LCD_PIN_RST, 1);
    sleep_ms(50);

    // EastRising ST7796S initialization for ER-TFT3.92-1.
    lcd_cmd(0xF0);
    lcd_data8(0xC3);
    lcd_cmd(0xF0);
    lcd_data8(0x96);

    lcd_cmd(0x36);
    lcd_data8(0x40);

    lcd_cmd(0x3A);
    lcd_data8(0x55);

    lcd_cmd(0xB4);
    lcd_data8(0x00);

    lcd_cmd(0xB6);
    lcd_data8(0x8A);
    lcd_data8(0x07);
    lcd_data8(0x27);

    lcd_cmd(0xB7);
    lcd_data8(0xC6);

    lcd_cmd(0xB9);
    lcd_data8(0x02);
    lcd_data8(0xE0);

    lcd_cmd(0xC0);
    lcd_data8(0x80);
    lcd_data8(0x06);

    lcd_cmd(0xC1);
    lcd_data8(0x15);

    lcd_cmd(0xC2);
    lcd_data8(0xA7);

    lcd_cmd(0xC5);
    lcd_data8(0x04);

    lcd_cmd(0xE8);
    lcd_data8(0x40);
    lcd_data8(0x8A);
    lcd_data8(0x00);
    lcd_data8(0x00);
    lcd_data8(0x29);
    lcd_data8(0x19);
    lcd_data8(0xAA);
    lcd_data8(0x33);

    lcd_cmd(0xE0);
    lcd_data8(0xF0);
    lcd_data8(0x06);
    lcd_data8(0x0F);
    lcd_data8(0x05);
    lcd_data8(0x04);
    lcd_data8(0x20);
    lcd_data8(0x37);
    lcd_data8(0x33);
    lcd_data8(0x4C);
    lcd_data8(0x37);
    lcd_data8(0x13);
    lcd_data8(0x14);
    lcd_data8(0x2B);
    lcd_data8(0x31);

    lcd_cmd(0xE1);
    lcd_data8(0xF0);
    lcd_data8(0x11);
    lcd_data8(0x1B);
    lcd_data8(0x11);
    lcd_data8(0x0F);
    lcd_data8(0x0A);
    lcd_data8(0x37);
    lcd_data8(0x43);
    lcd_data8(0x4C);
    lcd_data8(0x37);
    lcd_data8(0x13);
    lcd_data8(0x13);
    lcd_data8(0x2C);
    lcd_data8(0x32);

    lcd_cmd(0xF0);
    lcd_data8(0x3C);
    lcd_cmd(0xF0);
    lcd_data8(0x69);

    lcd_cmd(0x35);
    lcd_data8(0x00);

    lcd_cmd(0x11);
    sleep_ms(120);

    lcd_cmd(0x29);
    sleep_ms(20);

    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

        
void lcd_set_palette(uint8_t index, uint16_t color)
{
    lcd_palette[index] = color;
}

// Render framebuffer
void __inline __not_in_flash("lcd_render_framebuffer")
lcd_render_framebuffer(uint8_t buffer_index)
{
    gpio_put(LCD_PIN_DC, 1);

    const uint8_t* buffer = get_framebuffer(buffer_index);
    uint32_t count = (uint32_t)FRAMEBUFFER_WIDTH * (uint32_t)FRAMEBUFFER_HEIGHT;
    uint8_t line = 0;

    while (count--)
    {
        uint16_t color = lcd_palette[buffer[count]];
        lcd_write_pixel(color);
        lcd_write_pixel(color);

        if (count % FRAMEBUFFER_WIDTH == 0)
        {
            if (line++ == 0)
            {
                count += FRAMEBUFFER_WIDTH;
            }
            else
            {
                line = 0;
            }
        }
    }

    sync = 0;
}

void lcd_draw_pixel_xy(uint16_t x, uint16_t y, uint16_t color)
{
    lcd_set_window(x, y, x, y);
    gpio_put(LCD_PIN_DC, 1);
    lcd_write_pixel(color);

    // restore full window
    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}
