#ifndef __SOUND_H__
#define __SOUND_H__

#include <em_device.h>
#include <stdlib.h>
#include "cmu.h"
#include "systick.h"
#include "utils.h"
#include "nvic.h"

void sound_init();
void sound_play(uint16_t frequency, uint32_t duration);
void sound_queue_add(uint16_t frequency, uint32_t duration);

#endif  // __SOUND_H__