#include "brightness.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include "settings.h"

#define PWM_PIN 16
#define LEVELS BRIGHTNESS_LEVELS

// default level
static uint8_t level = 6;

// Higher duty value = dimmer backlight (PWM wrap = 16000).
// The three brightest levels of v1.2 (2500, 1500, 500) were removed: they
// drew too much current and made the battery voltage sag (low battery
// indicator). Three dimmer levels were added at the bottom instead.
static const uint16_t duty_table[LEVELS] = {
            12000,11000,10250,9500,8500,7000,6000,5000,4000
};

void brightness_apply() {
    uint slice = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_set_chan_level(slice, PWM_CHAN_A, duty_table[level]);
}

void brightness_init() {
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PWM_PIN);

    pwm_config cfg = pwm_get_default_config();
    
    pwm_config_set_wrap(&cfg, 16000);
    pwm_init(slice, &cfg, true);

    level = settings_get_brightness();
    
    if (level >= LEVELS) level = LEVELS - 1;
    
    brightness_apply();
}

void brightness_up() {
    if (level < LEVELS - 1) {
        level++;
        brightness_apply();
        settings_set_brightness(level); 
    }
}

void brightness_down() {
    
    if (level > 0) { 
        level--;
        brightness_apply();
        settings_set_brightness(level); 
    }
}

uint8_t brightness_get_level() {
    return level;
}