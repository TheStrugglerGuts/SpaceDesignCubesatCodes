#include <Wire.h>

#define EEPROM_ADDR 0x50

char mymessage[7] = "HELLO!";  // Increase the size to fit the null terminator
char readitnow;                // Declare as a single char without initialization

void setup() {
  Wire.begin(D2, D1); // Use D1 for SDA, D2 for SCL
  Serial.begin(115200);
  delay(1500);

  Serial.println("Writing EEProm");
  writeonce();

  Serial.println("Reading EEProm");
  readonce();
}

void writeonce() {
  for (int i = 0; i < 6; i++) {
    i2c_eeprom_write_byte(EEPROM_ADDR, i, mymessage[i]);
    Serial.print("Address: ");
    Serial.print(i);
    Serial.print("   Value: ");
    Serial.println(mymessage[i]);
    delay(500);
  }
}

void readonce() {
  for (int i = 0; i < 6; i++) {
    readitnow = i2c_eeprom_read_byte(EEPROM_ADDR, i);
    Serial.print("Address: ");
    Serial.print(i);
    Serial.print("   Value: ");
    Serial.println(readitnow);
    delay(500);
  }
}

void loop() {}

void i2c_eeprom_write_byte(int deviceaddress, unsigned int eeaddress, byte data) {
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));    // Address High Byte
  Wire.write((int)(eeaddress & 0xFF));  // Address Low Byte
  Wire.write(data);
  Wire.endTransmission();
}

byte i2c_eeprom_read_byte(int deviceaddress, unsigned int eeaddress) {
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));    // Address High Byte
  Wire.write((int)(eeaddress & 0xFF));  // Address Low Byte
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress, 1);
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}
