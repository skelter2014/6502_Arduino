// Master Arduino Code
const int ledPin = 13;   // The pin for the master's LED
const int syncPin = 2;   // The pin to send the sync signal to the slave
const int potPin = A0;   // The analog pin connected to the potentiometer's center pin

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(syncPin, OUTPUT);
  // Potentiometer pin is an analog input by default, so no need for pinMode
}

void loop() {
  // Read the value from the potentiometer (0 to 1023)
  int potValue = analogRead(potPin);

    // Toggle the LED state and send it to the slave
  if (ledState == 0) {
    ledState = 1;
  } else {
    ledState = 0;
  }

  // Map the potentiometer value to a desired delay range
  // Here, we map it from 0-1023 to 100-2000 milliseconds (0.1 to 2 seconds)
  // You can adjust the minimum and maximum delay values as needed
  int waitTime = map(potValue, 0, 1023, 100, 2000); 

  // Blink and send the signal at the same time
  digitalWrite(ledPin, HIGH);
  digitalWrite(syncPin, HIGH);
  delay(1); // Small delay to ensure the signal is read

  digitalWrite(ledPin, LOW);
  digitalWrite(syncPin, LOW);
  delay(waitTime); // Wait for the duration controlled by the potentiometer
}
