#include "estimator.h"

// Radyan <-> Derece dönüşümü gerekebilir ama biz tüm sistemi RADYAN tutacağız.
// Çünkü sin(), cos() radyan ister.

Estimator::Estimator() : _roll(0), _pitch(0), _yaw(0), _alpha(0.98f) {
}

void Estimator::init(float alpha) {
    _alpha = alpha;
}

void Estimator::update(const SensorData& data, float dt) {
    // 1. İVMEÖLÇERDEN AÇI HESABI (Trigonometri)
    // Yerçekimi vektörüne göre Roll ve Pitch bulma
    // Accel birimi m/s^2 veya g olabilir, atan2 kullandığımız için oran önemlidir, birim değil.
    
    float acc_roll = atan2(data.accel[1], data.accel[2]); 
    float acc_pitch = atan2(-data.accel[0], sqrt(data.accel[1]*data.accel[1] + data.accel[2]*data.accel[2]));

    // 2. GYRO ENTEGRASYONU (Tahmin)
    // Yeni Açı = Eski Açı + (Hız * Zaman)
    float gyro_roll = _roll + data.gyro[0] * dt;
    float gyro_pitch = _pitch + data.gyro[1] * dt;
    float gyro_yaw = _yaw + data.gyro[2] * dt;

    // 3. SENSOR FÜZYONU (Complementary Filter)
    // Gyro'ya çok güven (%98), İvmeölçerle düzelt (%2)
    _roll = (_alpha * gyro_roll) + ((1.0f - _alpha) * acc_roll);
    _pitch = (_alpha * gyro_pitch) + ((1.0f - _alpha) * acc_pitch);
    
    // Yaw için pusula (Mag) füzyonu zordur, şimdilik sadece Gyro
    _yaw = gyro_yaw; 
}