/*
  Transmitter Code (Remote Control Side)
  Wiring: NRF CE->2, CSN->4, SCK->13, MOSI->11, MISO->12
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(8,9); // CE, CSN
const byte address[6] = "00001"; // Must match Receiver

void setup() {
  Serial.begin(9600);
  
  if (!radio.begin()) {
    Serial.println("NRF24 hardware not responding!");
    while (1);
  }
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening(); // Set to Transmitter mode
  
  Serial.println("Transmitter Ready. Type w, a, s, d, x");
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    
    // Send the character to the robot
    bool report = radio.write(&command, sizeof(command));
    
    if (report) {
      Serial.print("Sent: ");
      Serial.println(command);
    } else {
      Serial.println("Transmission Failed (Check wiring or power)");
    }
  }
}