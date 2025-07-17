/*
*******************************************************************************
* Author: John Zawodniak
* Date: 2024-02-01
* Version: 2.7
* Description:
*   Controls two stepper motors using M5Stack Core ESP32 and Module 13.2 driver.
*   Implements 1/16 microstepping, speed control cycling with Button B,
*   movement for multiple revolutions with Buttons A (forward) and C (reverse),
*   and proper stopping behavior when speed is zero.
*
* Key Features:
* - Smooth ramp-up and ramp-down with configurable acceleration.
* - Speed steps: 0%, 20%, 40%, 60%, 80%, 100% mapped to microstep frequencies.
* - Auto enable pin control via FastAccelStepper's setAutoEnable(true).
* - Stops motors cleanly when speed is zero to prevent unwanted rotation.
*******************************************************************************
*/

#include <Arduino.h>
#include <FastAccelStepper.h>
#include <M5Unified.h>
#include <Module_Stepmotor.h>

// Function prototypes for clarity and compiler correctness
void drawStatus();
void drawInstructions();
void moveBothMotors(int32_t steps);
void updateSpeed();

// Pin definitions for M5Stack Core (Basic) + Module 13.2
#define X_DIR_PIN 17     // Direction control pin for X motor
#define X_STEP_PIN 16    // Step control pin for X motor
#define Y_DIR_PIN 13     // Direction control pin for Y motor
#define Y_STEP_PIN 12    // Step control pin for Y motor

// Stepper motor constants
#define FULL_STEP_PER_REV 200            // Number of full steps per motor revolution (1.8° steps)
#define MICRO_STEPS 16                   // Microstepping factor (1/16 microstepping)
#define STEPS_PER_REV (FULL_STEP_PER_REV * MICRO_STEPS)  // Total microsteps per revolution (3200 steps)

// Adjustable runtime parameters
int accelerationRate = 2000;             // Acceleration in steps/sec² for ramping speed
int revolutionsPerMove = 5;              // Number of revolutions moved per Button A or C press

// FastAccelStepper engine and motor stepper pointers
FastAccelStepperEngine engine;
FastAccelStepper* steppers[2] = {nullptr, nullptr}; // Pointer array for X and Y motors

// Module_Stepmotor driver instance for motor control communication
Module_Stepmotor driver;

// Track the number of pulses sent to each motor (for display)
long pulseCounts[2] = {0, 0};

// Define the speeds: microsteps per second values, and corresponding speed percentages
const int speedLevels[] = {0, 1600, 3200, 4800, 6400, 8000};   // Speeds in microsteps/sec (Hz)
const int speedPercentages[] = {0, 20, 40, 60, 80, 100};       // Display percentages
const int speedLevelsCount = sizeof(speedLevels) / sizeof(speedLevels[0]);
int currentSpeedIndex = 0;           // Index for current speed in speedLevels array

/**
 * Updates the on-screen instructions and status to show user controls
 * and current speed/revolutions/acceleration settings.
 */
void drawInstructions() {
  M5.Lcd.fillRect(0, 100, 320, 50, BLACK);  // Clear instruction area
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.printf("Press B to change speed\n");
  M5.Lcd.printf("Speed: %d%%\n", speedPercentages[currentSpeedIndex]);
  M5.Lcd.printf("Move %d revolutions\n", revolutionsPerMove);
  M5.Lcd.printf("Accel: %d\n", accelerationRate);
}

/**
 * Initialization code, runs once at startup.
 * Sets up M5Stack, initializes steppers, driver, LCD, and serial.
 */
void setup() {
  auto cfg = M5.config();       // Default M5Stack config
  M5.begin(cfg);                // Initialize M5Stack
  Serial.begin(115200);         // Serial for debug output
  Serial.println("Setup starting...");

  // Initialize LCD display
  M5.Lcd.setTextSize(2);
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Stepper Ready (1/16 Step)");
  M5.Lcd.println("A: FWD 5rev  C: REV 5rev");

  // Initialize stepper engine
  engine.init();

  // Connect the step pins to the stepper engine and configure direction pins
  steppers[0] = engine.stepperConnectToPin(X_STEP_PIN);  // X axis stepper
  steppers[1] = engine.stepperConnectToPin(Y_STEP_PIN);  // Y axis stepper

  // Setup each stepper motor's direction pin, acceleration, speed, and enable auto management of enable pin
  for (int i = 0; i < 2; i++) {
    if (steppers[i]) {
      int dirPin = (i == 0) ? X_DIR_PIN : Y_DIR_PIN;
      steppers[i]->setDirectionPin(dirPin);     // Set direction control pin
      steppers[i]->setAutoEnable(true);          // Let library manage enable pin automatically
      steppers[i]->setAcceleration(accelerationRate);    // Set acceleration rate
      steppers[i]->setSpeedInHz(speedLevels[currentSpeedIndex]);  // Initial speed (likely zero)
      Serial.printf("Initial speed stepper %d: %d Hz\n", i, speedLevels[currentSpeedIndex]);
      Serial.printf("Acceleration: %d\n", accelerationRate);
    }
  }

  // Initialize I2C and motor driver (Module 13.2)
  Wire.begin(21, 22, 400000UL);
  driver.init(Wire);
  driver.resetMotor(0, 0);      // Reset motor 0 (X)
  driver.resetMotor(1, 0);      // Reset motor 1 (Y)
  driver.enableMotor(1);        // Enable driver chip (both motors)

  // Display initial UI elements
  drawInstructions();
  drawStatus();

  Serial.println("Setup complete.");
}

