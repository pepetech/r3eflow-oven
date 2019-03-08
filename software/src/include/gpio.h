#ifndef __GPIO_H__
#define __GPIO_H__

#include <em_device.h>
#include "utils.h"

#define LED_ON()        PERI_REG_BIT_SET(&GPIO->P[0].DOUT) = BIT(0)
#define LED_OFF()       PERI_REG_BIT_CLEAR(&GPIO->P[0].DOUT) = BIT(0)
#define LED_TOGGLE()    GPIO->P[0].DOUTTGL = BIT(0);

void gpio_init();

#endif  // __GPIO_H__
