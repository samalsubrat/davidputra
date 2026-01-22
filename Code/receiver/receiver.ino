#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

// ================= NRF24 =================
RF24 radio(9, 10);  // CE, CSN
const byte address[6] = "00001";

// ================= Bluetooth =================
SoftwareSerial bt(A3, A2);  // RX, TX (Arduino side)

// ================= NeoPixel =================
#define LED_PIN A4
#define LED_COUNT 16
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// ================= TB6612 =================
#define AIN1 2
#define AIN2 4
#define PWMA 3
#define BIN1 7
#define BIN2 6
#define PWMB 5
#define STBY 8

// ================= HC-SR04 =================
#define TRIG_PIN A0
#define ECHO_PIN A1

// ================= Config =================
int speedVal = 130;

unsigned long lastPacketTime = 0;
const unsigned long NRF_TIMEOUT = 1500;

const int OBSTACLE_DISTANCE = 20;  // cm
unsigned long lastUltrasonicTime = 0;
const unsigned long ULTRASONIC_INTERVAL = 60;

bool obstacleDetected = false;
char lastMoveCmd = 'x';

// ================= Robot State =================
enum State { STOP,
             FORWARD,
             BACKWARD,
             LEFT,
             RIGHT };
State currentState = STOP;

// ================= NeoPixel Animation =================
unsigned long lastAnimationTime = 0;
int animationStep = 0;
const int animationSpeed = 200;

uint32_t colorWhite;
uint32_t colorYellow;
uint32_t colorOrange;

uint8_t rgbHue = 0;
unsigned long lastRGBTime = 0;
const unsigned long RGB_INTERVAL = 20;  // lower = smoother

uint32_t wheel(byte pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return strip.Color(255 - pos * 3, 0, pos * 3);
  }
  if (pos < 170) {
    pos -= 85;
    return strip.Color(0, pos * 3, 255 - pos * 3);
  }
  pos -= 170;
  return strip.Color(pos * 3, 255 - pos * 3, 0);
}


// ================= Setup =================
void setup() {
  Serial.begin(9600);
  bt.begin(9600);

  // NeoPixel init
  strip.begin();
  strip.setBrightness(150);
  strip.show();

  colorWhite = strip.Color(255, 255, 255);
  colorYellow = strip.Color(255, 200, 0);
  colorOrange = strip.Color(255, 92, 0);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(STBY, HIGH);
  stopMotors();

  initNRF();
}

// ================= NRF =================
void initNRF() {
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, address);
  radio.startListening();
}

void recoverNRF() {
  radio.stopListening();
  radio.powerDown();
  delay(5);
  radio.powerUp();
  delay(5);
  initNRF();
}

// ================= Ultrasonic =================
long getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  if (duration == 0) return 999;
  return duration / 58;
}

// ================= Motor Control =================
void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  currentState = STOP;
}

void left() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMA, speedVal);
  analogWrite(PWMB, speedVal);
  currentState = LEFT;
}

void right() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMA, speedVal);
  analogWrite(PWMB, speedVal);
  currentState = RIGHT;
}

void forward() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMA, speedVal);
  analogWrite(PWMB, speedVal);
  currentState = FORWARD;
}

void backward() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMA, speedVal);
  analogWrite(PWMB, speedVal);
  currentState = BACKWARD;
}

// ================= Command Handler =================
void handleCommand(char cmd) {

  if (cmd == 'w' || cmd == 'a' || cmd == 's' || cmd == 'd') {
    lastMoveCmd = cmd;
  }

  lastPacketTime = millis();

  // Block ONLY forward when obstacle is detected
  if (obstacleDetected && cmd == 'w') return;


  switch (cmd) {
    case 'd': right(); break;
    case 'a': left(); break;
    case 'w': forward(); break;
    case 's': backward(); break;
    case 'x': stopMotors(); break;

    case '1': speedVal = 80; break;
    case '2': speedVal = 150; break;
    case '3': speedVal = 255; break;
  }
}