/**
 * Moves both motors by the specified number of microsteps.
 * If speed is zero, motors are stopped and no move is issued.
 * @param steps Number of microsteps to move (positive or negative)
 */
void moveBothMotors(int32_t steps) {
  // If speed is zero, do not move but ensure motors are stopped cleanly
  if (speedLevels[currentSpeedIndex] == 0) {
    Serial.println("Speed is 0, skipping move and stopping motors.");
    for (int i = 0; i < 2; i++) {
      if (steppers[i]) {
        steppers[i]->setSpeedInHz(0);   // Set stepper speed to zero (no pulses)
        steppers[i]->stopMove();        // Stop any ongoing moves immediately
      }
    }
    return;  // Exit without initiating move
  }

  Serial.printf("Moving motors by %ld steps at speed index %d (%d Hz)\n",
                steps, currentSpeedIndex, speedLevels[currentSpeedIndex]);
  Serial.printf("Acceleration: %d\n", accelerationRate);

  // Configure each stepper and issue the move command
  for (int i = 0; i < 2; i++) {
    if (steppers[i]) {
      steppers[i]->setAcceleration(accelerationRate);                // Set acceleration
      steppers[i]->setSpeedInHz(speedLevels[currentSpeedIndex]);     // Set speed for this move
      steppers[i]->move(steps);                                      // Start move (non-blocking)
      pulseCounts[i] += steps;                                       // Update pulse counters
    }
  }

  // Wait for both motors to finish the move (blocking)
  while (
    (steppers[0] && steppers[0]->isRunning()) ||
    (steppers[1] && steppers[1]->isRunning())
  ) {
    delay(10);
  }
  Serial.println("Move complete.");
}

/**
 * Updates the RTC and Y pulses count shown on the LCD.
 * Clears previous pulse count area before writing.
 */
void drawStatus() {
  M5.Lcd.fillRect(0, 40, 320, 60, BLACK);  // Clear pulse count display area
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.printf("X Pulses: %ld\n", pulseCounts[0]);
  M5.Lcd.printf("Y Pulses: %ld\n", pulseCounts[1]);
}

/**
 * Cycles the speed setting to the next value in the speed array.
 * If speed is set to zero, motors are stopped immediately.
 * Otherwise motors remain ready for moves at new speed.
 */
void updateSpeed() {
  currentSpeedIndex++;
  if (currentSpeedIndex >= speedLevelsCount) {
    currentSpeedIndex = 0;  // Wrap around to start of speed array
  }
  Serial.printf("Speed changed to index %d (%d Hz = %d%%)\n",
                currentSpeedIndex, speedLevels[currentSpeedIndex], speedPercentages[currentSpeedIndex]);

  // Update motor speed and stop if zero speed selected
  if (speedLevels[currentSpeedIndex] == 0) {
    for (int i = 0; i < 2; i++) {
      if (steppers[i]) {
        steppers[i]->setSpeedInHz(0);    // Ensure speed zero
        steppers[i]->stopMove();         // Stop immediately
      }
    }
  } else {
    for (int i = 0; i < 2; i++) {
      if (steppers[i]) {
        steppers[i]->setSpeedInHz(speedLevels[currentSpeedIndex]);   // Set new speed
      }
    }
  }

  drawInstructions();
}

/**
 * Main loop - handles button input and triggers motor moves/speed changes.
 * Button A -> move forward by revolutionsPerMove revolutions.
 * Button C -> move backward by revolutionsPerMove revolutions.
 * Button B -> cycle through speed settings.
 */
void loop() {
  M5.update();     // Update button states

  if (M5.BtnA.wasClicked()) {
    moveBothMotors(STEPS_PER_REV * revolutionsPerMove);
    drawStatus();
  }

  if (M5.BtnC.wasClicked()) {
    moveBothMotors(-STEPS_PER_REV * revolutionsPerMove);
    drawStatus();
  }

  if (M5.BtnB.wasClicked()) {
    updateSpeed();
  }
}
