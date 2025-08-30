// Master Arduino Code (I2C)

#include <Wire.h>

const int ledPin = 13;
const int slaveAddress = 9;  // Address for the slave Arduino
int ledState = 0;
const int potPin = A0;   // The analog pin connected to the potentiometer's center pin
void setup() {
  Wire.begin();  // Join the I2C bus as master
  pinMode(ledPin, OUTPUT);
}

void loop() {

  int potValue = analogRead(potPin);

  ledState = 1;
  digitalWrite(ledPin, ledState);
  Wire.beginTransmission(slaveAddress);  // Begin transmission to slave LED ON
  Wire.write(ledState);                  // Send the LED state
  Wire.endTransmission();

  delay(5);
  

  ledState = 0;
  digitalWrite(ledPin, ledState);
  Wire.beginTransmission(slaveAddress);  // Begin transmission to slave LED OFF
  Wire.write(ledState);                  // Send the LED state
  Wire.endTransmission();


  delay(500);
}
