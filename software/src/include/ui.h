#ifndef __UI_H__
#define __UI_H__

#include <em_device.h>
#include "systick.h"
#include "trng.h"
#include "sk9822.h"
#include "lv_disp_ili9488.h"
#include "lv_indev_ft6x36.h"
#include "lvgl.h"
#include "oven.h"

typedef enum {
    STYLE_ABORT,
    STYLE_IDLE,
    STYLE_WORKING
} led_style_presets_t;

void ui_init();
void ui_task();
void ui_abort_popup(uint8_t reason);
void ui_set_led_style(led_style_presets_t ledStyle);

#endif  // __UI_H__