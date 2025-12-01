#include "mixer.h"
#include <algorithm>

Mixer::Mixer() {}

// Quadcopter X mix: motors M1(Front Left), M2(Front Right), M3(Rear Right), M4(Rear Left)
// Basic mixing: throttle + roll - pitch +/- yaw
std::array<int, 4> Mixer::mix(int roll_u, int pitch_u, int yaw_u, int throttle){
    // Scale corrections; assume roll_u/pitch_u/yaw_u are in reasonable range
    int m1 = throttle + roll_u - pitch_u + yaw_u; // front-left
    int m2 = throttle - roll_u - pitch_u - yaw_u; // front-right
    int m3 = throttle - roll_u + pitch_u + yaw_u; // rear-right
    int m4 = throttle + roll_u + pitch_u - yaw_u; // rear-left

    auto clamp = [](int v){ return std::min(2000, std::max(1000, v)); };
    return { clamp(m1), clamp(m2), clamp(m3), clamp(m4) };
}
