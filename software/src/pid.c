#include "pid.h"

void calc_pid(PID * pid)
{
        // Calculate error
    double error = (*pid)._setpoint - (*pid)._value;

    // Proportional term
    double Pout = (*pid)._Kp * error;

    // Integral term
    (*pid)._integral += error * (*pid)._dt;
    double Iout = (*pid)._Ki * (*pid)._integral;

    // Derivative term
    double derivative = (error - (*pid)._pre_error) / (*pid)._dt;
    double Dout = (*pid)._Kd * derivative;

    // Calculate total output
    (*pid)._output = Pout + Iout + Dout;

    // Restrict to max/min
    if( (*pid)._output > (*pid)._max )
        (*pid)._output = (*pid)._max;
    else if( (*pid)._output < (*pid)._min )
        (*pid)._output = (*pid)._min;

    // Save error to previous error
    (*pid)._pre_error = error;
}