#ifndef __ADC_H__
#define __ADC_H__

#include <em_device.h>
#include "cmu.h"
#include "emu.h"
#include "systick.h"

void adc_init();

float adc_get_avdd();
float adc_get_dvdd();
float adc_get_iovdd();
float adc_get_corevdd();
float adc_get_r5v_vregi();
float adc_get_r5v_vregi_current();
float adc_get_r5v_vbus();
float adc_get_r5v_vbus_current();
float adc_get_r5v_vrego();

float adc_get_temperature();

#endif  // __ADC_H__
