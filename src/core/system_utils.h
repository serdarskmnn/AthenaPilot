#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include <cstdint>
#include <chrono>

// Sistem açıldığından beri geçen süreyi mikrosaniye (us) cinsinden döner
inline uint64_t get_time_usec() {
    static auto start_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count();
}

// Milisaniye (ms) cinsinden döner
inline uint32_t get_time_msec() {
    return (uint32_t)(get_time_usec() / 1000);
}

#endif // SYSTEM_UTILS_H