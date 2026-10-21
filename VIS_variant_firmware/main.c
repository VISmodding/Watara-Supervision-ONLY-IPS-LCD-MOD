#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <hardware/vreg.h>
#include <hardware/clocks.h>
#include "lib/supervision.h"
#include "lib/lcd.h"
#include "lib/menu.h"
#include "lib/palette.h"
#include "lib/settings.h"
#include "lib/brightness.h"

volatile uint8_t sync = 0;
volatile uint8_t render_buffer_index = 0;
volatile bool game_active = false;

// --- CORE 1: Rendering e Menu ---
void __time_critical_func(render_core)() {
    lcd_init();
    settings_init();
    brightness_init();

    palette_apply_splash();
    inject_vis_variant_overlay(1);
    
    lcd_render_framebuffer(render_buffer_index);

    menu_init();
    bool palette_loaded = false;

    sync = 1;

    while (true) {
        
        if (game_active && !palette_loaded) {
            palette_init(); 
            palette_loaded = true;
        }

        menu_update();

        if (sync) {
            menu_draw_overlay(render_buffer_index);   // menu, brightness bar, "SAVED" popup
            lcd_render_framebuffer(render_buffer_index);
        }
    }
}

// --- CORE 0: Capture Data (Bus Supervision) ---
void __not_in_flash_func(capture_data)() {
    uint8_t last_clock_state = 0;
    uint8_t last_polarity_state = 1;
    uint8_t field = 0;
    
    uint8_t *buffer = get_framebuffer(1);
    
    supervision_gpio_init();

    // Wait a new frame from the console
    while (gpio_get(SV_PIN_FRAME_POLARITY)) {}
    while (!gpio_get(SV_PIN_FRAME_POLARITY)) {}

    while (true) {
        const uint32_t bus = gpio_get_all();
        const uint8_t clock = (bus >> SV_PIN_PIXEL_CLOCK) & 1;
        const uint8_t polarity = (bus >> SV_PIN_FRAME_POLARITY) & 1;

        if (clock && !last_clock_state) {
            
            if (!game_active) {
                game_active = true;
            }

            if (field == 0) {
                *buffer++ = (bus >> SV_PIN_DATA0) & 1;
                *buffer++ = (bus >> SV_PIN_DATA1) & 1;
                *buffer++ = (bus >> SV_PIN_DATA2) & 1;
                *buffer++ = (bus >> SV_PIN_DATA3) & 1;
            } else {
                *buffer++ += (bus >> SV_PIN_DATA0) & 1;
                *buffer++ += (bus >> SV_PIN_DATA1) & 1;
                *buffer++ += (bus >> SV_PIN_DATA2) & 1;
                *buffer++ += (bus >> SV_PIN_DATA3) & 1;
            }
        }

        if (polarity != last_polarity_state) {
            if (field == 2) {
                if (!sync) {
                    render_buffer_index = !render_buffer_index;
                    sync = 1;
                }
                field = 0;
            } else {
                field++;
            }
            buffer = get_framebuffer(!render_buffer_index);
        }

        last_clock_state = clock;
        last_polarity_state = polarity;
    }
}

int main() {
    // Overclock to 240 MHz for fast bit-banging 
    vreg_set_voltage(VREG_VOLTAGE_1_20);
    sleep_ms(10);
    set_sys_clock_khz(240000, true);
    sleep_ms(10);

    sleep_ms(50);
    multicore_launch_core1(render_core);

    sleep_ms(1000);
    multicore_lockout_victim_init();   // lets Core 1 pause Core 0 while saving settings to flash
    capture_data();
    
    return 0;
}