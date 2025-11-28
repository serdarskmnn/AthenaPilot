#ifndef HIL_INTERFACE_H
#define HIL_INTERFACE_H

#include <cstdint>
#include "common/mavlink.h" // MAVLink kütüphanesini tanıması lazım

// --- BİZİM DİLİMİZ (Temiz Veri Yapıları) ---

// Sensörlerden gelen ham veriler (Simulink -> Firmware)
struct SensorData {
    uint64_t timestamp_us; // Zaman damgası
    float gyro[3];         // rad/s (x, y, z)
    float accel[3];        // m/s^2 (x, y, z)
    float mag[3];          // Gauss (x, y, z)
    float baro_alt;        // Metre
    float pressure;        // hPa
};

// Motorlara gidecek komutlar (Firmware -> Simulink)
struct ActuatorCmd {
    uint64_t timestamp_us;
    float motors[4];       // 0.0 ile 1.0 arası normal değer (Motor 1-4)
};

// --- SINIF TANIMI ---
class HilInterface {
public:
    HilInterface();

    // Gelen HIL_SENSOR mesajını çözer ve SensorData yapısına doldurur
    bool decode_sensor_packet(const mavlink_message_t* msg, SensorData& out_data);

    // Motor komutlarını alır ve HIL_ACTUATOR_CONTROLS mesajı oluşturur
    void encode_actuator_packet(const ActuatorCmd& cmd, mavlink_message_t* out_msg);

private:
    // Sistem ID'leri (MAVLink için)
    uint8_t system_id;
    uint8_t component_id;
};

#endif // HIL_INTERFACE_H