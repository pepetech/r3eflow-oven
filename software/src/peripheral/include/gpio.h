#ifndef __GPIO_H__
#define __GPIO_H__

#include <em_device.h>
#include "cmu.h"
#include "systick.h"
#include "utils.h"
#include "nvic.h"
#include "lv_indev_ft6x36.h"

// TFT MACROS
#define ILI9488_RESET()     PERI_REG_BIT_CLEAR(&(GPIO->P[1].DOUT)) = BIT(5)
#define ILI9488_UNRESET()   PERI_REG_BIT_SET(&(GPIO->P[1].DOUT)) = BIT(5)
#define ILI9488_SELECT()    PERI_REG_BIT_CLEAR(&(GPIO->P[2].DOUT)) = BIT(5)
#define ILI9488_UNSELECT()  PERI_REG_BIT_SET(&(GPIO->P[2].DOUT)) = BIT(5)
#define ILI9488_IRQ()       PERI_REG_BIT(&(GPIO->P[1].DIN), 3)
#define ILI9488_SETUP_DAT() PERI_REG_BIT_SET(&(GPIO->P[2].DOUT)) = BIT(1)
#define ILI9488_SETUP_CMD() PERI_REG_BIT_CLEAR(&(GPIO->P[2].DOUT)) = BIT(1)

#define TFT_BL_ON()         PERI_REG_BIT_SET(&(GPIO->P[1].DOUT)) = BIT(0)
#define TFT_BL_OFF()        PERI_REG_BIT_CLEAR(&(GPIO->P[1].DOUT)) = BIT(0)

#define FT6X36_RESET()     PERI_REG_BIT_CLEAR(&(GPIO->P[1].DOUT)) = BIT(6)
#define FT6X36_UNRESET()   PERI_REG_BIT_SET(&(GPIO->P[1].DOUT)) = BIT(6)

void gpio_init();

#endif  // __GPIO_H__
