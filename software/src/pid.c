#include "pid.h"

pid_t pid_init(float fMax, float fMin, float fKp, float fKd, float fKi)
{
    pid_t sPID = {
        .fDeltaTime = 0,
        .fMax = fMax,
        .fMin = fMin,
        .fKp = fKp,
        .fKd = fKd,
        .fKi = fKi,
        .fPreviousError = 0,
        .fIntegral = 0,
        .fSetpoint = 0,
        .fValue = 0,
        .fOutput = fMin };

    return sPID;
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