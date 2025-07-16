# Stepper Motor Control: M5Stack Core (Basic) + Module 13.2

**Author:** John Zawodniak  
**Date:** 2024-02-01  
**Version:** 1.0

## Overview

This code provides a **simple way to control two stepper motors** using the M5Stack Core (Basic) hardware with the Module 13.2 add-on. It leverages the **FastAccelStepper** library for precise stepper movement and the **Module_Stepmotor** library for hardware interfacing.

- **Button A** on the device moves both motors forward by one revolution.
- **Button C** moves both motors backward by one revolution.
- Current **pulse counts** (the number of steps taken) for both motors are **shown on the LCD** screen.
- **Full step mode** is used (*no microstepping*).
- Motors are connected to:  
  - X axis: Pins 16 (step), 17 (dir)  
  - Y axis: Pins 12 (step), 13 (dir)

## Features

- **Hardware:** M5Stack Core (Basic) + Module 13.2
- **Software:** PlatformIO, Arduino, M5Unified library
- **Control:** Buttons A (forward) and C (reverse) on the device
- **User Feedback:** LCD displays pulse counts for each motor
- **Movement:** Both motors move together, one revolution per button press

## Setup

Sample PlatformIO configuration:

[env:m5stack-core]
platform = espressif32
board = m5stack-core-esp32
framework = arduino
lib_deps =
m5stack/M5Unified @ ^0.2.5
m5stack/Module_Stepmotor @ ^0.0.2
gin66/FastAccelStepper @ ^0.31.5
upload_speed = 115200


## Key Functions

- `moveBothMotors(steps)`: Initiates movement (one or both directions) for both motors.
- `drawStatus()`: Updates the LCD with latest pulse counts.
- `loop()`: Checks for button presses (A: forward, C: reverse), moves motors, and updates display.

## How It Works

1. **Initialization**: Libraries, pins, and motors are set up in `setup()`. LCD displays a starting message.
2. **Motor Control**: When a button press is detected in `loop()`, both motors move either forward or backward by exactly one revolution (200 steps).
3. **Status Update**: After every movement, the LCD shows the current number of pulses for each motor, providing real-time feedback.

> This minimal codebase is designed for learning and quick integration with M5Stack hardware.  
> For further customization or more advanced movement, expand the button controls or adjust step/motor parameters as desired.
> Realistic Expecations - You should expect some jitter and step loss from your motors as you are in full step mode.  Spinning the motor up and down 
> will create skips in steps. Investigate microstepping if more precision is required.

