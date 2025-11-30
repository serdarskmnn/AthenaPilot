# AthenaPilot - Hybrid SITL Flight Control (Local UDP Lock-step)

This repo contains a simple scaffolding for a Hybrid SITL Flight Control system with three components:
- Simulink (physics engine) — not included here, presumed to run and communicate via UDP.
- Firmware (C++), built using CMake.
- Ground Control Station (Python, PyQt6).

## Ports (UDP, localhost)
- Simulink -> Firmware: 9002 (sensor data), blocking (master clock)
- Firmware -> Simulink: 9001 (motor outputs)
- GCS -> Firmware: 9000 (RC channel commands), non-blocking
- Firmware -> GCS: 9003 (telemetry)

## Build & Run - Firmware (C++)
Requirements:
- CMake >= 3.16
- GCC / Clang supporting C++17

Commands (inside project root):

```bash
mkdir -p firmware/build
cd firmware/build
cmake ..
cmake --build . -- -j$(nproc)
# You can run the firmware binary:
./athena_firmware
```

Notes: If you use an external Simulink or physics engine, ensure it sends sensor UDP payloads to `127.0.0.1:9002` and listens for motor outputs on `127.0.0.1:9001`. The firmware blocks waiting for sensor packets (lock-step behavior).

## Run - Ground Control Station (Python, PyQt6)
Requirements:
- Python 3.10+
- PyQt6 (pip install PyQt6)

Commands (inside project root):

```bash
python -m pip install PyQt6
python -m ground_control.main
```

Common issue: If you see this error while running from inside `ground_control/`:

```
ModuleNotFoundError: No module named 'ground_control'
```

That means your current working directory is not the repository root and Python cannot find the `ground_control` package.
Run the command from the repository root where `ground_control` is a subfolder, or use `PYTHONPATH`:

```bash
# from repo root
python -m ground_control.main

# or from inside `ground_control/` folder
PYTHONPATH=.. python -m ground_control.main
```

Controls (keyboard):
- W/S: Pitch Down/Up
- A/D: Roll Left/Right
- Up/Down arrows: Throttle Increase/Decrease
- Left/Right arrows: Yaw Left/Right
- Arm/Disarm: toggle button
- Mode: dropdown (STABILIZE, LOITER, LAND)

Telemetry appears in the console and the HUD updates based on Roll/Pitch values.

## Notes and Extensions
- The communication protocol is a simple CSV `KEY:VALUE` format for telemetry, motor outputs and RC commands.
- The `firmware` code provided is a simple, readable demo scaffold. In a production avionics system, replace the parsing and mixing with robust binary protocols, CRC checks, bespoke encoders, and safety-critical features.
- For Simulink integration, configure a UDP send/receive block to match the port definitions above; ensure lock-step (Simulink pauses until a motor response is received) if needed.

## Files of Interest
- `firmware/src/main.cpp` - Main firmware loop implementing the lock-step, state machine, and non-blocking RC listener.
- `firmware/src/pid.cpp` - PID controller.
- `firmware/src/mixer.cpp` - Simple quadcopter mixer.
- `ground_control/main.py` - PyQt application entrypoint.
- `ground_control/gui/main_window.py` - PyQt main window and keyboard handling.
- `ground_control/gui/hud_widget.py` - Artificial horizon widget (simple painter).
- `ground_control/comms/udp_link.py` - UDP link client & telemetry listener.

---

If you'd like, I can:
- Add unit tests for the C++ PID and mixer.
- Add example Simulink UDP block payloads / MATLAB script for local testing.
- Add a small Python-based simulator to send fake sensor data to test the lock-step logic.
