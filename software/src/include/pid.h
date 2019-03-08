#ifndef __PID_H__
#define __PID_H__

#include <stdint.h>

typedef struct pid_t
{
    float dt;
    float max;
    float min;
    float Kp;
    float Kd;
    float Ki;
    float pre_error;
    float integral;
    volatile float setpoint;
    volatile float value;
    volatile float output;
} pid_t;

void pid_calc(pid_t *pid);

#endif