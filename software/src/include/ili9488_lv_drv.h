#ifndef __ILI9488_LV_DRV_H__
#define __ILI9488_LV_DRV_H__

#include "em_device.h"
#include "usart.h"
#include "ldma.h"
#include "gpio.h"
#include "ili9488.h"
#include "lvgl.h"

#define LV_DRV_USE_LDMA 0
#if LV_DRV_USE_LDMA == 1
#define LV_DRV_DMA_CHANNEL	9
#endif

//static void ili9488_lv_drv_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

void ili9488_lv_drv_init(void);

void ili9488_lv_drv_bl_init(uint32_t ulFrequency);
void ili9488_lv_drv_bl_set(float fBrightness);

#endif // __ILI9488_LV_DRV_H__