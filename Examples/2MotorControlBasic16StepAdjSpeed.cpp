/*
*******************************************************************************
* Author: John Zawodniak
* Date: 2024-02-01
* Version: 2.1
* Updated for 1/16 microstepping and speed control cycling via Button B
*******************************************************************************
*/

#include <Arduino.h>
#include <FastAccelStepper.h>
#include <M5Unified.h>
#include <Module_Stepmotor.h>

// Function prototypes
void drawStatus();
void drawInstructions();
void moveBothMotors(int32_t steps);
void updateSpeed();

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

// Speed control variables
const int speedLevels[] = {0, 100, 200, 300, 400, 500}; // Hz corresponding to 0,20,...100%
const int speedPercentages[] = {0, 20, 40, 60, 80, 100};
const int speedLevelsCount = sizeof(speedLevels) / sizeof(speedLevels[0]);
int currentSpeedIndex = 0;

void drawInstructions() {
  M5.Lcd.fillRect(0, 100, 320, 40, BLACK); // Clear area for instructions and speed display
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.printf("Press B to change speed\n");
  M5.Lcd.printf("Speed: %d%%\n", speedPercentages[currentSpeedIndex]);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);

  M5.Lcd.setTextSize(2);
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Stepper Ready (1/16 Step)");
  M5.Lcd.println("A: FWD 1rev  C: REV 1rev");

  // Stepper driver initialization  
  engine.init();

  // Connect Step pins and configure direction pins for each stepper
  steppers[0] = engine.stepperConnectToPin(X_STEP_PIN);  // X
  steppers[1] = engine.stepperConnectToPin(Y_STEP_PIN);  // Y

  for (int i = 0; i < 2; i++) {
    if (steppers[i]) {
      int dirPin = (i == 0) ? X_DIR_PIN : Y_DIR_PIN;
      steppers[i]->setDirectionPin(dirPin);
      steppers[i]->setAutoEnable(true);
      steppers[i]->setAcceleration(500);   // Reasonable acceleration for smooth moves
      steppers[i]->setSpeedInHz(speedLevels[currentSpeedIndex]);  // Start at speed 0
    }
  }

  Wire.begin(21, 22, 400000UL);
  driver.init(Wire);
  driver.resetMotor(0, 0);
  driver.resetMotor(1, 0);
  driver.enableMotor(1);  // enable both motors

  drawInstructions();
  drawStatus();
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
    (steppers[1] && steppers[1]->isRunning())
  ) {
    delay(10);
  }
}

void drawStatus() {
  M5.Lcd.fillRect(0, 40, 320, 60, BLACK); // clear only stats area for flicker-free updates
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.printf("X Pulses: %ld\n", pulseCounts[0]);
  M5.Lcd.printf("Y Pulses: %ld\n", pulseCounts[1]);
}

void updateSpeed() {
  currentSpeedIndex++;
  if (currentSpeedIndex >= speedLevelsCount) {
    currentSpeedIndex = 0; // Wrap around to 0%
  }
  // Update speed for all motors
  for (int i = 0; i < 2; i++) {
    if (steppers[i]) {
      steppers[i]->setSpeedInHz(speedLevels[currentSpeedIndex]);
    }
  }
  drawInstructions();
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

  if (M5.BtnB.wasClicked()) {
    updateSpeed();
  }
}
