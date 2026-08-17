#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include "ICM_20948.h"

// BLE definitions
#define DEVICE_NAME         "SkateABS"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// I2C and accelerometer definitions
#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_SPEED 100000

// Moving average filter settings
#define WINDOW_SIZE 10

BLECharacteristic* pCharacteristic;
BLEAdvertising* pAdvertising = nullptr;

// ====== Callback for characteristic writes ======
class MyBLECallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) override {
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.print("BLE Received: ");
      Serial.println(value);
    }
  }
};

// ====== Callback for client connect/disconnect ======
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    Serial.println("Client connected.");
  }
  void onDisconnect(BLEServer* pServer) override {
    Serial.println("Client disconnected.");
    delay(1500); // Give some time before restarting advertising
    if (pAdvertising) {
      pAdvertising->start();
      Serial.println("Advertising restarted.");
    }
  }
};

ICM_20948_I2C myICM;

// Arrays for moving average filter
float accX_buffer[WINDOW_SIZE] = {0};
float accY_buffer[WINDOW_SIZE] = {0};
float accZ_buffer[WINDOW_SIZE] = {0};
int bufferIndex = 0;
bool bufferFilled = false;

float getAverage(float* buffer) {
  float sum = 0;
  int count = bufferFilled ? WINDOW_SIZE : bufferIndex;
  for (int i = 0; i < count; i++) {
    sum += buffer[i];
  }
  return sum / count;
}

// Calibration globals
float calX = 0, calY = 0, calZ = 0;
bool calibrated = false;

// High-pass filter globals
float hpAccX = 0, hpAccY = 0, hpAccZ = 0;
float lastDynX = 0, lastDynY = 0, lastDynZ = 0;
const float hpAlpha = 0.9; // Adjust: closer to 1 means less filtering

// Calibration routine: averages a number of samples to determine gravity offsets
void calibrateSensor() {
  const int calSamples = 100;
  float sumX = 0, sumY = 0, sumZ = 0;
  Serial.println("Calibrating sensor... Keep it still.");
  for (int i = 0; i < calSamples; i++) {
    while (!myICM.dataReady()) { }
    myICM.getAGMT();
    sumX += myICM.accX();
    sumY += myICM.accY();
    sumZ += myICM.accZ();
    delay(10);
  }
  calX = sumX / calSamples;
  calY = sumY / calSamples;
  calZ = sumZ / calSamples;
  calibrated = true;
  Serial.print("Calibration offsets: ");
  Serial.print(calX, 5); Serial.print(", ");
  Serial.print(calY, 5); Serial.print(", ");
  Serial.println(calZ, 5);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { }

  // Initialize BLE
  BLEDevice::init(DEVICE_NAME);
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  
  pCharacteristic->setCallbacks(new MyBLECallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("BLE Advertising started. Device is now discoverable as 'SkateABS'.");

  // Initialize I2C and the accelerometer
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(I2C_SPEED);
  Serial.println("Initializing IMU...");

  if (myICM.begin(Wire, 0x69) != ICM_20948_Stat_Ok) {
    Serial.println("IMU not detected at 0x69, trying 0x68...");
    if (myICM.begin(Wire, 0x68) != ICM_20948_Stat_Ok) {
      Serial.println("IMU not detected at 0x68 either. Check wiring.");
    } else {
      Serial.println("IMU initialization successful at 0x68!");
    }
  } else {
    Serial.println("IMU initialization successful at 0x69!");
  }
}

void loop() {
  // Handle Serial input for BLE notifications
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) {
      Serial.print("Serial Received: ");
      Serial.println(input);
      pCharacteristic->setValue(input.c_str());
      pCharacteristic->notify();
      Serial.println("BLE Notification Sent for Serial input.");
    }
  }

  // Read accelerometer data when available
  if (myICM.dataReady()) {
    myICM.getAGMT();
    
    // Get raw accelerometer values
    float rawX = myICM.accX();
    float rawY = myICM.accY();
    float rawZ = myICM.accZ();
    
    // Calibrate if not yet done (sensor must be still during calibration)
    if (!calibrated) {
      calibrateSensor();
    }
    
    // Subtract calibration offsets to remove gravity (dynamic acceleration)
    float dynX = rawX - calX;
    float dynY = rawY - calY;
    float dynZ = rawZ - calZ;
    
    // Apply high-pass filter to remove remaining low-frequency (gravity) component
    hpAccX = hpAlpha * (hpAccX + dynX - lastDynX);
    hpAccY = hpAlpha * (hpAccY + dynY - lastDynY);
    hpAccZ = hpAlpha * (hpAccZ + dynZ - lastDynZ);
    
    // Update the previous dynamic values
    lastDynX = dynX;
    lastDynY = dynY;
    lastDynZ = dynZ;
    
    // Now, add the high-pass filtered (dynamic) values to the moving average buffers
    accX_buffer[bufferIndex] = hpAccX;
    accY_buffer[bufferIndex] = hpAccY;
    accZ_buffer[bufferIndex] = hpAccZ;

    bufferIndex++;
    if (bufferIndex >= WINDOW_SIZE) {
      bufferIndex = 0;
      bufferFilled = true;
    }
    
    // Calculate moving average on the filtered values
    float avgAccX = getAverage(accX_buffer);
    float avgAccY = getAverage(accY_buffer);
    float avgAccZ = getAverage(accZ_buffer);
    
    // Format the dynamic acceleration data as a string
    String sensorData = "";
    sensorData += "Dynamic Acc (mg): [";
    sensorData += String(avgAccX, 5) + ", ";
    sensorData += String(avgAccY, 5) + ", ";
    sensorData += String(avgAccZ, 5) + "]";
    
    Serial.println(sensorData);
    
    // Send the data over BLE
    pCharacteristic->setValue(sensorData.c_str());
    pCharacteristic->notify();
    
    delay(700); // Adjust delay as needed
  } else {
    Serial.println("Waiting for sensor data...");
    delay(700);
  }
}
