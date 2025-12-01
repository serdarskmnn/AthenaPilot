#include "pid.h"

PID::PID(double kp, double ki, double kd) : kp_(kp), ki_(ki), kd_(kd) {}

void PID::set_gains(double kp, double ki, double kd){
    kp_ = kp; ki_ = ki; kd_ = kd;
}

double PID::update(double setpoint, double measurement, double dt){
    double error = setpoint - measurement;
    integrator_ += error * dt;
    double derivative = 0.0;
    if (dt > 1e-9) derivative = (error - last_error_) / dt;
    last_error_ = error;
    return kp_*error + ki_*integrator_ + kd_*derivative;
}

void PID::reset(){
    integrator_ = 0.0; last_error_ = 0.0;
}
