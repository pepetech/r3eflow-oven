#ifndef __FT6X36_LV_DRV_H__
#define __FT6X36_LV_DRV_H__

#include "em_device.h"
#include "i2c.h"
#include "ldma.h"
#include "gpio.h"
#include "ft6x36.h"
#include "lvgl.h"

//static bool ft6x36_lv_drv_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

void ft6x36_lv_drv_init(void);

#endif // __FT6X36_LV_DRV_H__