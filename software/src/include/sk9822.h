#ifndef __WS2812B_H__
#define __WS2812B_H__

#include <em_device.h>
#include <stdlib.h>
#include <string.h>
#include "cmu.h"
#include "ldma.h"
#include "utils.h"

#define SK9822_NUM_LEDS     8
#define SK9822_DMA_CHANNEL	13
#define SK9822_BUFF_LEN     (((SK9822_NUM_LEDS + 2) * 4) + (1 + ((SK9822_NUM_LEDS/2)/8)))

#define SK9822_MAX_BRIGHTNESS 0x1F
#define SK9822_BRIGHTNESS(b) ((b) * SK9822_MAX_BRIGHTNESS)

void sk9822_init();
void sk9822_set_color(uint16_t usLED, uint8_t ubBrightness, uint8_t ubRed, uint8_t ubGreen, uint8_t ubBlue, uint8_t ubUpdate);
void sk9822_update();

#endif // __WS2812B_H__