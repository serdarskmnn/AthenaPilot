#include "mixer.h"

Mixer::Mixer() {
}

float Mixer::constrain(float val, float min_val, float max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

void Mixer::mix(float throttle, float roll_cmd, float pitch_cmd, float yaw_cmd, float* motor_outputs) {
    // Güvenlik: Gaz negatif olamaz
    throttle = constrain(throttle, 0.0f, 1.0f);

    // Quad-X Mixing Matrisi
    // Not: Bu işaretler (+/-) motorların yerine ve dönüş yönüne göre değişir.
    // Standart ArduPilot/Betaflight Quad-X sırlaması varsayılmıştır.
    
    // Motor 1 (Sağ Ön - CCW)
    motor_outputs[0] = throttle - roll_cmd + pitch_cmd + yaw_cmd;

    // Motor 2 (Sol Arka - CCW)
    motor_outputs[1] = throttle + roll_cmd - pitch_cmd + yaw_cmd;

    // Motor 3 (Sol Ön - CW)
    motor_outputs[2] = throttle + roll_cmd + pitch_cmd - yaw_cmd;

    // Motor 4 (Sağ Arka - CW)
    motor_outputs[3] = throttle - roll_cmd - pitch_cmd - yaw_cmd;

    // Çıktıları 0.0 ile 1.0 arasına sıkıştır (Clipping)
    // Gerçek bir uçuş kontrolcüsünde burada "Desaturation" algoritması olur
    // ama şimdilik basit kesme yeterli.
    for (int i = 0; i < 4; i++) {
        motor_outputs[i] = constrain(motor_outputs[i], 0.0f, 1.0f);
    }
}