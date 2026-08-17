#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#define DisplaySerial Serial1
#include "Goldelox_Serial_4DLib.h"
#include "Goldelox_Const4D.h"

#define Xpin        1    
#define Ypin        2   
#define SEL         23  
#define BTN1_PIN    18  
#define BTN2_PIN     5   
#define BTN3_PIN     6   
#define BTN4_PIN     7   
#define LED_STATUS   3  
#define RESETLINE    4

// set our ESPNOW channel
const uint8_t ESPNOW_CHANNEL = 1;

// replace with existiing MAC ADDRESS
uint8_t CAR_MAC[6] = { 0x98, 0xA3, 0x16, 0x7E, 0x70, 0x78 };

// packet struct
typedef struct __attribute__((packed)) {
  int16_t joyX;
  int16_t joyY;
  uint8_t buttons;
  uint32_t seq;
} ControlPacket;

ControlPacket pkt;
uint32_t seqCounter = 0;
// we had to install this library for our uLCD
Goldelox_Serial_4DLib Display(&DisplaySerial);

// joystick center/deadzone with the noise
const int JOY_CENTER_X = 2480;  // 2478–2497
const int JOY_CENTER_Y = 1750;  // 1743–1753
const int JOY_DEAD     = 250;

// this is our center for our screen for our circle that moves to joystcik input
const word CX = 64;
const word CY = 64;

// enum for directions possibleon controller
enum Dir {
  DIR_STOP = 0,
  DIR_UP,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT
};
Dir lastDir = DIR_STOP;

// we are using our on board light to blink to make sure it is sending packages, small way of debugging
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  digitalWrite(LED_STATUS, status == ESP_NOW_SEND_SUCCESS ? HIGH : LOW);
}


// so inorder for me to send the message with the buttons that were pressed and the joystick commands i am going to make a message, m, and then make certain bits one
// corresponding to whcih button/signal is high , similiar to memory map IO
uint8_t readButtonsMask() {
  uint8_t m = 0;
  if (digitalRead(SEL) == LOW)  m |= (1 << 0); // joystick B,1,2,3,4
  if (digitalRead(BTN1_PIN) == HIGH) m |= (1 << 1); 
  if (digitalRead(BTN2_PIN) == HIGH) m |= (1 << 2); 
  if (digitalRead(BTN3_PIN) == HIGH) m |= (1 << 3); 
  if (digitalRead(BTN4_PIN) == HIGH) m |= (1 << 4); 
  return m;
}


void initDisplay() {
  //reset the display
  digitalWrite(RESETLINE, LOW);
  delay(100);
  digitalWrite(RESETLINE, HIGH);
  delay(5000);

  // RX=9, TX=8
  DisplaySerial.begin(9600, SERIAL_8N1, 9, 8);
  Display.TimeLimit4D = 5000;
  Display.gfx_Cls();
}

// draw a circle position because the arrows idea did not work
void drawCircle(Dir d) {
  Display.gfx_Cls();

  word x = CX;
  word y = CY;
  word color = BLUE;
  word r = 6;

  switch (d) {
    case DIR_UP:
      x = CX;
      y = CY / 2; // upper half
      break;
    case DIR_DOWN:
      x = CX;
      y = CY + (CY / 2);// lower half
      break;
    case DIR_LEFT:
      x = CX / 2;// left half
      y = CY;
      break;
    case DIR_RIGHT:
      x = CX + (CX / 2);// right half
      y = CY;
      break;
    case DIR_STOP:
    default:
      x = CX;
      y = CY;// center
      color = RED;// different color for stop
      break;
  }

  Display.gfx_CircleFilled(x, y, r, color);
}

