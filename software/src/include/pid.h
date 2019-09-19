#ifndef __PID_H__
#define __PID_H__

#include <em_device.h>
#include <stdlib.h>
#include "utils.h"

typedef struct pid_struct_t pid_struct_t;

struct pid_struct_t
{
    float fDeltaTime;
    float fMax;
    float fMin;
    float fKiCap;
    float fKp;
    float fKi;
    float fKd;
    float fPreviousError;
    float fIntegral;
    float fRange;
    volatile float fSetpoint;
    volatile float fValue;
    volatile float fOutput;
};

pid_struct_t* pid_init(float fMax, float fMin, float fRange, float fKiCap, float fKp, float fKi, float fKd);

void pid_free(pid_struct_t *pPID);

void pid_calc(pid_struct_t *pPID);

#endif  //__PID_H__