# Change Documentation: M5Stack Dual Stepper Speed Cycling (v2.1)
**Date:** Wednesday, July 16, 2025

## Overview

This update adds interactive motor speed control using Button B on the M5Stack Basic controller. Each press of Button B cycles the stepper motors’ speed through six preset levels (0%, 20%, 40%, 60%, 80%, 100%) and updates the user interface on screen. Function declarations were also standardized to resolve scope-related compilation errors.

---

## Summary of Changes

### 1. **Speed Control via Button B**
- Added two arrays:
  - `speedLevels[]` containing Hz values representing motor speed (0, 100, 200, 300, 400, 500 Hz).
  - `speedPercentages[]` containing matching percentages (0, 20, 40, 60, 80, 100).
- Introduced `currentSpeedIndex` to keep track of the active speed level.
- Implemented `updateSpeed()` function:
  - Increments `currentSpeedIndex` on each Button B press, wrapping to 0 after the final level.
  - Sets the speed of both stepper motors using `setSpeedInHz()`.
- Pressing **Button B** cycles through the available speed percentages.

### 2. **User Interface Updates**
- `drawInstructions()` displays:
  - Usage instructions (“Press B to change speed”).
  - Current speed percentage.
- LCD layout refreshed so the user always knows how to change speed and what the current speed setting is.

### 3. **Code Structure and Prototypes**
- Added full function prototypes at the top of the code for all user-defined functions to prevent scope and compilation issues.
- Ensured all logic for stepping and speed changes is self-contained and clearly structured.

### 4. **Behavioral Summary**
- **Button A:** Moves both motors forward one full revolution at the current speed.
- **Button C:** Moves both motors backward one full revolution at the current speed.
- **Button B:** Cycles through preset speed percentages and updates both motors and the LCD.
- Display always shows:
  - Number of pulses for X and Y.
  - Instructions for using Button B and the current speed percentage.

---

## Example UI (LCD)
Stepper Ready (1/16 Step)
A: FWD 1rev C: REV 1rev
X Pulses: 3200
Y Pulses: 3200
Press B to change speed
Speed: 40%


---

## Rationale
- *Improves usability* by enabling on-the-fly speed adjustment without reprogramming.
- *Gives clear feedback* to the user regarding motor speed and available controls.
- *Adheres to good coding practice* with function declarations and logical separation of features.

---

**This version is ready for user speed experimentation and further expansion.**
