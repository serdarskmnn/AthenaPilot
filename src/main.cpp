#include <iostream>
#include <thread>
#include <vector>
#include <cmath>

// Kendi Modüllerimiz
#include "drivers/udp_driver.h"
#include "core/system_utils.h"
#include "common/mavlink.h"
#include "modules/hil_interface.h"
#include "modules/mixer.h"
#include "modules/pid.h"
#include "modules/estimator.h"

// --- AYARLAR ---
#define SIM_IP          "127.0.0.1"
#define REMOTE_PORT     14550 
#define LOCAL_PORT      14551

// --- GLOBAL NESNELER ---
UdpDriver udp;
HilInterface hil;
Mixer mixer;
Estimator estimator;

// --- PID NESNELERİ (KADEMELİ YAPI) ---

// 1. İç Halka (Rate Loop) - Hızlı tepki verir (Dönme hızını kontrol eder)
Pid pid_roll_rate;
Pid pid_pitch_rate;
Pid pid_yaw_rate;

// 2. Dış Halka (Angle Loop) - Patron (Açıyı kontrol eder)
Pid pid_roll_angle;
Pid pid_pitch_angle;
// Yaw Angle genelde pusula gerektirir, şimdilik Yaw Rate'de kalacağız.

// MAVLink Değişkenleri
mavlink_status_t status;
mavlink_message_t msg;

// Zamanlama Değişkeni
uint64_t last_time_us = 0;

// --- FONKSİYON PROTOTİPLERİ ---
void handle_message(mavlink_message_t* msg);
void send_heartbeat();

