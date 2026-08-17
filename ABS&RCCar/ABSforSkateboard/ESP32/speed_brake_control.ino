#include <Arduino.h>
#include <BluetoothSerial.h>
#include <ESP32Servo.h>

// Define motor control pins
#define MOTOR_PWM 21
#define MOTOR_DIR1 5
#define MOTOR_DIR2 6

// Define servo control pin
#define SERVO_PIN 22

// Define constants for acceleration, braking, and incline adjustment
const float maxSpeed = 255.0;  // Max PWM value
const float minSpeed = 0.0;
const float accelerationStep = 10.0; // Gradual acceleration step
const float decelerationStep = 15.0; // Faster braking
const float gravityCompensation = 5.0; // Compensation factor for incline

// Bluetooth setup
BluetoothSerial SerialBT;

// Servo setup
Servo steeringServo;

// Speed control variables
float motorSpeed = 0.0;
float targetSpeed = 0.0;
float servoPosition = 90;  // Neutral position (90 degrees)
float inclineAngle = 0.0;  // Simulated incline angle

// Function to adjust speed based on incline
float adjustSpeedForIncline(float baseSpeed, float angle) {
    return baseSpeed + gravityCompensation * sin(angle * PI / 180.0);
}

// Function to gradually adjust motor speed
void updateMotorSpeed() {
    if (motorSpeed < targetSpeed) {
        motorSpeed += accelerationStep;
        if (motorSpeed > targetSpeed) motorSpeed = targetSpeed;
    } else if (motorSpeed > targetSpeed) {
        motorSpeed -= decelerationStep;
        if (motorSpeed < targetSpeed) motorSpeed = targetSpeed;
    }

    // Apply PWM speed and direction
    if (motorSpeed > 0) {
        digitalWrite(MOTOR_DIR1, HIGH);
        digitalWrite(MOTOR_DIR2, LOW);
    } else if (motorSpeed < 0) {
        digitalWrite(MOTOR_DIR1, LOW);
        digitalWrite(MOTOR_DIR2, HIGH);
    } else {
        digitalWrite(MOTOR_DIR1, LOW);
        digitalWrite(MOTOR_DIR2, LOW);
    }

    analogWrite(MOTOR_PWM, abs(motorSpeed));
}

void setup() {
    Serial.begin(115200);
    SerialBT.begin("ESP32_EV_Controller"); // Bluetooth device name

    // Configure motor pins
    pinMode(MOTOR_PWM, OUTPUT);
    pinMode(MOTOR_DIR1, OUTPUT);
    pinMode(MOTOR_DIR2, OUTPUT);
    
    // Attach servo
    steeringServo.attach(SERVO_PIN);
    steeringServo.write(servoPosition);
}

void loop() {
    if (SerialBT.available()) {
        char bnum = SerialBT.read(); // Read button number
        char bhit = SerialBT.read(); // Read button state

        switch (bnum) {
            case '5':  // Accelerate
                if (bhit == '1') {
                    targetSpeed = adjustSpeedForIncline(motorSpeed + accelerationStep, inclineAngle);
                    if (targetSpeed > maxSpeed) targetSpeed = maxSpeed;
                }
                break;

            case '6':  // Brake
                if (bhit == '1') {
                    targetSpeed = minSpeed;
                }
                break;

            case '7':  // Turn left
                if (bhit == '1') {
                    servoPosition -= 10;
                    if (servoPosition < 45) servoPosition = 45; // Limit turn angle
                    steeringServo.write(servoPosition);
                }
                break;

            case '8':  // Turn right
                if (bhit == '1') {
                    servoPosition += 10;
                    if (servoPosition > 135) servoPosition = 135; // Limit turn angle
                    steeringServo.write(servoPosition);
                }
                break;

            case '9':  // Gradual slowdown
                if (bhit == '1') {
                    targetSpeed = adjustSpeedForIncline(motorSpeed - decelerationStep, inclineAngle);
                    if (targetSpeed < minSpeed) targetSpeed = minSpeed;
                }
                break;

            default:
                targetSpeed = 0.0;
                break;
        }
    }

    updateMotorSpeed();
    delay(50); // Small delay for stability
}
