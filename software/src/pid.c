#include "pid.h"

void pid_init(pid_t *pPid ,float fMin, float fMax, float fKp, float fKd, float fKi)
{
    pPid->fDeltaTime = 0;
    pPid->fMax = fMax;
    pPid->fMin = fMin;
    pPid->fKp = fKp;
    pPid->fKd = fKd;
    pPid->fKi = fKi;
    pPid->fPreviousError = 0;
    pPid->fIntegral = 0;
    pPid->fSetpoint = 0;
    pPid->fValue = 0;
    pPid->fOutput = fMin;
}

void pid_calc(pid_t *pPID)
{
    // Calculate error
    float fError = pPID->fSetpoint - pPID->fValue;

    // Proportional term
    float fProportional = pPID->fKp * fError;

    // Integral term
    pPID->fIntegral += fError * pPID->fDeltaTime;
    float fIntegral = pPID->fKi * pPID->fIntegral;

    // Derivative term
    float fDeriv = (fError - pPID->fPreviousError) / pPID->fDeltaTime;
    float fDerivative = pPID->fKd * fDeriv;

    // Calculate total output
    pPID->fOutput = fProportional + fIntegral + fDerivative;

    // Restrict to max/min
    if(pPID->fOutput > pPID->fMax) pPID->fOutput = pPID->fMax;
    else if(pPID->fOutput < pPID->fMin) pPID->fOutput = pPID->fMin;

    // Save error to previous error
    pPID->fPreviousError = fError;
}