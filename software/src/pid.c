#include "pid.h"

void pid_calc(pid_t * pid)
{
    // Calculate error
    float error = pid->setpoint - pid->value;

    // Proportional term
    float Pout = pid->Kp * error;

    // Integral term
    pid->integral += error * pid->dt;
    float Iout = pid->Ki * pid->integral;

    // Derivative term
    float derivative = (error - pid->pre_error) / pid->dt;
    float Dout = pid->Kd * derivative;

    // Calculate total output
    pid->output = Pout + Iout + Dout;

    // Restrict to max/min
    if(pid->output > pid->max) pid->output = pid->max;
    else if(pid->output < pid->min) pid->output = pid->min;

    // Save error to previous error
    pid->pre_error = error;
}