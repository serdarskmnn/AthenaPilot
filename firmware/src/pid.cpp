#include "pid.h"
#include <cmath>
#include <limits>

PID::PID(double kp, double ki, double kd, double outputMin, double outputMax)
    : kp_(kp), ki_(ki), kd_(kd), integrator_(0.0), prevError_(0.0), outMin_(outputMin), outMax_(outputMax) {}

void PID::setGains(double kp, double ki, double kd) {
    kp_ = kp; ki_ = ki; kd_ = kd;
}

void PID::setOutputLimits(double minVal, double maxVal) {
    outMin_ = minVal; outMax_ = maxVal;
}

void PID::reset() {
    integrator_ = 0.0;
    prevError_ = 0.0;
}

double PID::compute(double setpoint, double measured, double dt) {
    if (dt <= 0.0) return 0.0; // prevent divide by zero
    double error = setpoint - measured;
    // Integrate with simple trapezoid rule (approx)
    integrator_ += 0.5 * (error + prevError_) * dt;
    // Derivative
    double derivative = (error - prevError_) / dt;
    double output = kp_ * error + ki_ * integrator_ + kd_ * derivative;
    // Clamp
    if (output > outMax_) output = outMax_;
    if (output < outMin_) output = outMin_;
    prevError_ = error;
    return output;
}