// decide direction from joystick, the deadzone was bringing circle in direction of noise
Dir getDirection(int xRaw, int yRaw) {
  int dx = xRaw - JOY_CENTER_X;
  int dy = yRaw - JOY_CENTER_Y;

  // deadzone
  if (abs(dx) < JOY_DEAD && abs(dy) < JOY_DEAD) {
    return DIR_STOP;
  }

  if (abs(dy) >= abs(dx)) {
    // more changes in y so vertical dominates
    if (dy > JOY_DEAD)  return DIR_UP;     // higher Y = forward
    if (dy < -JOY_DEAD) return DIR_DOWN;
  } else {
    // more changes in the x so horizontal dominates
    if (dx > JOY_DEAD)  return DIR_RIGHT;
    if (dx < -JOY_DEAD) return DIR_LEFT;
  }

  return DIR_STOP;
}


void setup() {
  Serial.begin(115200);
  delay(500);

  //setup pins
  pinMode(SEL, INPUT_PULLUP);
  pinMode(BTN1_PIN, INPUT_PULLDOWN);
  pinMode(BTN2_PIN, INPUT_PULLDOWN);
  pinMode(BTN3_PIN, INPUT_PULLDOWN);
  pinMode(BTN4_PIN, INPUT_PULLDOWN);

  pinMode(LED_STATUS, OUTPUT);
  digitalWrite(LED_STATUS, LOW);

  pinMode(RESETLINE, OUTPUT);
  initDisplay();
// debugs
  Serial.println("wifi_sta");
  WiFi.mode(WIFI_STA);
  Serial.print("esp channel ");
  Serial.println(ESPNOW_CHANNEL);
  esp_err_t chRes = esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
  Serial.print("esp_wifi_set_channel -> ");
  Serial.println((int)chRes);
  Serial.print("CONTROLLER MAC: ");
  Serial.println(WiFi.macAddress());

  // init ESPNOW with debug
  esp_err_t initRes = esp_now_init();
  Serial.print("esp_now_init -> ");
  Serial.println((int)initRes);
  if (initRes != ESP_OK) {
    Serial.println("esp_now_init FAILED");
    while (true) {
      digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
      delay(500);
    }
  }

  esp_now_register_send_cb(onDataSent);

  //adding peer to register the CAR so the controller can send packets specifically to that MAC address
  //ESPNOW cannot send data added to the peer list
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, CAR_MAC, 6);
  peer.channel = ESPNOW_CHANNEL;
  peer.encrypt = false;

  esp_err_t addRes = esp_now_add_peer(&peer);
  Serial.print("esp_now_add_peer -> ");
  Serial.println((int)addRes);
  if (addRes != ESP_OK) {
    Serial.println("peer add FAILED");
    while (true) {
      digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
      delay(500);
    }
  }
  // we are just always going to start with stop
  drawCircle(DIR_STOP);
  Serial.println("CONTROLLER READY");
}

void loop() {
  // read joystick (0..4095)
  int x = analogRead(Xpin);
  int y = analogRead(Ypin);

  // invert X because left was right
  x = 4095 - x;

  // read buttons
  uint8_t buttons = readButtonsMask();

  // determine direction and update display if changed
  Dir d = getDirection(x, y);
  if (d != lastDir) {
    lastDir = d;
    drawCircle(d);
  }

  // fill packet with the info 
  pkt.buttons = buttons;
  pkt.seq     = seqCounter++;
  pkt.joyX    = (int16_t)x;
  pkt.joyY    = (int16_t)y;

  // send packet
  esp_err_t r = esp_now_send(CAR_MAC, (uint8_t*)&pkt, sizeof(pkt));

  //debugs because wireless can be such a pain man
  Serial.print("X=");
  Serial.print(pkt.joyX);
  Serial.print("  Y=");
  Serial.print(pkt.joyY);
  Serial.print("  dir=");
  Serial.print((int)d);
  Serial.print("  buttons=0x");
  Serial.print(pkt.buttons, HEX);
  Serial.print("  esp_now_send=");
  Serial.println(r == ESP_OK ? "OK" : "ERR");

  delay(20);
}
