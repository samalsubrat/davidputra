/*
  Receiver Code - Sliding Turn Signals (Audi Style)
  
  --- Wiring ---
  1. NRF24L01: CE->8, CSN->9, SCK->13, MOSI->11, MISO->12
  2. Bluetooth (HC-05): TX->A4, RX->10
  3. Motors (TB6612FNG): AIN1->2, AIN2->4, PWMA->3, BIN1->7, BIN2->6, PWMB->5
  4. Motor Standby (STBY): A5
  5. NeoPixel LED (8 LEDs): Data Pin -> A2
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

// --- NeoPixel Setup ---
#define LED_PIN    A2
#define LED_COUNT  8
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- NRF Setup ---
RF24 radio(8, 9); 
const byte address[6] = "00001"; 

// --- Bluetooth Setup ---
SoftwareSerial btSerial(A4, 10); 

// --- Motor Pins ---
const int AIN1 = 2, AIN2 = 4, PWMA = 3;
const int BIN1 = 7, BIN2 = 6, PWMB = 5; 
const int STBY = A5; 

int motorSpeed = 80; 

// --- Robot States ---
// We need to track state to handle animation without blocking
enum State { STOP, FORWARD, BACKWARD, LEFT, RIGHT };
State currentState = STOP;

// --- Animation Variables ---
unsigned long lastAnimationTime = 0;
int animationStep = 0;
const int animationSpeed = 200; // Lower number = Faster slide (milliseconds)

// --- Color Definitions ---
uint32_t colorWhite = strip.Color(255, 255, 255);
uint32_t colorYellow = strip.Color(255, 200, 0);
uint32_t colorRed = strip.Color(255, 0, 0);
uint32_t colorOff = strip.Color(0, 0, 0);

void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);

  // Initialize Lights
  strip.begin();
  strip.setBrightness(150); // 0-255
  strip.show(); 
  
  // Motor Pins
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT); pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT); pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH); 

  if (radio.begin()) {
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
    Serial.println("NRF24 Ready.");
  }
}

void loop() {
  // 1. READ INPUTS
  if (radio.available()) {
    char command = 0;
    radio.read(&command, sizeof(command));
    processCommand(command);
  }
  if (btSerial.available()) {
    char command = btSerial.read();
    processCommand(command);
  }

  // 2. UPDATE OUTPUTS (Motors & Lights)
  updatePhysicalRobot();
}

void processCommand(char cmd) {
  // Update the target state based on input
  switch (cmd) {
    case 'w': currentState = FORWARD; break;
    case 's': currentState = BACKWARD; break;
    case 'a': currentState = LEFT; break;
    case 'd': currentState = RIGHT; break;
    case 'x': currentState = STOP; break;
    
    // Speed settings
    case '1': motorSpeed = 60; break; 
    case '2': motorSpeed = 100; break; 
    case '3': motorSpeed = 255; break; 
  }
}

void updatePhysicalRobot() {
  unsigned long currentMillis = millis();

  switch (currentState) {
    
    case FORWARD:
      // Motor
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); analogWrite(PWMA, motorSpeed);
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); analogWrite(PWMB, motorSpeed);
      // Light (Static White)
      strip.fill(colorWhite);
      strip.show();
      break;

    case BACKWARD:
      // Motor
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); analogWrite(PWMA, motorSpeed);
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); analogWrite(PWMB, motorSpeed);
      // Light (Static Yellow)
      strip.fill(colorYellow);
      strip.show();
      break;

    case LEFT:
      // Motor (Spin Left)
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); analogWrite(PWMA, motorSpeed);
      digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); analogWrite(PWMB, motorSpeed);
      
      // Light (Sliding Red Animation on Left Side: 2 -> 1 -> 0)
      if (currentMillis - lastAnimationTime > animationSpeed) {
        lastAnimationTime = currentMillis;
        animationStep++;
        if (animationStep > 3) { // Reset animation
           animationStep = 0;
           strip.clear(); 
        }
        
        // Logic to build the bar (3 LEDs: indices 2, 1, 0)
        // Step 1: Pixel 2 ON
        // Step 2: Pixel 2, 1 ON
        // Step 3: Pixel 2, 1, 0 ON
        if (animationStep > 0) strip.setPixelColor(2, colorRed); 
        if (animationStep > 1) strip.setPixelColor(1, colorRed);
        if (animationStep > 2) strip.setPixelColor(0, colorRed);
        
        // Ensure right side is off
        strip.setPixelColor(5, 0); strip.setPixelColor(6, 0); strip.setPixelColor(7, 0);
        strip.show();
      }
      break;

    case RIGHT:
      // Motor (Spin Right)
      digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); analogWrite(PWMA, motorSpeed);
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); analogWrite(PWMB, motorSpeed);
      
      // Light (Sliding Red Animation on Right Side: 5 -> 6 -> 7)
      if (currentMillis - lastAnimationTime > animationSpeed) {
        lastAnimationTime = currentMillis;
        animationStep++;
        if (animationStep > 3) { 
           animationStep = 0;
           strip.clear(); 
        }
        
        // Logic to build the bar (3 LEDs: indices 5, 6, 7)
        if (animationStep > 0) strip.setPixelColor(5, colorRed);
        if (animationStep > 1) strip.setPixelColor(6, colorRed);
        if (animationStep > 2) strip.setPixelColor(7, colorRed);

        // Ensure left side is off
        strip.setPixelColor(0, 0); strip.setPixelColor(1, 0); strip.setPixelColor(2, 0);
        strip.show();
      }
      break;

    case STOP:
      // Motor
      digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW); analogWrite(PWMA, 0);
      digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW); analogWrite(PWMB, 0);
      // Light (Off)
      strip.clear();
      strip.show();
      break;
  }
}