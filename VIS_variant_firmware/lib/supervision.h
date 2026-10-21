#include <stdint.h>

#define SV_WIDTH 160
#define SV_HEIGHT 160

#define SV_PIN_DATA0 22
#define SV_PIN_DATA1 21
#define SV_PIN_DATA2 20
#define SV_PIN_DATA3 19
#define SV_PIN_PIXEL_CLOCK 18
#define SV_PIN_FRAME_POLARITY 17

void supervision_gpio_init(void);