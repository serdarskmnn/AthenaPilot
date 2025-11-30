#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <cstring>
#include <array>

#include "pid.h"
#include "mixer.h"
#include "udp_socket.h"

enum FlightMode { DISARMED = 0, STABILIZE = 1, LOITER = 2, LAND = 3 };

struct RCState {
    int roll = 1500;
    int pitch = 1500;
    int yaw = 1500;
    int throttle = 1000;
    bool arm = false;
    FlightMode mode = DISARMED;
};

struct Telemetry {
    double roll = 0.0; // degrees
    double pitch = 0.0;
    double yaw = 0.0;
    bool armed = false;
    FlightMode mode = DISARMED;
    double battery = 12.6;
};

// Helper to parse simple CSV key:value messages
static void parse_kv_csv(const std::string &s, RCState &rc) {
    // Format: "ROLL:1500,PITCH:1500,YAW:1500,THROTTLE:1100,ARM:0,MODE:STABILIZE"
    size_t start = 0;
    while (start < s.size()) {
        auto end = s.find(',', start);
        if (end == std::string::npos) end = s.size();
        std::string token = s.substr(start, end - start);
        auto colon = token.find(':');
        if (colon != std::string::npos) {
            std::string key = token.substr(0, colon);
            std::string val = token.substr(colon + 1);
            if (key == "ROLL") rc.roll = std::stoi(val);
            else if (key == "PITCH") rc.pitch = std::stoi(val);
            else if (key == "YAW") rc.yaw = std::stoi(val);
            else if (key == "THROTTLE") rc.throttle = std::stoi(val);
            else if (key == "ARM") rc.arm = (std::stoi(val) != 0);
            else if (key == "MODE") {
                if (val == "STABILIZE") rc.mode = STABILIZE;
                else if (val == "LOITER") rc.mode = LOITER;
                else if (val == "LAND") rc.mode = LAND;
            }
        }
        start = end + 1;
    }
}

static std::string telemetry_to_csv(const Telemetry &t) {
    char buf[256];
    snprintf(buf, sizeof(buf), "ROLL:%.2f,PITCH:%.2f,YAW:%.2f,ARMED:%d,MODE:%d,BATT:%.2f",
             t.roll, t.pitch, t.yaw, t.armed ? 1 : 0, (int)t.mode, t.battery);
    return std::string(buf);
}

static std::string motors_to_csv(const std::array<int,4> &m) {
    char buf[256];
    snprintf(buf, sizeof(buf), "M1:%d,M2:%d,M3:%d,M4:%d", m[0], m[1], m[2], m[3]);
    return std::string(buf);
}

