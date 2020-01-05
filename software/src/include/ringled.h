#ifndef __RINGLED_H__
#define __RINGLED_H__

#include <em_device.h>
#include "systick.h"
#include "trng.h"
#include "sk9822.h"

typedef enum {
    STYLE_ABORT,
    STYLE_IDLE,
    STYLE_WORKING
} ringled_style_presets_t;

void ringled_task();
void ringled_set_style(ringled_style_presets_t ringledStylePreset);

#endif // __RINGLED_H__