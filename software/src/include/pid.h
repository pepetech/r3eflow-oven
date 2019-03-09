#ifndef __PID_H__
#define __PID_H__

#include <em_device.h>

typedef struct
{
    float fDeltaTime;
    float fMax;
    float fMin;
    float fKp;
    float fKd;
    float fKi;
    float fPreviousError;
    float fIntegral;
    volatile float fSetpoint;
    volatile float fValue;
    volatile float fOutput;
} pid_t;

pid_t pid_init(float fMin, float fMax, float fKp, float fKd, float fKi);

void pid_calc(pid_t *pPID);

#endif  //__PID_H__