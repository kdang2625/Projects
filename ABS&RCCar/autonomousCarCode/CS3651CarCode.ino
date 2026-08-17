#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include "Adafruit_VL6180X.h"

Adafruit_VL6180X tof = Adafruit_VL6180X();

#ifndef LED_BUILTIN
#define LED_BUILTIN 8
#endif

const uint8_t ESPNOW_CHANNEL = 1;

// define pins for motor driver
#define STBY_PIN   18
#define AIN1_PIN   19
#define AIN2_PIN   20
#define PWMA_PIN   21
#define BIN1_PIN   22
#define BIN2_PIN   23
#define PWMB_PIN    1

#define HONK_PIN   6
#define BRAKE_PIN 11

// make my struct for the control packet
// using ESP-NOW packets
typedef struct __attribute__((packed)) {
  int16_t joyX;
  int16_t joyY;
  uint8_t buttons;
  uint32_t seq;
} ControlPacket;

// create a packet that is last one recieved, need volatile for newest val in mem
volatile ControlPacket lastPacket;
volatile bool gotPacket = false;

// global variables for autonomous mode
  bool autoMode = false;
  bool turnLeftNext = false;
// enum for our states
enum AutoState {
  AUTO_FWD = 0,
  AUTO_BACK,
  AUTO_TURN
};

// just go ahead and define our starting state
AutoState autoState = AUTO_FWD;
uint32_t autoStateStartMs = 0;
//creating a scale variable for our auto state , through trial and errror we landed on this being optimal speed
const float AUTO_SPEED_SCALE = 0.40f;

// base speeds per state 
const int AUTO_FWD_BASE   = 180;
const int AUTO_BACK_BASE  = -150;
const int AUTO_TURN_L_BASE = 150;
const int AUTO_TURN_R_BASE = -150;

// show that full packet was received and make sure that the packet is of correct length so we do not get ourselves into an errorif trying to find somethingnot there
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  if (len == sizeof(ControlPacket)) {
    memcpy((void*)&lastPacket, incomingData, sizeof(ControlPacket));
    gotPacket = true;
  }
}

// need a helper to set a motor, the motor can be defined through the params
//set the base line for the motors so it is not driving with no inputs
void setMotor(int pwmSigned, uint8_t IN1, uint8_t IN2, uint8_t PWMPin) {
  int duty = abs(pwmSigned);
  if (duty > 255) duty = 255;

  if (pwmSigned > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else if (pwmSigned < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }

  analogWrite(PWMPin, duty);
}

// initializing motors
void setupMotors() {
  pinMode(STBY_PIN, OUTPUT);
  digitalWrite(STBY_PIN, HIGH);

  pinMode(AIN1_PIN, OUTPUT);
  pinMode(AIN2_PIN, OUTPUT);
  pinMode(PWMA_PIN, OUTPUT);

  pinMode(BIN1_PIN, OUTPUT);
  pinMode(BIN2_PIN, OUTPUT);
  pinMode(PWMB_PIN, OUTPUT);

  setMotor(0, AIN1_PIN, AIN2_PIN, PWMA_PIN);
  setMotor(0, BIN1_PIN, BIN2_PIN, PWMB_PIN);
}

// calibrate the joystick to handle the deadzones because the motors are running with
// no inputs, make sure to handle the turns and set threshold for the joystick noise
void joystickToMotors(int16_t joyX, int16_t joyY, int &leftOut, int &rightOut) {
  static bool calibrated = false;
  // these values are not always mid
  static int16_t midX = 2048;
  static int16_t midY = 2048;
// check if it was calibrated if not then calibrate it
  if (!calibrated) {
    midX = joyX;
    midY = joyY;
    calibrated = true;
    // need debug because cant really say if it happened or not
    Serial.printf("Calibrated, midX=%d midY=%d\n", midX, midY);
  }

  int32_t centeredY = joyY - midY;
  int32_t centeredX = joyX - midX;

// set this up so that after the joystick movement was pass the thereshold then the motors would move, this is from how like finiky or i guess sensitive the joystick ended up being
  const int DEADZONE = 200;
  if (abs(centeredY) < DEADZONE) centeredY = 0;
  if (abs(centeredX) < DEADZONE) centeredX = 0;
  
  // map(input, lower, upper, toLower, toUpper)
  long baseSpeed = map(centeredY, -2048, 2047, -255, 255);
  long turn = map(centeredX, -2048, 2047, -255, 255);
  long left  = baseSpeed + turn;
  long right = baseSpeed - turn;

  // we need to contrain our left and right  to a certain range, figured this would help protect our stuff before we end up breaking anything
  left  = constrain(left,  -255, 255);
  right = constrain(right, -255, 255);
  
  // deadzone handling
  const int ZERO_CLAMP = 40;
  if (abs(left) < ZERO_CLAMP) left = 0;
  if (abs(right) < ZERO_CLAMP) right = 0;

  leftOut = left;
  rightOut = right;
}

// helper for tof sensor to read dist
int readDistanceMM() {
  int dist = tof.readRange();
  return dist;
}

// honk helper .. we had to add delay a small amount because initially we just had it going on then off haha
void honk() {
  tone(HONK_PIN, 900);
  delay(120);
  noTone(HONK_PIN);
}

void motorsEnable() {
  digitalWrite(STBY_PIN, HIGH);
}

// going to make helper for when we get our handbrake 
void motorsBrake() {
  motorsEnable();

  analogWrite(PWMA_PIN, 0);
  analogWrite(PWMB_PIN, 0);

  digitalWrite(BRAKE_PIN, HIGH);
  delay(10);
  digitalWrite(BRAKE_PIN, LOW);
}

// idea for autonomous behavior, toggle autonomous mode, poll the TOF, go fwd, if something is detected,
// backup a certain amount and turn left or right
void runAutonomous(uint32_t now) {
  motorsEnable();

  // read distance
  int dist = readDistanceMM();
  Serial.printf("ToF dist = %d mm\n", dist);

  int leftSpeed = 0;
  int rightSpeed = 0;
// going to use switch and case statements, seems best way to handle this
  switch (autoState) {
    // oksy the way this is going to work... go forward hit something, back it up, turn left/right then continue to moving forward agsin
    case AUTO_FWD:
      // drive forward (scaled)
      leftSpeed  = (int)(AUTO_FWD_BASE * AUTO_SPEED_SCALE);
      rightSpeed = (int)(AUTO_FWD_BASE * AUTO_SPEED_SCALE);

      // dbstacle detected ==  back up
      if (dist < 100) {
        Serial.println("Obstacle ahead, switching to BACK");
        autoState = AUTO_BACK;
        autoStateStartMs = now;
      }
      break;
    
    case AUTO_BACK:
      leftSpeed  = (int)(AUTO_BACK_BASE * AUTO_SPEED_SCALE);
      rightSpeed = (int)(AUTO_BACK_BASE * AUTO_SPEED_SCALE);

      if (now - autoStateStartMs > 800) {
        Serial.println("Done backing up, switching turn");
        autoState = AUTO_TURN;
        autoStateStartMs = now;
      }
      break;

    case AUTO_TURN:
      // need to implent alter turn directions so that it can go back on course
      if (turnLeftNext) {
        Serial.println("TURNING LEFT");
        leftSpeed  = (int)(AUTO_TURN_L_BASE * AUTO_SPEED_SCALE);
        rightSpeed = (int)(AUTO_TURN_R_BASE * AUTO_SPEED_SCALE);
      } else {
        Serial.println("TURNING RIGHT");
        leftSpeed  = (int)(AUTO_TURN_R_BASE * AUTO_SPEED_SCALE);
        rightSpeed = (int)(AUTO_TURN_L_BASE * AUTO_SPEED_SCALE);
      }

      // flip directions
      if (now - autoStateStartMs > 400) {
        turnLeftNext = !turnLeftNext;
        Serial.println("Done turning, switching to FWD");
        autoState = AUTO_FWD;
        autoStateStartMs = now;
      }
      break;

  }

  setMotor(leftSpeed,  AIN1_PIN, AIN2_PIN, PWMA_PIN);
  setMotor(rightSpeed, BIN1_PIN, BIN2_PIN, PWMB_PIN);
}



void setup() {
  Serial.begin(115200);
  delay(500);
 
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(HONK_PIN, OUTPUT);
  pinMode(BRAKE_PIN, OUTPUT);

  setupMotors();

  Serial.println("Init ToF");
  Wire.begin(5, 4);

  if (!tof.begin()) {
    Serial.println("Error ToF not detected");
  } else {
    Serial.println("ToF found");
  }

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW fail");
    while (1);
  }

  esp_now_register_recv_cb(onDataRecv);

  Serial.println("Setup done");
}

