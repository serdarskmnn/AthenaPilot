
#include "mixer.h"
#include <cmath>
#include <algorithm>

Mixer::Mixer(int pwmMin, int pwmMax)
    : minPWM_(pwmMin), maxPWM_(pwmMax) {}

void Mixer::setPWMRange(int minVal, int maxVal) {
    minPWM_ = minVal; maxPWM_ = maxVal;
}

int Mixer::clampPWM(int val) {
    if (val < minPWM_) return minPWM_;
    if (val > maxPWM_) return maxPWM_;
    return val;
}

// Simple X-quad mixing: m1 front-left, m2 front-right, m3 rear-right, m4 rear-left
std::array<int,4> Mixer::mix(float roll, float pitch, float yaw, float thrust) {
    // Inputs assume roll/pitch/yaw are control signals in the range [-1..1], thrust in [0..1]
    float t = std::clamp(thrust, 0.0f, 1.0f);
    // Example mixing: each motor receives thrust +/- contributions
    float m1 = t + pitch + roll - yaw; // front-left
    float m2 = t + pitch - roll + yaw; // front-right
    float m3 = t - pitch - roll - yaw; // rear-right
    float m4 = t - pitch + roll + yaw; // rear-left

    // Normalize from possible range [-3..3] to [minPWM, maxPWM]
    const float maxRange = 3.0f; // maximum absolute value roughly
    auto toPWM = [&](float v)->int {
        // map v from [-maxRange..maxRange] to [minPWM..maxPWM]
        float normalized = (v + maxRange) / (2.0f * maxRange); // [0..1]
        int pwm = static_cast<int>(minPWM_ + normalized * (maxPWM_ - minPWM_));
        return clampPWM(pwm);
    };

    return { toPWM(m1), toPWM(m2), toPWM(m3), toPWM(m4) };
}