int main() {
    try {
        UDPSocket sensor_sock;
        sensor_sock.bind_port(9002 /* sensors from Simulink */);

        UDPSocket rc_sock;
        rc_sock.bind_port(9000 /* RC from GCS */);
        rc_sock.set_nonblocking(true);

        UDPSocket motor_sock; // we'll use send_to - no bind needed for sending
        UDPSocket telemetry_sock; // to Python GCS 9003

        RCState rc_state;
        Telemetry telemetry;

        PID pid_roll(1.0, 0.0, 0.05);
        PID pid_pitch(1.0, 0.0, 0.05);
        PID pid_yaw(1.0, 0.0, 0.02);

        auto last_time = std::chrono::steady_clock::now();

        std::cout << "AthenaPilot Firmware starting - waiting for sensor data on UDP:9002 (blocking)\n";

        char buffer[4096];
        sockaddr_in sender;

        while (true) {
            // 1) BLOCKING read from Simulink sensor packet (master clock)
            int len = sensor_sock.recv_from(buffer, sizeof(buffer) - 1, sender);
            if (len <= 0) {
                // socket closed or error; short sleep & retry
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            buffer[len] = '\0';
            std::string sensor_msg(buffer);

            // parse sensor (CSV like: ROLL:0.1,PITCH:-0.2,YAW:1.2)
            // simple parsing
            double sim_roll = 0.0, sim_pitch = 0.0, sim_yaw = 0.0;
            size_t start = 0;
            while (start < sensor_msg.size()) {
                auto end = sensor_msg.find(',', start);
                if (end == std::string::npos) end = sensor_msg.size();
                std::string token = sensor_msg.substr(start, end - start);
                auto colon = token.find(':');
                if (colon != std::string::npos) {
                    std::string key = token.substr(0, colon);
                    std::string val = token.substr(colon + 1);
                    if (key == "ROLL") sim_roll = std::stod(val);
                    else if (key == "PITCH") sim_pitch = std::stod(val);
                    else if (key == "YAW") sim_yaw = std::stod(val);
                }
                start = end + 1;
            }

            // 2) Non-blocking RC update (if pending)
            sockaddr_in rc_sender;
            int rlen = rc_sock.recv_from(buffer, sizeof(buffer) - 1, rc_sender);
            if (rlen > 0) {
                buffer[rlen] = '\0';
                parse_kv_csv(std::string(buffer), rc_state);
            }

            // 3) Determine dt from last sensor packet (physics master clock)
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> dt_dur = now - last_time;
            double dt = dt_dur.count();
            if (dt <= 0) dt = 0.01; // fallback
            last_time = now;

            telemetry.roll = sim_roll;
            telemetry.pitch = sim_pitch;
            telemetry.yaw = sim_yaw;
            telemetry.armed = rc_state.arm;
            telemetry.mode = rc_state.mode;

            // 4) State machine
            static FlightMode state = DISARMED;
            if (state == DISARMED && rc_state.arm) {
                state = STABILIZE;
                std::cout << "State: DISARMED -> STABILIZE (armed)\n";
            } else if (state != DISARMED && !rc_state.arm) {
                state = DISARMED;
                std::cout << "State: -> DISARMED (disarmed)\n";
            } else {
                // mode switch
                if (rc_state.mode != state) {
                    state = rc_state.mode;
                    std::cout << "Mode changed -> " << (int)state << "\n";
                }
            }

            // 5) Control
            std::array<int,4> motor_pwms{1000,1000,1000,1000};
            if (state == DISARMED) {
                // motors 0
                motor_pwms = {1000,1000,1000,1000};
                pid_roll.reset(); pid_pitch.reset(); pid_yaw.reset();
            } else {
                // Map RC to setpoints
                double roll_sp = (rc_state.roll - 1500) / 500.0; // [-1,1] -> map to desired degrees or rate
                double pitch_sp = (rc_state.pitch - 1500) / 500.0; // simple
                double yaw_sp = (rc_state.yaw - 1500) / 500.0;
                double throttle = (rc_state.throttle - 1000) / 1000.0;
                if (throttle < 0.0) throttle = 0.0; if (throttle > 1.0) throttle = 1.0;

                // Use simple PID controlling angle -> for demo we just treat setpoint as measured
                double roll_cmd = pid_roll.update(roll_sp, telemetry.roll / 45.0 /* normalized approx */, dt);
                double pitch_cmd = pid_pitch.update(pitch_sp, telemetry.pitch / 45.0, dt);
                double yaw_cmd = pid_yaw.update(yaw_sp, telemetry.yaw / 180.0, dt);

                // combine and mix
                auto motors = mix_motors(roll_cmd, pitch_cmd, yaw_cmd, throttle);
                motor_pwms = motors;
            }

            // 6) Send motor outputs to Simulink (9001)
            std::string motor_msg = motors_to_csv(motor_pwms);
            motor_sock.send_to(motor_msg, "127.0.0.1", 9001);

            // 7) Send telemetry to Python GCS (9003)
            std::string telem = telemetry_to_csv(telemetry);
            telemetry_sock.send_to(telem, "127.0.0.1", 9003);

            // 8) (Optional) Print a small status
            std::cout << "Telemetry: " << telem << " -> Motors: " << motor_msg << "\n";

            // The Simulink master will pause until it gets motor data if configured; otherwise small delay
            // We continue to next sensor packet - main loop is locked by blocking recv
        }

    } catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
