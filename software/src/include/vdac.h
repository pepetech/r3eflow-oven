#ifndef __VDAC_H__
#define __VDAC_H__

#include <em_device.h>
#include <stdlib.h>
#include <math.h>
#include "utils.h"
#include "ldma.h"
#include "cmu.h"

#define VDAC_DMA_CH 12

void vdac_init();

void vdac_set_dc(uint16_t usOutput);
void vdac_set_ac(uint16_t *pWave, uint16_t usFrequency, uint16_t usSamplesPeriod);

void vdac_gen_sine(float fAmplitude, uint16_t usFrequency, uint32_t ulMaxSamplesSecond);

#endif  // __VDAC_H__