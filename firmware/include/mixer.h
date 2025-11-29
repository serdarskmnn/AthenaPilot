#ifndef ATHENAPILOT_MIXER_H
#define ATHENAPILOT_MIXER_H

#include <array>

class Mixer {
public:
    Mixer(int pwmMin = 1000, int pwmMax = 2000);

    // Returns array of four PWM values (m1..m4)
    std::array<int,4> mix(float roll, float pitch, float yaw, float thrust);

    void setPWMRange(int minVal, int maxVal);
private:
    int minPWM_, maxPWM_;
    int clampPWM(int val);
};

#endif // ATHENAPILOT_MIXER_H