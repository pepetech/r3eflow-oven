
#include <stdint.h>

typedef struct pid
{
    double _dt;
    double _max;
    double _min;
    double _Kp;
    double _Kd;
    double _Ki;
    double _pre_error;
    double _integral;
    double _setpoint;
    double _value;
    double _output;
} PID;

void calc_pid(PID * pid);