// ================= NeoPixel Update =================
void updateNeoPixels() {
  unsigned long now = millis();

  strip.setPixelColor(3, colorWhite);
  strip.setPixelColor(4, colorWhite);
  strip.setPixelColor(5, colorWhite);
  strip.setPixelColor(6, colorWhite);
  strip.setPixelColor(7, colorWhite);
  strip.setPixelColor(8, colorWhite);
  strip.setPixelColor(9, colorWhite);
  strip.setPixelColor(10, colorWhite);
  strip.setPixelColor(11, colorWhite);
  strip.setPixelColor(12, colorWhite);

  switch (currentState) {

    case FORWARD:
      strip.fill(colorWhite);
      strip.show();
      break;

    case BACKWARD:
      if (now - lastRGBTime > RGB_INTERVAL) {
        lastRGBTime = now;
        rgbHue++;
      }

      for (int i = 0; i < LED_COUNT; i++) {
        // Offset hue per LED to create flow
        strip.setPixelColor(i, wheel(rgbHue + i * 8));
      }

      strip.show();
      break;


    case LEFT:
      if (now - lastAnimationTime > animationSpeed) {
        lastAnimationTime = now;
        animationStep++;
        if (animationStep > 3) {
          animationStep = 0;
          strip.clear();
          strip.setPixelColor(4, colorWhite);
          strip.setPixelColor(5, colorWhite);
          strip.setPixelColor(6, colorWhite);
          strip.setPixelColor(7, colorWhite);
          strip.setPixelColor(8, colorWhite);
          strip.setPixelColor(9, colorWhite);
          strip.setPixelColor(10, colorWhite);
          strip.setPixelColor(11, colorWhite);
          strip.setPixelColor(12, colorWhite);
          ;
        }

        if (animationStep > 0) strip.setPixelColor(3, colorOrange);
        if (animationStep > 0) strip.setPixelColor(2, colorOrange);
        if (animationStep > 1) strip.setPixelColor(1, colorOrange);
        if (animationStep > 2) strip.setPixelColor(0, colorOrange);

        strip.show();
      }
      break;

    case RIGHT:
      if (now - lastAnimationTime > animationSpeed) {
        lastAnimationTime = now;
        animationStep++;
        if (animationStep > 3) {
          animationStep = 0;
          strip.clear();
          strip.setPixelColor(3, colorWhite);
          strip.setPixelColor(4, colorWhite);
          strip.setPixelColor(5, colorWhite);
          strip.setPixelColor(6, colorWhite);
          strip.setPixelColor(7, colorWhite);
          strip.setPixelColor(8, colorWhite);
          strip.setPixelColor(9, colorWhite);
          strip.setPixelColor(10, colorWhite);
          strip.setPixelColor(11, colorWhite);
        }

        if (animationStep > 0) strip.setPixelColor(12, colorOrange);
        if (animationStep > 0) strip.setPixelColor(13, colorOrange);
        if (animationStep > 1) strip.setPixelColor(14, colorOrange);
        if (animationStep > 2) strip.setPixelColor(15, colorOrange);

        strip.show();
      }
      break;

    case STOP:
      strip.clear();
      strip.show();
      break;
  }
}


// ================= Loop =================
void loop() {

  // Ultrasonic
  if (millis() - lastUltrasonicTime > ULTRASONIC_INTERVAL) {
    lastUltrasonicTime = millis();
    long dist = getDistanceCM();

    if (dist < OBSTACLE_DISTANCE && currentState == FORWARD) {
      if (!obstacleDetected) {
        stopMotors();
        obstacleDetected = true;
      }
    } else {
      if (obstacleDetected) {
        obstacleDetected = false;
      }
    }
  }


    // NRF
    if (radio.available()) {
      char cmd;
      radio.read(&cmd, sizeof(cmd));
      handleCommand(cmd);
    }

    // Bluetooth
    if (bt.available()) {
      handleCommand(bt.read());
    }

    // Serial
    if (Serial.available()) {
      handleCommand(Serial.read());
    }

    // NRF failsafe
    if (millis() - lastPacketTime > NRF_TIMEOUT) {
      stopMotors();
      recoverNRF();
      lastPacketTime = millis();
    }

    updateNeoPixels();
  }
