#ifndef __PID_H__
#define __PID_H__

#include <em_device.h>
#include <stdlib.h>
#include "utils.h"

typedef struct
{
    float fDeltaTime;
    float fMax;
    float fMin;
    float fKp;
    float fKi;
    float fKd;
    float fPreviousError;
    float fIntegral;
    volatile float fSetpoint;
    volatile float fValue;
    volatile float fOutput;
} pid_t;

<<<<<<< HEAD
pid_t* pid_init(float fMax, float fMin, float fKp, float fKi, float fKd);
void pid_free(pid_t *pPID);
=======
inline void pid_init(pid_t* pPid ,float fMin, float fMax, float fKp, float fKd, float fKi);
>>>>>>> 992399800a3df921bb380cef2760b62cc341ab82

void pid_calc(pid_t *pPID);

#endif  //__PID_H__