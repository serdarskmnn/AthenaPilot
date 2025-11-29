#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <thread>
#include <algorithm>

#include "udp_socket.h"
#include "pid.h"
#include "mixer.h"

// Ports and addresses used in the lock-step system
static constexpr uint16_t SENSOR_PORT = 9001;       // Simulink -> Firmware (blocking receive)
static constexpr uint16_t MOTOR_COMMAND_PORT = 9002; // Firmware -> Simulink (motor commands)
static constexpr uint16_t GC_TELEM_PORT = 9003;     // Firmware -> Ground Control (telemetry)
static constexpr const char* LOCALHOST = "127.0.0.1";

struct SensorPacket {
    float roll, pitch, yaw, thrust;
    float altitude, groundspeed, battery;
    double timestamp;
};

// Deserialize a simple sensor CSV string: roll,pitch,yaw,thrust,alt,gs,batt,timestamp
bool parseSensorCSV(const std::string& s, SensorPacket& out) {
    std::istringstream iss(s);
    std::string token;
    std::vector<std::string> toks;
    while (std::getline(iss, token, ',')) toks.push_back(token);
    if (toks.size() < 8) return false;
    try {
        out.roll = std::stof(toks[0]); out.pitch = std::stof(toks[1]); out.yaw = std::stof(toks[2]);
        out.thrust = std::stof(toks[3]); out.altitude = std::stof(toks[4]);
        out.groundspeed = std::stof(toks[5]); out.battery = std::stof(toks[6]);
        out.timestamp = std::stod(toks[7]);
    } catch (...) {
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    std::cout << "Starting AthenaPilot Firmware (C++)" << std::endl;
    try {
        UDPSocket udp;
        udp.bindReceive(SENSOR_PORT);
        std::cout << "Listening for sensor data on port " << SENSOR_PORT << " (blocking)" << std::endl;

        // Create PID controllers (example gains)
        PID pid_roll(1.0, 0.01, 0.05, -1.0, 1.0);
        PID pid_pitch(1.0, 0.01, 0.05, -1.0, 1.0);
        PID pid_yaw(1.0, 0.0, 0.02, -1.0, 1.0);

        Mixer mixer(1000, 2000);

        char recvbuf[1024];
        while (true) {
            struct sockaddr_in from;
            ssize_t n = udp.blockingReceive(recvbuf, sizeof(recvbuf) - 1, &from);
            if (n <= 0) {
                std::cerr << "recvfrom returned <=0, n=" << n << std::endl;
                continue;
            }
            recvbuf[n] = '\0';
            std::string msg(recvbuf);
            SensorPacket packet;
            if (!parseSensorCSV(msg, packet)) {
                std::cerr << "Malformed sensor packet: '" << msg << "'" << std::endl;
                continue; // Keep blocking for next packet - lock-step
            }

            // Example setpoints: keep roll/pitch/yaw at zero, thrust set to sensor thrust as default
            double target_roll = 0.0;
            double target_pitch = 0.0;
            double target_yaw = 0.0;
            double dt = 0.02; // assume 50Hz control loop - dt might come from sensor timestamp in a real system
            // compute control outputs
            double cor_roll = pid_roll.compute(target_roll, packet.roll, dt);
            double cor_pitch = pid_pitch.compute(target_pitch, packet.pitch, dt);
            double cor_yaw = pid_yaw.compute(target_yaw, packet.yaw, dt);

            // Inputs to mixer: roll/pitch/yaw corrections [-1..1], thrust in [0..1]
            float thrust = std::clamp(packet.thrust, 0.0f, 1.0f);
            std::array<int,4> motors = mixer.mix((float)cor_roll, (float)cor_pitch, (float)cor_yaw, thrust);

            // pack motor command CSV: m1,m2,m3,m4
            std::ostringstream cmd;
            cmd << motors[0] << "," << motors[1] << "," << motors[2] << "," << motors[3];
            std::string cmdstr = cmd.str();
            udp.sendTo(cmdstr.c_str(), cmdstr.size(), LOCALHOST, MOTOR_COMMAND_PORT);

            // Send telemetry to ground control on port 9003
            std::ostringstream tel;
            tel << "TELEM," << "alt=" << packet.altitude
                << ",gs=" << packet.groundspeed
                << ",batt=" << packet.battery
                << ",r=" << packet.roll
                << ",p=" << packet.pitch
                << ",y=" << packet.yaw
                << ",m1=" << motors[0]
                << ",m2=" << motors[1]
                << ",m3=" << motors[2]
                << ",m4=" << motors[3];
            std::string telas = tel.str();
            udp.sendTo(telas.c_str(), telas.size(), LOCALHOST, GC_TELEM_PORT);

            // For developer visibility
            std::cout << "Received sensor packet @ " << packet.timestamp << " -> motors: " << cmdstr << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}