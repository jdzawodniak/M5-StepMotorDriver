# Summary of Stepper Motor Control Code Functionality

## Overview
This code controls two stepper motors connected to an M5Stack Core ESP32 device with a Module 13.2 stepper driver. It enables precise positioning with 1/16 microstepping and interactive speed control, leveraging the FastAccelStepper library.

## Key Features

### 1. Microstepping Configuration
- Uses 1/16 microstepping mode (3200 microsteps per full revolution).
- Motor step and direction pins are assigned for X and Y motors.

### 2. Speed Control
- Six predefined speed levels: 0%, 20%, 40%, 60%, 80%, and 100%, corresponding to microstep frequencies ranging from 0 to 8000 Hz.
- Button B cycles through these speed levels.
- Speed updates are immediately reflected in motor behavior:
  - At 0% speed, motors are fully stopped and no movement occurs.
  - At non-zero speeds, motors are ready to move at the selected speed.

### 3. Movement Control
- Button A commands the motors to move forward a configurable number of revolutions (`revolutionsPerMove`), default is 5.
- Button C commands the motors to move backward the same amount.
- Acceleration for motor movements is configurable (`accelerationRate`, default 2000 steps/sec²) to ensure smooth starting and stopping.
- Motor moves block program execution until completed, ensuring synchronous updates.

### 4. User Interface
- The M5Stack built-in display shows:
  - Current speed percentage.
  - Number of revolutions per move.
  - Acceleration rate.
  - Number of pulses moved by each motor.
  - Instructions for interacting with buttons.

### 5. Safety and Control
- Motors stop immediately when speed is set to zero, preventing unwanted motion.
- Stepper enable pins are managed automatically via the FastAccelStepper library (`setAutoEnable(true)`).
- Proper use of stopping methods (`stopMove()`) ensures reliable halting of stepper pulses.

## Summary Table for User Controls

| Button | Function                                      |
|--------|-----------------------------------------------|
| A      | Move motors forward by configured revolutions |
| C      | Move motors backward by configured revolutions |
| B      | Cycle through preset speed settings (0-100%)  |

## Hardware Requirements and Setup
- M5Stack Core ESP32 device.
- Module 13.2 Stepper driver hardware with jumpers configured for 1/16 microstepping mode.
- Motors wired to the configured direction and step pins.
- Power supply compatible with motor and driver specifications.

## Notes
- The code uses blocking calls during motor movement, meaning no other processing occurs during a move.
- Serial output at 115200 baud provides debugging and status messages.
- Adjust `accelerationRate` and `revolutionsPerMove` variables to tune responsiveness and duration of moves.

## Potential Extensions
- Add runtime control over acceleration and revolution count.
- Implement continuous rotation mode for constant speed spinning.
- Include error handling or stall detection for robustness.

---
This solution provides a solid basis for precise, user-interactive control of dual stepper motors with smooth acceleration and multiple speed tiers on the M5Stack platform.
