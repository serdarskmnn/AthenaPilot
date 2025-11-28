#ifndef MIXER_H
#define MIXER_H

#include <algorithm> // clamp için
#include <cmath>

class Mixer {
public:
    Mixer();

    // Girdi: Kontrolcüden gelen Roll, Pitch, Yaw tork istekleri (-1.0 ile +1.0 arası)
    //        ve Gaz (Throttle) isteği (0.0 ile 1.0 arası)
    // Çıktı: motor_outputs dizisine 0.0 - 1.0 arası motor hızlarını yazar.
    void mix(float throttle, float roll_cmd, float pitch_cmd, float yaw_cmd, float* motor_outputs);

private:
    // Motor doyuma ulaşırsa (Saturation) dengesizliği önlemek için kesme yapar
    float constrain(float val, float min_val, float max_val);
};

#endif // MIXER_H