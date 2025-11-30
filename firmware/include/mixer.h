#pragma once
#include <array>

// Simple 4-motor X-mixer using inputs: roll (-1..1), pitch (-1..1), yaw (-1..1), throttle (0..1)
std::array<int,4> mix_motors(double roll, double pitch, double yaw, double throttle);
