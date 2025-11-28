#include "pid.h"

Pid::Pid() : _kp(0), _ki(0), _kd(0), _imax(0), _integral(0), _prev_error(0) {
}

void Pid::init(float kp, float ki, float kd, float imax) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
    _imax = imax;
    reset();
}

void Pid::reset() {
    _integral = 0.0f;
    _prev_error = 0.0f;
}

float Pid::constrain(float val, float min_val, float max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

float Pid::update(float setpoint, float measured, float dt) {
    // 1. Hatayı Hesapla
    float error = setpoint - measured;

    // 2. Oransal (Proportional) Terim
    float p_term = _kp * error;

    // 3. İntegral (Integral) Terim
    _integral += error * dt;

    // Anti-Windup: İntegral çok şişerse sınırla
    if (_imax > 0) {
        _integral = constrain(_integral, -_imax, _imax);
    }
    float i_term = _ki * _integral;

    // 4. Türev (Derivative) Terim
    // dt çok küçükse (0'a yakınsa) bölme hatası olmasın
    float d_term = 0.0f;
    if (dt > 0.0001f) {
        d_term = _kd * (error - _prev_error) / dt;
    }

    // Gelecek hesaplama için hatayı sakla
    _prev_error = error;

    // 5. Toplam Çıktı
    return p_term + i_term + d_term;
}