uint32_t lastBlink = 0;

void loop() {
  uint32_t now = millis();

  // start blinking so we know is connected
  if (now - lastBlink >= 500) {
    lastBlink = now;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  
  // check for packets and then handle them accordingly

  if (gotPacket) {
    ControlPacket pkt;

    noInterrupts();
    memcpy((void *)&pkt, (const void *)&lastPacket, sizeof(ControlPacket));
    gotPacket = false;
    interrupts();


    // horn bit1 = 0x02
    if (pkt.buttons & 0x02) {
      honk();
    }

    // handbrake bit2 = 0x04 overrides everything , even auto mode
    bool handbrake = (pkt.buttons & 0x04) != 0;
    if (handbrake) {
      Serial.println("Handbrake ACTIVE");
      motorsBrake();
      return;
    }

    // autonomous bit3 = 0x08
    static uint8_t prevButtons = 0;
    bool b3Now  = (pkt.buttons & 0x08) != 0;
    bool b3Prev = (prevButtons & 0x08) != 0;
  // need to compare inorder to see if button pressed, then make the automode the oppositie of what it was
    if (b3Now && !b3Prev) {
      autoMode = !autoMode;
      if (autoMode) {
        Serial.println("AUTONOMOUS MODE: ON");
        autoState = AUTO_FWD;
        autoStateStartMs = now;
      } else {
        Serial.println("AUTONOMOUS MODE: OFF");
        motorsBrake();
      }
    }
    prevButtons = pkt.buttons;

    //run auto but cant let it hit the maunal joystick handling
    if (autoMode) {
      runAutonomous(now);
      return;
    }

    // since auto not hit we let joystick movements work

    int leftSpeed = 0;
    int rightSpeed = 0;

    joystickToMotors(pkt.joyX, pkt.joyY, leftSpeed, rightSpeed);

    int dist = readDistanceMM();
    Serial.printf("ToF dist = %d mm\n", dist);

    if (dist < 100) {
      if (leftSpeed > 0)  leftSpeed = 0;
      if (rightSpeed > 0) rightSpeed = 0;
      Serial.println("Detected obstacle");
    }

    setMotor(leftSpeed,  AIN1_PIN, AIN2_PIN, PWMA_PIN);
    setMotor(rightSpeed, BIN1_PIN, BIN2_PIN, PWMB_PIN);
  }
}
