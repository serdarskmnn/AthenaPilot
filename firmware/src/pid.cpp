#include "pid.h"

PID::PID(double kp, double ki, double kd)
    : kp_(kp), ki_(ki), kd_(kd), integral_(0.0), last_error_(0.0) {}

void PID::setGains(double kp, double ki, double kd) {
    kp_ = kp; ki_ = ki; kd_ = kd;
}

double PID::update(double setpoint, double measured, double dt) {
    if (dt <= 0) return 0.0;
    double error = setpoint - measured;
    integral_ += error * dt;
    double derivative = (error - last_error_) / dt;
    last_error_ = error;
    return kp_ * error + ki_ * integral_ + kd_ * derivative;
}

void PID::reset() {
    integral_ = 0.0;
    last_error_ = 0.0;
}
