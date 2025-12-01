# AthenaPilot - Hybrid SITL Flight Control System

This user guide explains how to compile the C++ firmware and run the Python Ground Control Station (GCS).

Prerequisites
- Linux (tested on Ubuntu)
- CMake (>= 3.10), a C++17-compatible compiler (g++/clang)
- Python 3.8+ with PyQt6

Build firmware
1. From the repository root, create the build directory and run cmake:

```bash
cd firmware
mkdir -p build && cd build
cmake ..
make -j
```

2. After building, the binary will be `firmware/build/athena_firmware`.

Run firmware and GCS
1. Start the firmware:

```bash
./firmware/build/athena_firmware
```

2. Start the Python Ground Control Station (in a separate terminal):

```bash
cd ground_control
python3 main.py
```

Notes on Ports (UDP on localhost):
- Simulink -> Firmware (Sensor): port 9002 (blocking - lock-step)
- Firmware -> Simulink (Motor commands): port 9001
- Python GCS -> Firmware (RC/Commands): port 9000 (non-blocking)
- Firmware -> Python GCS (Telemetry): port 9003

Keyboard mappings in the GCS:
- W/S: Pitch (Spring stick - centers on release)
- A/D: Roll (Spring stick - centers on release)
- Z/X: Yaw (Spring stick - centers on release)
- F/V: Throttle (Persists on release)

Basic messages over UDP
- RC: "RC,<roll>,<pitch>,<yaw>,<throttle>" (integers 1000-2000)
- ARM: "ARM,1" or "ARM,0"
- MODE: "MODE,<STABILIZE|LOITER|LAND>"
- SENSOR (from Simulink): "SENSOR,<roll>,<pitch>,<yaw>,<dt>"
- MOTOR (from firmware to Simulink): "MOTOR,<m1>,<m2>,<m3>,<m4>"
- TELEM (from firmware to Python): "TELEM,<roll>,<pitch>,<yaw>"

Optional: Run a simple Simulink mock simulator (for testing lock-step behavior):

```bash
python3 tools/sim_simulink.py
```

This sends sensor messages to port 9002 and waits for motor outputs on 9001; use this to validate the firmware's lock-step handling.

That's it. This is a minimal scaffold to get an SITL lock-step simulation setup with a PyQt6 ground control UI. Expanding to include real Simulink integration and richer UI telemetry is straightforward.
