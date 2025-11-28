#include "hil_interface.h"
#include <iostream>

HilInterface::HilInterface() {
    system_id = 1;     // Bizim Drone ID'miz
    component_id = 1;  // Otopilot Bileşen ID'si
}

bool HilInterface::decode_sensor_packet(const mavlink_message_t* msg, SensorData& out_data) {
    if (msg->msgid != MAVLINK_MSG_ID_HIL_SENSOR) {
        return false;
    }

    // MAVLink'in kendi decode fonksiyonunu kullanıyoruz
    mavlink_hil_sensor_t packet;
    mavlink_msg_hil_sensor_decode(msg, &packet);

    // Verileri bizim temiz yapımıza aktaralım
    out_data.timestamp_us = packet.time_usec;
    
    out_data.accel[0] = packet.xacc;
    out_data.accel[1] = packet.yacc;
    out_data.accel[2] = packet.zacc;

    out_data.gyro[0] = packet.xgyro;
    out_data.gyro[1] = packet.ygyro;
    out_data.gyro[2] = packet.zgyro;

    out_data.mag[0] = packet.xmag;
    out_data.mag[1] = packet.ymag;
    out_data.mag[2] = packet.zmag;

    out_data.pressure = packet.abs_pressure;
    out_data.baro_alt = packet.pressure_alt;

    return true;
}

void HilInterface::encode_actuator_packet(const ActuatorCmd& cmd, mavlink_message_t* out_msg) {
    mavlink_hil_actuator_controls_t packet = {};

    packet.time_usec = cmd.timestamp_us;
    packet.mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
    packet.flags = 0; // Şimdilik özel flag yok

    // 4 Motorun değerini ata
    // MAVLink controls dizisi 16 elemanlıdır, biz ilk 4'ünü kullanıyoruz.
    packet.controls[0] = cmd.motors[0];
    packet.controls[1] = cmd.motors[1];
    packet.controls[2] = cmd.motors[2];
    packet.controls[3] = cmd.motors[3];

    // Mesajı paketle
    mavlink_msg_hil_actuator_controls_encode(system_id, component_id, out_msg, &packet);
}