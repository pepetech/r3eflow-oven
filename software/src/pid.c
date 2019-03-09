#include "pid.h"

pid_t* pid_init(float fMax, float fMin, float fKp, float fKi, float fKd)
{
    pid_t *pPID = (pid_t *)malloc(sizeof(pid_t));

    if(!pPID)
        return NULL;

    pPID->fDeltaTime = 0.f;
    pPID->fMax = fMax;
    pPID->fMin = fMin;
    pPID->fKp = fKp;
    pPID->fKi = fKi;
    pPID->fKd = fKd;
    pPID->fPreviousError = 0.f;
    pPID->fIntegral = 0.f;
    pPID->fSetpoint = 0.f;
    pPID->fValue = 0.f;
    pPID->fOutput = fMin;

    return pPID;
}
void pid_free(pid_t *pPID)
{
    if(!pPID)
        return;

    free(pPID);
}

void pid_calc(pid_t *pPID)
{
    if(!pPID)
        return;

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

    // Clamp
    pPID->fOutput = CLAMP(pPID->fOutput, pPID->fMin, pPID->fMax);

    // Save error to differentiate
    pPID->fPreviousError = fError;
}