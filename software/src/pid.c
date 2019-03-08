#include "pid.h"

void pid_calc(SPID * pid)
{
    // Calculate error
    float error = pid->m_fSetpoint - pid->m_fValue;

    // Proportional term
    float Pout = pid->m_fKp * error;

    // Integral term
    pid->m_fIntegral += error * pid->m_fdt;
    float Iout = pid->m_fKi * pid->m_fIntegral;

    // Derivative term
    float derivative = (error - pid->m_fPre_error) / pid->m_fdt;
    float Dout = pid->m_fKd * derivative;

    // Calculate total output
    pid->m_fOutput = Pout + Iout + Dout;

    // Restrict to max/min
    if(pid->m_fOutput > pid->m_fMax) pid->m_fOutput = pid->m_fMax;
    else if(pid->m_fOutput < pid->m_fMin) pid->m_fOutput = pid->m_fMin;

    // Save error to previous error
    pid->m_fPre_error = error;
}