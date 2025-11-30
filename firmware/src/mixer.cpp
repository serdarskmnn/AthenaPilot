#include "mixer.h"
#include <algorithm>

static int throttle_to_pwm(double t) {
    int pwm = int(1000 + t * 1000.0);
    if (pwm < 1000) pwm = 1000;
    if (pwm > 2000) pwm = 2000;
    return pwm;
}

std::array<int,4> mix_motors(double roll, double pitch, double yaw, double throttle) {
    // roll, pitch, yaw in [-1, 1]; throttle in [0,1]
    // X config:
    // M1 = Throttle + Pitch + Roll - Yaw
    // M2 = Throttle + Pitch - Roll + Yaw
    // M3 = Throttle - Pitch - Roll - Yaw
    // M4 = Throttle - Pitch + Roll + Yaw
    double m1 = throttle + pitch + roll - yaw;
    double m2 = throttle + pitch - roll + yaw;
    double m3 = throttle - pitch - roll - yaw;
    double m4 = throttle - pitch + roll + yaw;

    auto clamp = [](double v){ if (v < 0.0) return 0.0; if (v > 1.0) return 1.0; return v; };
    m1 = clamp((m1 + 1.0) / 2.0);
    m2 = clamp((m2 + 1.0) / 2.0);
    m3 = clamp((m3 + 1.0) / 2.0);
    m4 = clamp((m4 + 1.0) / 2.0);

    return {throttle_to_pwm(m1), throttle_to_pwm(m2), throttle_to_pwm(m3), throttle_to_pwm(m4)};
}
