/*
  Gesture Control Transmitter (Complete Version)
  - Uses MPU6050 to detect tilt
  - Sends specific mapped data to Receiver
  - Library Requirements: RF24, I2Cdev, MPU6050
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>

// --- MPU6050 Setup ---
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

// --- Data Packet ---
// data[0] = X Axis (Forward/Back)
// data[1] = Y Axis (Left/Right)
int data[2];

// --- NRF24L01 Setup ---
// CE = 8, CSN = 9 (As per your working snippet)
RF24 radio(8, 9); 

// Address (Must match the Receiver exactly)
const uint64_t pipe = 0xE8E8F0F0E1LL;

void setup(void) {
  Serial.begin(9600);
  Wire.begin();
  
  // Initialize MPU6050
  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  
  // Verify MPU connection
  if(mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
  }

  // Initialize NRF24L01
  radio.begin();
  radio.openWritingPipe(pipe);
  radio.setPALevel(RF24_PA_MIN); // Low power mode (better for battery life/stability)
  radio.stopListening();         // Put radio in Transmitter mode
  
  Serial.println("Transmitter Ready.");
}

void loop(void) {
  // 1. Read Raw Data from MPU6050
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // 2. Map and Prepare Data
  // X Axis: Maps raw accel (-17000 to 17000) to range (300 to 400)
  // Center is approx 350. >365 is Forward, <335 is Backward.
  data[0] = map(ax, -17000, 17000, 300, 400); 

  // Y Axis: Maps raw accel (-17000 to 17000) to range (100 to 200)
  // Center is approx 150. >165 is Right, <135 is Left.
  data[1] = map(ay, -17000, 17000, 100, 200);

  // 3. Send Data via NRF
  bool report = radio.write(data, sizeof(data));

  // 4. Debugging (View in Serial Monitor)
  // Only prints if transmission was successful to avoid spamming errors
  if (report) {
    Serial.print("Sent -> X: ");
    Serial.print(data[0]);
    Serial.print(" | Y: ");
    Serial.println(data[1]);
  } else {
    Serial.println("Transmission Failed (Check NRF wiring)");
  }
  
  // Small delay to prevent flooding the radio
  delay(20); 
}