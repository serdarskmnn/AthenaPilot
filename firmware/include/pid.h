#pragma once

class PID {
public:
    PID(double kp = 1.0, double ki = 0.0, double kd = 0.0);
    void setGains(double kp, double ki, double kd);
    double update(double setpoint, double measured, double dt);
    void reset();
private:
    double kp_;
    double ki_;
    double kd_;
    double integral_;
    double last_error_;
};
