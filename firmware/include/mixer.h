#pragma once
#include <array>

class Mixer {
public:
    Mixer();
    // Input roll/pitch/yaw correction [-500..500], throttle [1000..2000]
    std::array<int, 4> mix(int roll_u, int pitch_u, int yaw_u, int throttle);
};
