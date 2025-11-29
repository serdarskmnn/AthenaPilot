// PID Simple Controller
#ifndef ATHENAPILOT_PID_H
#define ATHENAPILOT_PID_H

#include <algorithm>

class PID {
public:
    PID(double kp = 0.0, double ki = 0.0, double kd = 0.0, double outputMin = -1e6, double outputMax = 1e6);

    void setGains(double kp, double ki, double kd);
    void setOutputLimits(double minVal, double maxVal);
    void reset();

    // compute control output given error and dt
    double compute(double setpoint, double measured, double dt);

private:
    double kp_, ki_, kd_;
    double integrator_, prevError_;
    double outMin_, outMax_;
};

#endif // ATHENAPILOT_PID_H