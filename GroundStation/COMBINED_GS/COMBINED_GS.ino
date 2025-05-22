#include <SoftwareSerial.h>
#include "EBYTE.h"

#define PIN_RX 2
#define PIN_TX 3
#define PIN_M0 4
#define PIN_M1 5
#define PIN_AX 6

struct DATA {
  char mode = '1';
  char DataPack[50];
};

int Chan;
DATA MyData;
unsigned long Last;

SoftwareSerial ESerial(PIN_RX, PIN_TX);
EBYTE Transceiver(&ESerial, PIN_M0, PIN_M1, PIN_AX);

unsigned long hexCharToDec(char hexChar) {
  if (hexChar >= '0' && hexChar <= '9') return hexChar - '0';
  else if (hexChar >= 'A' && hexChar <= 'F') return 10 + (hexChar - 'A');
  else if (hexChar >= 'a' && hexChar <= 'f') return 10 + (hexChar - 'a');
  return 0;
}

String stringAdd(String num1, String num2) {
  String result = "";
  int carry = 0, i = num1.length() - 1, j = num2.length() - 1;
  while (i >= 0 || j >= 0 || carry > 0) {
    int digit1 = (i >= 0) ? (num1.charAt(i--) - '0') : 0;
    int digit2 = (j >= 0) ? (num2.charAt(j--) - '0') : 0;
    int sum = digit1 + digit2 + carry;
    result = String(sum % 10) + result;
    carry = sum / 10;
  }
  return result;
}

String stringMultiplyDigit(String num, int digit) {
  if (digit == 0) return "0";
  String result = "";
  int carry = 0;
  for (int i = num.length() - 1; i >= 0; i--) {
    int currentDigit = num.charAt(i) - '0';
    int product = currentDigit * digit + carry;
    result = String(product % 10) + result;
    carry = product / 10;
  }
  if (carry > 0) result = String(carry) + result;
  return result;
}

String hexToDecimal(String hexInput) {
  if (hexInput == "0" || hexInput == "") return "0";
  if (hexInput.startsWith("0x") || hexInput.startsWith("0X")) hexInput = hexInput.substring(2);
  hexInput.toUpperCase();
  String decimalString = "0";
  String currentPowerOf16 = "1";
  for (int i = hexInput.length() - 1; i >= 0; i--) {
    unsigned long hexDigitValue = hexCharToDec(hexInput.charAt(i));
    String termValue = stringMultiplyDigit(currentPowerOf16, hexDigitValue);
    decimalString = stringAdd(decimalString, termValue);
    currentPowerOf16 = stringMultiplyDigit(currentPowerOf16, 16);
  }
  return decimalString;
}

void setup() {
  Serial.begin(9600);
  ESerial.begin(9600);
  Serial.println("Starting Reader");
  Transceiver.init();
  Transceiver.PrintParameters();
}
char OperMode = '1';
void loop() {

  if (Serial.available()) {
        char UserInput = Serial.read();
        if (UserInput != '\n' && UserInput != '\r') {
        OperMode = UserInput;
        while(1){
        Transceiver.SendStruct(&MyData, sizeof(MyData));
        delay(500);
        break;
        }
       }
    }
  static String content = "";
  static int startIndex = -1, endIndex = -1;

  if (ESerial.available()) {
  
    Transceiver.GetStruct(&MyData, sizeof(MyData));

    Serial.print("CUBESAT RAW:,");
    Serial.print(MyData.DataPack);

    String dataStr = String(MyData.DataPack);
    content = "";
    MyData.mode = OperMode;
    if (MyData.mode == '6') {
      startIndex = dataStr.indexOf("HA");
      endIndex = dataStr.indexOf("FA");

      if (startIndex != -1 && endIndex != -1 && endIndex > startIndex + 2) {
        content = dataStr.substring(startIndex + 2, endIndex);
        Serial.print(",EXTRACTED CONTENT:,");
        Serial.print(content);
      } else {
        Serial.println("Pattern not found or malformed.");
      }
    }

    if (MyData.mode == '1') {
      startIndex = dataStr.indexOf("MAACM");

      if (startIndex != -1) {
        content = dataStr.substring(startIndex + 5);
        Serial.print(",EXTRACTED CONTENT:,");
        Serial.print(content);
      } else {
        Serial.println("Pattern 'MAACM' not found.");
      }
    }

    // Only decode if content was extracted
    if (content.length() > 0) {
      String decoded = hexToDecimal(content);
      Serial.print(",DECODED:,");
      Serial.println(decoded);
    }

    Last = millis();
  } else if ((millis() - Last) > 1000) {
    Serial.println("Searching: ");
    Last = millis();
  }
}
