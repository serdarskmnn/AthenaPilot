# 🦉 AthenaPilot: Advanced Hybrid Flight Control System

![C++](https://img.shields.io/badge/Firmware-C%2B%2B17-blue?logo=c%2B%2B) ![Python](https://img.shields.io/badge/Ground_Control-Python_3.10-yellow?logo=python) ![MATLAB](https://img.shields.io/badge/Simulation-MATLAB_%26_Simulink-orange?logo=mathworks) ![License](https://img.shields.io/badge/License-MIT-green)

**AthenaPilot** is a modular, open-source flight control ecosystem designed for **VTOL (Vertical Take-Off and Landing)** UAVs. Unlike traditional autopilots, it decouples the control logic, mission planning, and physical simulation into three specialized environments, enabling rapid prototyping and Hardware-in-the-Loop (HIL) testing.

---

## 🏗️ System Architecture: "The Triad"

The project is built upon a **Tri-Layer Architecture**, where each component runs independently but communicates in real-time.

```mermaid
graph TD;
    A[🐍 Python Navigator] -- Waypoints / Velocity --> B[⚡ C++ Flight Controller];
    B -- Motor PWM --> C[🔬 MATLAB Physics Engine];
    C -- Sensor Data (IMU/GPS) --> B;
    B -- Telemetry --> A;

    1. ⚡ The Brain: Firmware (C++)
Role: Real-time flight control, sensor fusion, and actuator mixing.

Target Hardware: STM32 F7/H7 Series (Nucleo-144).

Features:

Custom Scheduler & RTOS integration.

Cascaded PID Control Loop (400Hz).

VTOL Transition Logic (State Machine).

2. 🐍 The Navigator: Ground Control (Python)
Role: High-level mission planning and computer vision.

Target Hardware: Companion Computer (Raspberry Pi / Jetson) or Ground Station.

Features:

Autonomous Path Planning (A* Algorithm).

Object Detection & Visual Servoing.

MAVLink Telemetry Bridge.

3. 🔬 The Simulator: Physics Engine (MATLAB/Simulink)
Role: High-fidelity flight dynamics simulation & HIL Interface.

Target: Desktop PC.

Features:

6-DOF Rigid Body Dynamics.

Battery Voltage Sag & Power Consumption Model.

Environmental disturbances (Wind, Turbulence).

AthenaPilot/
│
├── firmware/           # C++ Source Code for STM32
│   ├── src/            # Drivers, PID, Mixer
│   └── include/        # Header files
│
├── ground_control/     # Python Scripts for Mission Planning
│   ├── vision/         # OpenCV & AI tracking scripts
│   └── mission/        # Autonomous waypoint logic
│
├── simulation/         # MATLAB & Simulink Models
│   ├── plant_models/   # Physics engine files (.slx)
│   └── hil_interface/  # Hardware-in-the-Loop blocks
│
└── docs/               # System Architecture & Datasheets


🚀 Roadmap
[ ] Phase 1: Setup Basic C++ Project Structure & CMake.

[ ] Phase 2: Implement "Virtual Sensors" class in C++ to read data from MATLAB via Serial/UDP.

[ ] Phase 3: Port PID Logic from MATLAB to C++.

[ ] Phase 4: Establish Python-to-C++ Communication (MAVLink or Custom).

[ ] Phase 5: First "Virtual Flight" with all three systems running in sync.

🤝 Contributing
This is a research project focused on Model-Based Design (MBD). If you want to contribute, please check the issues page or submit a PR.

Author: Serdar Sökmen🚀 Roadmap
[ ] Phase 1: Setup Basic C++ Project Structure & CMake.

[ ] Phase 2: Implement "Virtual Sensors" class in C++ to read data from MATLAB via Serial/UDP.

[ ] Phase 3: Port PID Logic from MATLAB to C++.

[ ] Phase 4: Establish Python-to-C++ Communication (MAVLink or Custom).

[ ] Phase 5: First "Virtual Flight" with all three systems running in sync.

🤝 Contributing
This is a research project focused on Model-Based Design (MBD). If you want to contribute, please check the issues page or submit a PR.

Author: Serdar Sökmen