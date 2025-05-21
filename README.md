# Autonomous Car Robot with Obstacle Avoidance and Infrared Detection (KAYO TEAM)

## Overview
This project involves designing, building, and programming a robot capable of both remote-controlled operation and autonomous navigation. The robot Detection  lane while avoiding obstacles using Arduino and Raspberry Pi. Part of Kuwait University, Robotics class with Dr. Alshaibani.

### Project Phases:
1. **Phase 1:** Assembly and remote operation.
2. **Phase 2:** Autonomous driving with obstacle avoidance.

## Key Features
- **Remote Control:** Manual operation via a web interface (Flask server on Raspberry Pi).
- **Autonomous Navigation:** Utilizes Q-learning for path-following and obstacle avoidance.
- **Sensor Integration:**
  - 3x IR Sensors for line detection.
  - 3x Ultrasonic Sensors for obstacle detection.
- **Motor Control:** L298N motor driver with 4 DC motors.
- **Communication:** UART serial between Raspberry Pi (main controller) and Arduino (motor/sensor handler).

## Components

### **Hardware**
- Raspberry Pi 4 (Main controller, runs Flask server).
- Arduino Uno (Handles motor control & sensor readings).
- L298N Motor Driver (Controls DC motors).
- 4x DC Motors with Wheels (Movement).
- 3x IR Infrared Sensors (Line detection).
- 3x Ultrasonic Sensors (HCSR04) (Obstacle detection).
- Power Bank & Li-ion Battery (Power supply).

### **Software & Tools**
- Python (Flask server, Q-learning algorithm).
- Arduino IDE (Motor/sensor control logic).
- GitHub (Version control).

## Phase 1: Remote Control

### **Assembly & Operation**
- Flask web server allows manual control (forward, backward, left, right).
- Commands sent via UART to Arduino, which drives motors.

## Phase 2: Autonomous Mode

### **Q-Learning Algorithm**
- **States:** IR sensor readings (e.g., `000` = all white, `010` = middle black).
- **Actions:** Forward, left, right, scan_left, scan_right, strongLeft, strongRight.
- **Rewards:** Predefined based on sensor inputs (e.g., staying centered = high reward).
- **Training:** Robot learns optimal path-following via reinforcement learning.

### **Obstacle Avoidance**
- Ultrasonic sensors detect obstacles (threshold: 20cm).
- If an obstacle is detected, the robot switches lanes and signals with LEDs.

## Setup & Usage

### **Setup Guide**
#### **Hardware Assembly**
1. Connect motors, sensors, and controllers as per the wiring diagram.
2. Power Raspberry Pi (USB-C) and Arduino (Li-ion battery).

#### **Software Setup**
1. Upload Arduino code to handle sensors/motors.
2. Run Flask server on Raspberry Pi (`app.py`) for web interface.

#### **Training Autonomous Mode**
1. Start training via the web interface.
2. Robot explores and updates Q-table (`model.pth`).

## Demo

### **Robot Demo**
- Place robot on a track with lanes and obstacles.
- Switch between manual and autonomous modes.

## Challenges & Improvements

### **Challenges**
- Sensor calibration for varying light conditions.
- Motor synchronization issues.
- Q-learning convergence time.

### **Future Work**
- Fix Issues with Q-learning
- Add a camera for advanced object recognition.

## Contributors
- **Hala Almutairi**
- **Zaharaa Alrashidi**
