#ifndef ESTIMATOR_H
#define ESTIMATOR_H

#include <cmath>
#include "hil_interface.h" // SensorData yapısını kullanacağız

class Estimator {
public:
    Estimator();

    // Filtreyi başlatır
    void init(float alpha);

    // Sensör verilerini alır, açıyı günceller
    void update(const SensorData& data, float dt);

    // Hesaplanan açıları okumak için
    float get_roll() { return _roll; }
    float get_pitch() { return _pitch; }
    float get_yaw() { return _yaw; } // Şimdilik sadece Gyro entegrasyonu

private:
    float _roll;    // Radyan cinsinden
    float _pitch;   // Radyan cinsinden
    float _yaw;     // Radyan cinsinden
    
    float _alpha;   // Filtre katsayısı (Örn: 0.98)
};

#endif // ESTIMATOR_H