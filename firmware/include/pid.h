#pragma once

class PID {
public:
    PID(double kp = 0.0, double ki = 0.0, double kd = 0.0);
    void set_gains(double kp, double ki, double kd);
    double update(double setpoint, double measurement, double dt);
    void reset();
private:
    double kp_, ki_, kd_;
    double integrator_ = 0.0;
    double last_error_ = 0.0;
};
