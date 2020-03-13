#ifndef __OVEN_H__
#define __OVEN_H__

#include <em_device.h>
#include "debug_macros.h"
#include "systick.h"
#include "mcp9600.h"
#include "pid.h"
#include "nvic.h"
#include "pac_lookup.h"
#include "profile.h"

#define ZEROCROSS_DELAY     10    // us
#define SSR_LATCH_OFFSET    10       // us
#define ZEROCROSS_DEADTIME  300     // us

#define PHASE_ANGLE_WIDTH   10000   // us
#define MAX_PHASE_ANGLE     (PHASE_ANGLE_WIDTH - ZEROCROSS_DEADTIME)
#define MIN_PHASE_ANGLE     (2 * SSR_LATCH_OFFSET)

#define PID_OPERATING_RANGE 15 // PID starts at (setpoint -+ this value)
#define PID_KP  350      // PID Proportional gain
#define PID_KI  20       // PID Integration gain
#define PID_KI_CAP  300  // PID Integration gain
#define PID_KD  10        // PID Derivative gain

typedef enum {
    IDLE,
    ABORT,
    PREHEAT_RAMP,
    PREHEAT,
    SOAK_RAMP,
    SOAK,
    REFLOW_RAMP,
    REFLOW,
    COOLDOWN
} ovenMode_t;

typedef enum {
    ERRONEOUS_STATE,
    USER_ABORT,
    TEMP_PROB_FAILURE,
    TARGET_UNREACHEAD,
    RESET
} ovenErr_t;

static const char *ovenERR_str[] = {
    "erroneous state", "user abort", "temperature probe failure", "target unreached", "reset"
};

void oven_init();
void oven_task();
ovenMode_t oven_get_mode();
float oven_get_temperature();
float oven_get_target_temperature();
void oven_start();
void oven_abort(ovenErr_t cause);
ovenErr_t oven_get_err();
void oven_clr_err();

#endif  // __OVEN_H__