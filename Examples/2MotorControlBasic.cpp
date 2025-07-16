
/*
*******************************************************************************
* Author: John Zawodniak
* Date: 2024-02-01
* Version: 1.0
* Provided is the base code for a simple stepper motor control
* using the M5Stack Core (Basic) and Module 13.2.
* The code uses the FastAccelStepper library for stepper control
* and the Module_Stepmotor library for M5Stack module control.
* Button A moves both motors forward one revolution,
* Button C moves both motors backward one revolution.
* PulseCounts are displayed on the M5Stack LCD.
* Motors are configured for full step mode (no microstepping).
* Motors are connected to pins 16, 17 (X) and 12, 13 (Y).
* The code is designed to be simple and easy to understand.
* The platformio.ini file is provided in comment below for easy setup.
* 
* Coding environment: PlatformIO with M5Unified library
* C++ language standard version: C++11
*******************************************************************************
*/


// platformio.ini example configuration for M5Stack Core (Basic) + Module 13.2 (copy to your platformio.ini file):
// [env:m5stack-core]
// platform = espressif32
// board = m5stack-core-esp32
// framework = arduino
// lib_deps =
//   m5stack/M5Unified @ ^0.2.5
//    m5stack/Module_Stepmotor @ ^0.0.2
//    gin66/FastAccelStepper @ ^0.31.5
//  upload_speed = 115200


// This code is designed to work with the M5Stack Core (Basic) and Module 13.2


#include <Arduino.h>
#include <FastAccelStepper.h>
#include <M5Unified.h>
#include <Module_Stepmotor.h>

// Pin configuration for M5Stack Core (Basic) + Module 13.2
#define X_DIR_PIN 17
#define X_STEP_PIN 16
#define Y_DIR_PIN 13
#define Y_STEP_PIN 12

#define FULL_STEP_PER_REV 200    // Full step, because no jumpers = full step mode
#define MICRO_STEPS 1            // No microstepping (hardware: no jumpers)
#define STEPS_PER_REV (FULL_STEP_PER_REV * MICRO_STEPS)   // = 200

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *steppers[2] = {nullptr, nullptr};

Module_Stepmotor driver;
long pulseCounts[2] = {0, 0};  // Total pulses per motor

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);

  M5.Lcd.setTextSize(2);
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Stepper Ready (Full Step)");

  // Stepper driver initialization  
  engine.init();

  // Connect Step pins and configure direction pins for each stepper
  steppers[0] = engine.stepperConnectToPin(X_STEP_PIN);  // X
  steppers[1] = engine.stepperConnectToPin(Y_STEP_PIN);  // Y

  if (steppers[0]) {
    steppers[0]->setDirectionPin(X_DIR_PIN);
    steppers[0]->setAutoEnable(true);
    steppers[0]->setAcceleration(500);   // Reasonable acceleration for slow move
    steppers[0]->setSpeedInHz(500);      // Slow, visible motion
  }
  if (steppers[1]) {
    steppers[1]->setDirectionPin(Y_DIR_PIN);
    steppers[1]->setAutoEnable(true);
    steppers[1]->setAcceleration(500);
    steppers[1]->setSpeedInHz(500);
  }

  Wire.begin(21, 22, 400000UL);
  driver.init(Wire);
  driver.resetMotor(0, 0);
  driver.resetMotor(1, 0);
  driver.enableMotor(1);  // enable both motors

  M5.Lcd.println("A: FWD 1rev  C: REV 1rev");
  M5.Lcd.println();
}

void moveBothMotors(int32_t steps) {
  for (int i = 0; i < 2; i++) {
    if (steppers[i]) {
      steppers[i]->move(steps);
      pulseCounts[i] += steps;
    }
  }

  // Wait for both movements to complete
  while (
    (steppers[0] && steppers[0]->isRunning()) ||
    (steppers[1] && steppers[1]->isRunning())) {
    delay(10);
  }
}

void drawStatus() {
  M5.Lcd.fillRect(0, 40, 320, 80, BLACK); // clear only stats area for flicker-free updates
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.printf("X Pulses: %ld\n", pulseCounts[0]);
  M5.Lcd.printf("Y Pulses: %ld\n", pulseCounts[1]);
}

void loop() {
  M5.update();

  if (M5.BtnA.wasClicked()) {
    moveBothMotors(STEPS_PER_REV);   // Forward one revolution
    drawStatus();
  }

  if (M5.BtnC.wasClicked()) {
    moveBothMotors(-STEPS_PER_REV);  // Backward one revolution
    drawStatus();
  }
}
