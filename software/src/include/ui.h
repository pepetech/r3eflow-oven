#ifndef __UI_H__
#define __UI_H__

#include <em_device.h>
#include "systick.h"
#include "lv_disp_ili9488.h"
#include "lv_indev_ft6x36.h"
#include "lvgl.h"
#include "oven.h"
#include "ringled.h"
#include "sound.h"

void ui_init();
void ui_task();

#endif  // __UI_H__