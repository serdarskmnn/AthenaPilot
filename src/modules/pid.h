#ifndef PID_H
#define PID_H

#include <cmath>

class Pid {
public:
    Pid();

    // PID Katsayılarını Ayarla
    // kp, ki, kd: Standart katsayılar
    // imax: İntegral rüzgarı (Windup) önlemek için limit
    void init(float kp, float ki, float kd, float imax);

    // Hesaplama Fonksiyonu
    // setpoint: Olması gereken değer (Target)
    // measured: Ölçülen değer (Actual)
    // dt: Geçen süre (Saniye cinsinden)
    float update(float setpoint, float measured, float dt);

    // Reset (İntegral birikimini sıfırlar - Arm ederken lazım olur)
    void reset();

private:
    float _kp, _ki, _kd;
    float _imax;          // İntegral doygunluk limiti
    float _integral;      // Biriken hata
    float _prev_error;    // Önceki hata (Türev için)
    
    // Yardımcı: Değeri belli aralıkta tutma
    float constrain(float val, float min_val, float max_val);
};

#endif // PID_H