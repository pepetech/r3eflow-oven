#ifndef __UI_H__
#define __UI_H__

#include <em_device.h>
#include "systick.h"
#include "trng.h"
#include "sk9822.h"
#include "lv_disp_ili9488.h"
#include "lv_indev_ft6x36.h"
#include "lvgl.h"

void ui_init();
void ui_task();

#endif  // __UI_H__