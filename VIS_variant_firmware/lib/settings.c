#include "settings.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include <string.h>

// For 2MB flash memory
#define FLASH_TARGET_OFFSET (2 * 1024 * 1024 - 4096)

// For 16MB flash memory
//#define FLASH_TARGET_OFFSET (16 * 1024 * 1024 - 4096)

typedef struct {
    uint8_t palette;
    uint8_t brightness;
    uint16_t magic;
} settings_t;

static settings_t settings;

void settings_init() {
    const settings_t *flash = (const settings_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
    if(flash->magic == 0x4242) {
        settings = *flash;
    } else {
        settings.palette = 0;
        settings.brightness = 6; //default level
        settings.magic = 0x4242;
    }
}

uint8_t settings_get_palette() { return settings.palette; }
uint8_t settings_get_brightness() { return settings.brightness; }
void settings_set_palette(uint8_t p) { settings.palette = p; }
void settings_set_brightness(uint8_t b) { settings.brightness = b; }

void settings_save_now(void) {
    static uint8_t buffer[FLASH_PAGE_SIZE];      // 256 bytes, kept off the small Core 1 stack
    memset(buffer, 0xFF, sizeof buffer);
    memcpy(buffer, &settings, sizeof settings);

    multicore_lockout_start_blocking();          // pause Core 0 while flash is unavailable
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
    multicore_lockout_end_blocking();            // Core 0 resumes
}

// Writes the flash only if something actually changed (saves flash wear).
// Returns true if a write was done.
bool settings_save_if_changed(void) {
    const settings_t *flash = (const settings_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
    if (flash->magic == settings.magic &&
        flash->palette == settings.palette &&
        flash->brightness == settings.brightness) {
        return false;
    }
    settings_save_now();
    return true;
}