int main() {
    std::cout << "[ATHENA] Sistem Baslatiliyor..." << std::endl;

    // --- 1. PID AYARLARI (TUNING) ---
    
    // A. RATE CONTROLLERS (İç Halka)
    // "Saniyede X derece dön" emrini yerine getirir.
    // A. RATE CONTROLLERS (İç Halka) - YUMUŞATILDI
    // P: 0.15 -> 0.05 (Tepki gücü 3 kat azaldı)
    // D: 0.005 -> 0.002


 // --- TEMİZ BAŞLANGIÇ AYARI (P-Only Controller) ---

    // A. RATE CONTROLLERS (İç Halka)
    // D = 0.0 (Kesinlikle 0 olsun, gürültü istemiyoruz)
    // P = 0.10 (Orta karar bir güç)
    pid_roll_rate.init(0.10f, 0.0f, 0.000f, 5.0f);
    pid_pitch_rate.init(0.10f, 0.0f, 0.000f, 5.0f);
    
    // Yaw
    pid_yaw_rate.init(0.20f, 0.05f, 0.000f, 5.0f);

    // B. ANGLE CONTROLLERS (Dış Halka)
    // P = 2.0 (Hata varsa düzeltmeye çalışsın)
    pid_roll_angle.init(2.0f, 0.0f, 0.0f, 0.0f);
    pid_pitch_angle.init(2.0f, 0.0f, 0.0f, 0.0f);

    // --- 2. ESTIMATOR AYARLARI ---
    // 0.98 = Gyro'ya %98 güven (Hızlı), %2 İvmeölçer (Drift düzeltme)
    estimator.init(0.98f);

    // --- 3. İLETİŞİM BAŞLATMA ---
    if (!udp.init(SIM_IP, REMOTE_PORT, LOCAL_PORT)) {
        std::cerr << "[HATA] UDP Baslatilamadi!" << std::endl;
        return -1;
    }

    std::cout << "[ATHENA] Bekleme Modu (Lockstep Active)..." << std::endl;
    std::cout << "[MOD] STABILIZE (Self-Leveling)" << std::endl;
    
    last_time_us = get_time_usec();
    uint8_t rx_buffer[2048];
    uint32_t last_heartbeat = 0;

    // --- ANA DÖNGÜ ---
    while (true) {
        int bytes_read = udp.read(rx_buffer, sizeof(rx_buffer));

        if (bytes_read > 0) {
            for (int i = 0; i < bytes_read; i++) {
                if (mavlink_parse_char(MAVLINK_COMM_0, rx_buffer[i], &msg, &status)) {
                    handle_message(&msg);
                }
            }
        }

        uint32_t now = get_time_msec();
        if (now - last_heartbeat > 1000) {
            send_heartbeat();
            last_heartbeat = now;
        }

        if (bytes_read == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    return 0;
}

// --- KONTROL ALGORİTMASI ---
void handle_message(mavlink_message_t* msg) {
    switch (msg->msgid) {
        case MAVLINK_MSG_ID_HIL_SENSOR:
        {
            SensorData sensors;
            if (hil.decode_sensor_packet(msg, sensors)) {
                
                // 1. ZAMAN FARKI (DT)
                uint64_t now_us = get_time_usec();
                float dt = (now_us - last_time_us) / 1000000.0f;
                last_time_us = now_us;
                if (dt <= 0.0f || dt > 0.1f) dt = 0.004f;

                // 2. DURUM KESTİRİMİ (Açıları Bul)
                estimator.update(sensors, dt);

                // İstersen açıları görmek için burayı açabilirsin:
                // float roll_deg = estimator.get_roll() * 57.2958f;
                // std::cout << "Roll: " << roll_deg << std::endl;

                // 3. PİLOT EMİRLERİ (SETPOINTS)
                // Hedef: 0 Derece (Dümdüz Dur) -> STABILIZE MOD
                float target_roll_angle = 0.0f;
                float target_pitch_angle = 0.0f;
                float target_yaw_rate = 0.0f; // Yaw'da dönme

                // Gaz Ayarı (Simülasyon fiziğine göre oynayabilirsin)
                // Eğer drone çok hızlı yükseliyorsa bunu 0.50'ye düşür.
                float throttle = 0.58f; 

                // 4. KADEMELİ PID KONTROL (CASCADED LOOP)
                
                // A. DIŞ HALKA (Angle -> Rate)
                // Hedef Açı ile Mevcut Açı arasındaki farka göre "Hedef Hız" üretir.
                float target_roll_rate = pid_roll_angle.update(target_roll_angle, estimator.get_roll(), dt);
                float target_pitch_rate = pid_pitch_angle.update(target_pitch_angle, estimator.get_pitch(), dt);

                // B. İÇ HALKA (Rate -> Torque)
                // Hedef Hız ile Mevcut Gyro Hızı arasındaki farka göre "Motor Torku" üretir.
                float roll_torque = pid_roll_rate.update(target_roll_rate, sensors.gyro[0], dt);
                float pitch_torque = pid_pitch_rate.update(target_pitch_rate, sensors.gyro[1], dt);
                float yaw_torque = pid_yaw_rate.update(target_yaw_rate, sensors.gyro[2], dt);

                // 5. MIXER
                float motor_pwm[4];
                mixer.mix(throttle, roll_torque, pitch_torque, yaw_torque, motor_pwm);

                // 6. PAKETLEME
                ActuatorCmd motors;
                motors.timestamp_us = now_us;
                for(int i=0; i<4; i++) motors.motors[i] = motor_pwm[i];

                mavlink_message_t motor_msg;
                hil.encode_actuator_packet(motors, &motor_msg);

                uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
                uint16_t len = mavlink_msg_to_send_buffer(buffer, &motor_msg);
                udp.write(buffer, len);
            }
            break;
        }
        default: break;
    }
}

void send_heartbeat() {
    mavlink_heartbeat_t hb;
    hb.type = MAV_TYPE_QUADROTOR;
    hb.autopilot = MAV_AUTOPILOT_GENERIC;
    hb.base_mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED | MAV_MODE_FLAG_STABILIZE_ENABLED; // Mod bayrağını güncelledik
    hb.system_status = MAV_STATE_ACTIVE;
    hb.mavlink_version = 3;

    mavlink_message_t msg_out;
    mavlink_msg_heartbeat_encode(1, 1, &msg_out, &hb);

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg_out);

    udp.write(buffer, len);
}