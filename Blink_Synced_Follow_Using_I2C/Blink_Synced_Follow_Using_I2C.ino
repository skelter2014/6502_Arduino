// Slave Arduino Code (I2C)

#include <Wire.h>

const int ledPin = 13;
const int slaveAddress = 9; // Address of this slave

void setup() {
  Wire.begin(slaveAddress); // Join the I2C bus as a slave
  Wire.onReceive(receiveEvent); // Register a function to call when data is received
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // The main loop is empty because the `receiveEvent` function handles the blinking
}

void receiveEvent(int howMany) {
  if (Wire.available()) {
    int receivedState = Wire.read();
    digitalWrite(ledPin, receivedState);
  }
}
