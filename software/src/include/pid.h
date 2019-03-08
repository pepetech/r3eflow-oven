#ifndef __PID_H__
#define __PID_H__

#include <stdint.h>

typedef struct SPid
{
    float m_fdt;
    float m_fMax;
    float m_fMin;
    float m_fKp;
    float m_fKd;
    float m_fKi;
    float m_fPre_error;
    float m_fIntegral;
    volatile float m_fSetpoint;
    volatile float m_fValue;
    volatile float m_fOutput;
} SPID;

void pid_calc(SPID *pid);

#endif // __PID_H__