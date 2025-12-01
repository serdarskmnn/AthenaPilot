#include <iostream>
#include <string>
#include <cmath>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>
#include <atomic>

#include "udp_socket.h"
#include "pid.h"
#include "mixer.h"

enum class FlightMode { DISARMED, STABILIZE, LOITER, LAND };

struct RCState {
    int roll = 1500;
    int pitch = 1500;
    int yaw = 1500;
    int throttle = 1000;
    bool arm = false;
    std::string mode = "STABILIZE";
};

// Parse RC command message from Python. Expected messages:
// RC,<roll>,<pitch>,<yaw>,<throttle> (integers 1000-2000)
// ARM,1 or ARM,0
// MODE,<STABILIZE|LOITER|LAND>
void parse_rc_command(const std::string &msg, RCState &rc){
    if (msg.rfind("RC,", 0) == 0){
        int r,p,y,t;
        if (sscanf(msg.c_str()+3, "%d,%d,%d,%d", &r,&p,&y,&t) == 4){
            rc.roll = r; rc.pitch = p; rc.yaw = y; rc.throttle = t;
        }
    } else if (msg.rfind("ARM,",0) == 0){
        int a = atoi(msg.c_str()+4);
        rc.arm = a != 0;
    } else if (msg.rfind("MODE,",0) == 0){
        rc.mode = msg.substr(5);
    }
}

int main(){
    std::cout << "AthenaPilot Firmware (C++) starting..." << std::endl;

    // Ports
    const std::string LOCALHOST = "127.0.0.1";
    const uint16_t PORT_SIMULINK_RECV = 9002; // from Simulink (sensor data) - blocking
    const uint16_t PORT_SIMULINK_SEND = 9001; // to Simulink (motor outputs)
    const uint16_t PORT_PY_RCV = 9000; // from Python - RC/Commands - non-blocking
    const uint16_t PORT_PY_SEND = 9003; // to Python - telemetry

    UDPSocket simulink_sock; // bind to simulink port for receiving sensors
    if (!simulink_sock.bind_to_port(PORT_SIMULINK_RECV)){
        std::cerr << "Failed to bind to Simulink sensor port " << PORT_SIMULINK_RECV << std::endl;
        return 1;
    }
    UDPSocket py_sock; // bind for rc commands
    if (!py_sock.bind_to_port(PORT_PY_RCV)){
        std::cerr << "Failed to bind to Python RC port " << PORT_PY_RCV << std::endl;
        return 1;
    }
    py_sock.set_blocking(false);

    // Telemetry send socket is done by UDPSocket::send_to which creates ephemeral socket if not bound
    UDPSocket send_sock;

    PID pid_roll(0.5,0.001,0.03), pid_pitch(0.5,0.001,0.03), pid_yaw(0.3,0.0005,0.01);
    Mixer mixer;

    RCState rc;
    FlightMode mode = FlightMode::DISARMED;

    constexpr int BUF_MAX = 2048;
    char buf[BUF_MAX];
    std::string from_ip;
    uint16_t from_port = 0;

    while (true){
        // Blocking: Wait for sensor data packet from Simulink
        ssize_t r = simulink_sock.recv_blocking(buf, BUF_MAX-1, from_ip, from_port);
        if (r <= 0) {
            std::cerr << "Simulink recv error or no bytes: " << r << std::endl;
            continue;
        }
        buf[r] = '\0';
        std::string msg(buf);

        // Example sensor message: SENSOR,roll,pitch,yaw,dt
        double roll=0, pitch=0, yaw=0, dt=0.02;
        if (msg.rfind("SENSOR,",0) == 0){
            if (sscanf(msg.c_str()+7, "%lf,%lf,%lf,%lf", &roll, &pitch, &yaw, &dt) != 4){
                // Bad parse, skip
                continue;
            }
        }

        // Non-blocking: Check for RC commands from Python
        ssize_t rn = py_sock.recv_nonblocking(buf, BUF_MAX-1, from_ip, from_port);
        if (rn > 0){
            buf[rn] = '\0';
            std::string rcmsg(buf);
            parse_rc_command(rcmsg, rc);
            std::cout << "Parsed RC msg: " << rcmsg << std::endl;
            // Update flight mode
            if (rc.arm) mode = FlightMode::STABILIZE; else if (!rc.arm) mode = FlightMode::DISARMED;
            if (rc.mode == "LOITER") mode = FlightMode::LOITER;
            if (rc.mode == "LAND") mode = FlightMode::LAND;
        }

        // State machine: Simple
        if (mode == FlightMode::DISARMED){
            // Send zero motors (idle)
            std::string motors = "MOTOR,1000,1000,1000,1000";
            send_sock.send_to(LOCALHOST, PORT_SIMULINK_SEND, motors);
        } else {
            // Simple control: map RC 1000..2000 to -500..500 correction inputs
            double target_roll = (rc.roll - 1500) / 2.0; // range approx -250..250
            double target_pitch = (rc.pitch - 1500) / 2.0;
            double target_yaw = (rc.yaw - 1500) / 2.0;
            double target_throttle = rc.throttle; // 1000..2000

            double roll_correction = pid_roll.update(target_roll, roll, dt);
            double pitch_correction = pid_pitch.update(target_pitch, pitch, dt);
            double yaw_correction = pid_yaw.update(target_yaw, yaw, dt);

            // clamp corrections to integer
            int r_u = (int)std::round(roll_correction);
            int p_u = (int)std::round(pitch_correction);
            int y_u = (int)std::round(yaw_correction);

            auto motor_pwms = mixer.mix(r_u, p_u, y_u, (int)target_throttle);
            char out[256];
            snprintf(out, sizeof(out), "MOTOR,%d,%d,%d,%d", motor_pwms[0], motor_pwms[1], motor_pwms[2], motor_pwms[3]);
            send_sock.send_to(LOCALHOST, PORT_SIMULINK_SEND, out);

            // Telemetry: send roll/pitch/yaw back to Python
            char tbuf[128];
            snprintf(tbuf, sizeof(tbuf), "TELEM,%.2f,%.2f,%.2f", roll, pitch, yaw);
            send_sock.send_to(LOCALHOST, PORT_PY_SEND, tbuf);
        }

        // Sleep a bit to avoid hogging CPU (Simulink lock-step is master; it paused after sending sensor data)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return 0;
}
