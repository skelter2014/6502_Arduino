#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

// Buffer for serial data
#define BUFFER_SIZE 16
byte dataBuffer[BUFFER_SIZE];
unsigned int currentEEPROMAddress;

// Protocol constants
const char START_BYTE = 'S';
const char END_BYTE = 'E';
const char ESCAPE_BYTE = 'X'; // New escape character

enum State {
  WAITING_FOR_START,
  RECEIVING_ADDRESS,
  RECEIVING_DATA,
  RECEIVING_ESCAPED_BYTE
};

State currentState = WAITING_FOR_START;
int bytesRead = 0;
int addressBytesRead = 0;
byte addressBuffer[4];

/*
 * Output the address bits and outputEnable signal using shift registers.
 */
void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

/*
 * Read a byte from the EEPROM at the specified address.
 */
byte readEEPROM(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, INPUT);
  }
  setAddress(address, /*outputEnable*/ true);

  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}

/*
 * Write a byte to the EEPROM at the specified address.
 */
void writeEEPROM(int address, byte data) {
  setAddress(address, /*outputEnable*/ false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, OUTPUT);
  }

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);
  delay(10);
}

/*
 * Write data from a buffer to the EEPROM.
 */
void writeEEPROM_fromBuffer(unsigned int startAddress, byte* buffer, int bufferSize) {
  Serial.print("Writing to EEPROM starting at address ");
  Serial.println(startAddress, HEX);
  for (int i = 0; i < bufferSize; i++) {
    writeEEPROM(startAddress + i, buffer[i]);
  }
  Serial.println("writeEEPROM_fromBuffer complete.");
}

/*
 * Read the contents of the EEPROM and print them to the serial monitor.
 */
void printContents(int firstAddress, int lastAddress) {
  Serial.println("\n--- EEPROM Contents ---");
  for (int base = firstAddress; base <= lastAddress; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1) {
      data[offset] = readEEPROM(base + offset);
    }
    char buf[82];
    sprintf(buf, "%07x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    Serial.println(buf);
  }
  Serial.println("-----------------------\n");
}

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);
  Serial.begin(57600);
  while (!Serial) {
    // Wait for serial port to connect. Needed for native USB boards only.
  }
  Serial.println("Ready to receive data via Serial (escaped protocol with address).");
}

void loop() {
  if (Serial.available()) {
    byte incomingByte = Serial.read();

    switch (currentState) {
      case WAITING_FOR_START:
        if (incomingByte == START_BYTE) {
          Serial.println("Started receiving data batch...");
          addressBytesRead = 0;
          currentState = RECEIVING_ADDRESS;
        }
        break;

      case RECEIVING_ADDRESS:
        addressBuffer[addressBytesRead++] = incomingByte;
        if (addressBytesRead == 4) {
          memcpy(&currentEEPROMAddress, addressBuffer, 4);
          bytesRead = 0;
          currentState = RECEIVING_DATA;
        }
        break;

      case RECEIVING_DATA:
        if (incomingByte == END_BYTE) {
          // Write the received data
          writeEEPROM_fromBuffer(currentEEPROMAddress, dataBuffer, bytesRead);
          
          // Pad with 0xff if buffer not full (assuming fixed-size blocks)
          if (bytesRead < BUFFER_SIZE) {
            for (int i = bytesRead; i < BUFFER_SIZE; i++) {
              writeEEPROM(currentEEPROMAddress + i, 0xff);
            }
          }
          
          Serial.println("Write complete.");
          printContents(currentEEPROMAddress - BUFFER_SIZE, currentEEPROMAddress + BUFFER_SIZE);
          currentState = WAITING_FOR_START;
        } else if (incomingByte == ESCAPE_BYTE) {
          currentState = RECEIVING_ESCAPED_BYTE;
        } else {
          if (bytesRead < BUFFER_SIZE) {
            dataBuffer[bytesRead++] = incomingByte;
          }
        }
        break;

      case RECEIVING_ESCAPED_BYTE:
        if (bytesRead < BUFFER_SIZE) {
          dataBuffer[bytesRead++] = incomingByte;
        }
        currentState = RECEIVING_DATA;
        break;
    }
  }
}
