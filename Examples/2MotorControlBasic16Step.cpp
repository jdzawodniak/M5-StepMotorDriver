/*
*******************************************************************************
* Author: John Zawodniak
* Date: 2024-02-01
* Version: 2.0
* Updated for 1/16 microstepping on M5Stack Core (Basic) and Module 13.2.
* Now requires 3200 steps per revolution (200 full steps * 16 microsteps).
*******************************************************************************
*/

#include <Arduino.h>
#include <FastAccelStepper.h>
#include <M5Unified.h>
#include <Module_Stepmotor.h>

// Pin configuration for M5Stack Core (Basic) + Module 13.2
#define X_DIR_PIN 17
#define X_STEP_PIN 16
#define Y_DIR_PIN 13
#define Y_STEP_PIN 12

#define FULL_STEP_PER_REV 200    // Full steps per revolution
#define MICRO_STEPS 16           // 1/16 microstepping enabled by hardware jumper M2
#define STEPS_PER_REV (FULL_STEP_PER_REV * MICRO_STEPS)   // 3200 steps per revolution

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
  M5.Lcd.println("Stepper Ready (1/16 Step)");

  // Stepper driver initialization  
  engine.init();

  // Connect Step pins and configure direction pins for each stepper
  steppers[0] = engine.stepperConnectToPin(X_STEP_PIN);  // X
  steppers[1] = engine.stepperConnectToPin(Y_STEP_PIN);  // Y

  if (steppers[0]) {
    steppers[0]->setDirectionPin(X_DIR_PIN);
    steppers[0]->setAutoEnable(true);
    steppers[0]->setAcceleration(500);   // Reasonable acceleration for smooth moves
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
    moveBothMotors(STEPS_PER_REV);   // Forward one revolution (3200 steps)
    drawStatus();
  }

  if (M5.BtnC.wasClicked()) {
    moveBothMotors(-STEPS_PER_REV);  // Backward one revolution (-3200 steps)
    drawStatus();
  }
}
