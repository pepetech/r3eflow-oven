#ifndef __PID_H__
#define __PID_H__

#include <stdint.h>

typedef struct pid
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
} PID;

void pid_calc(PID *pid);

#endif // __PID_H__