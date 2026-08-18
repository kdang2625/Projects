# ESP32 ABS Prototype & Wireless Autonomous RC Car

## Overview
This directory contains two embedded-systems projects built around ESP32 microcontrollers:

1. **ABS for Skateboard** — a prototype braking and sensor-telemetry system that communicates with a Python application over Bluetooth Low Energy (BLE).
2. **Wireless Autonomous RC Car** — an ESP32-based vehicle with wireless joystick control, motor control, obstacle sensing, autonomous behavior, braking, and controller feedback.

Both projects focus on hardware/software integration, real-time inputs, wireless communication, sensor data, and system-level debugging.

## ABS for Skateboard

### Technologies
- ESP32 / Arduino C++
- Bluetooth Low Energy (BLE)
- Python
- Bleak
- `asyncio`
- Sensor telemetry
- Brake-control experimentation

### Key Features
- BLE device discovery and connection
- GATT characteristic reads and writes
- Asynchronous BLE notifications
- Timestamped sensor-data reception
- ESP32-to-Python communication
- Python-side data processing

### Files
```text
ABSforSkateboard/
├── ESP32/
├── README.md
├── ble_client.py
├── brake_control.ino
└── graph.py
```

### Python Setup
```bash
python3 -m pip install bleak
python3 ble_client.py
```

The ESP32 should be powered, advertising, and running the matching firmware before the client is started.

## Wireless Autonomous RC Car

### Overview
The RC car uses two ESP32 devices: one as a handheld controller and one on the vehicle. The controller sends joystick and button data using ESP-NOW. The vehicle receives those packets, converts joystick input into left/right motor commands, and can switch into an autonomous obstacle-avoidance mode using a VL6180X time-of-flight sensor.

### Technologies
- ESP32 / Arduino C++
- ESP-NOW
- Wi-Fi radio configuration
- I2C
- VL6180X time-of-flight sensor
- PWM motor control
- 4D Systems display
- Finite-state-machine control

### Key Features
- Low-latency ESP-NOW controller-to-car communication
- Packed control packets with joystick values, button bitmask, and sequence number
- Joystick calibration and dead-zone handling
- Differential left/right motor control
- Manual and autonomous driving modes
- Time-of-flight obstacle detection
- Forward / reverse / turn autonomous state machine
- Brake and horn outputs
- Controller display feedback
- LED and Serial debugging

### System Architecture
```text
Joystick + Buttons
       |
       v
Controller ESP32
       |
       | ESP-NOW ControlPacket
       v
Vehicle ESP32
   |        |         |
   v        v         v
Motors   ToF Sensor  Brake/Horn
```

### Files
```text
ABS&RCCar/
├── ABSforSkateboard/
├── autonomousCarCode/
│   └── CarCode.ino
└── wirelessController/
    └── ControllerCode.ino
```

## Engineering Concepts Demonstrated
- Embedded C/C++
- BLE and ESP-NOW
- Asynchronous Python programming
- Custom packet structures and bitmasks
- Sensor integration
- PWM motor control
- Finite-state machines
- Hardware/software troubleshooting
- Input calibration and noise handling

## Key Troubleshooting Work
Analog joystick noise could cause unintended motion. The controller and vehicle logic use calibration and dead zones so small deviations around the joystick center are not interpreted as motor commands. Packet sequence numbers, Serial output, and LED status indicators also help validate wireless communication.
