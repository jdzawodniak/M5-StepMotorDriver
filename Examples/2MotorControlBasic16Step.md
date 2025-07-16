# Summary: Enabling 1/16 Microstepping on M5Stack Stepper Motor Module

## Hardware Changes
- **Microstepping Pad Configuration:**
  - Soldered (jumpered) the **M2 pad** on the Module 13.2 for each motor channel.
  - Left **M1** and **M0** pads unsoldered (logic LOW).
- **Result:** This sets the motor driver to **1/16 microstepping mode** according to the M5Stack documentation.

## Software Changes
- **Microstepping Definition:**
#define MICRO_STEPS 16

text
- **Updated Steps per Revolution:**
#define FULL_STEP_PER_REV 200
#define STEPS_PER_REV (FULL_STEP_PER_REV * MICRO_STEPS) // 3200 steps/rev

- **Motion Commands Updated:**
- Movement functions updated to command **3200 steps per revolution** instead of 200 steps.
- Example:
  ```
  moveBothMotors(STEPS_PER_REV);   // Move 1 full revolution at 1/16 microstepping
  moveBothMotors(-STEPS_PER_REV);  // Move 1 full revolution backward
  ```
- **UI Feedback:**
- LCD initialization message updated to indicate **"Stepper Ready (1/16 Step)"**.

## Effect of Changes
- The motor now moves **1/16 the mechanical angle per input pulse**, resulting in much smoother motion.
- Full revolution requires 3200 pulses due to microstepping.
- Torque is slightly reduced per microstep, but overall movement quality and precision are improved.

---

**Note:** Always ensure hardware microstepping pads and software step counts are synchronized for correct motor behavior.
