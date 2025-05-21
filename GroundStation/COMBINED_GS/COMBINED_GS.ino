/*

  This example shows how to connect to an EBYTE transceiver
  using an Arduino Nano

  This code for for the sender


  connections
  Module      Arduino
  M0          4
  M1          5
  Rx          2 (This is the MCU Tx lined)
  Tx          3 (This is the MCU Rx line)
  Aux         6
  Vcc         3V3
  Gnd         Gnd

*/

#include <SoftwareSerial.h>
#include "EBYTE.h"

#define PIN_RX 2
#define PIN_TX 3
#define PIN_M0 4
#define PIN_M1 5
#define PIN_AX 6


// i recommend putting this code in a .h file and including it
// from both the receiver and sender modules
struct DATA {
  char mode = '1';
  char DataPack[50];

};


// these are just dummy variables, replace with your own
int Chan;
DATA MyData;
unsigned long Last;

// connect to any digital pin to connect to the serial port
// don't use pin 01 and 1 as they are reserved for USB communications
SoftwareSerial ESerial(PIN_RX, PIN_TX);

// create the transceiver object, passing in the serial and pins
EBYTE Transceiver(&ESerial, PIN_M0, PIN_M1, PIN_AX);


// Helper function to convert a single hex character to its integer value
unsigned long hexCharToDec(char hexChar) {
  if (hexChar >= '0' && hexChar <= '9') {
    return hexChar - '0';
  } else if (hexChar >= 'A' && hexChar <= 'F') {
    return 10 + (hexChar - 'A');
  } else if (hexChar >= 'a' && hexChar <= 'f') {
    return 10 + (hexChar - 'a');
  }
  return 0; // Should not happen with valid hex input
}

// Function to perform string addition (for large numbers)
String stringAdd(String num1, String num2) {
  String result = "";
  int carry = 0;
  int i = num1.length() - 1;
  int j = num2.length() - 1;

  while (i >= 0 || j >= 0 || carry > 0) {
    int digit1 = (i >= 0) ? (num1.charAt(i) - '0') : 0;
    int digit2 = (j >= 0) ? (num2.charAt(j) - '0') : 0;
    int sum = digit1 + digit2 + carry;
    result = String(sum % 10) + result;
    carry = sum / 10;
    i--;
    j--;
  }
  return result;
}

// Function to perform string multiplication by a single digit
String stringMultiplyDigit(String num, int digit) {
  if (digit == 0) {
    return "0";
  }
  String result = "";
  int carry = 0;
  for (int i = num.length() - 1; i >= 0; i--) {
    int currentDigit = num.charAt(i) - '0';
    int product = currentDigit * digit + carry;
    result = String(product % 10) + result;
    carry = product / 10;
  }
  if (carry > 0) {
    result = String(carry) + result;
  }
  return result;
}

// Function to perform string multiplication by a power of 16
String stringMultiplyPowerOf16(String num, int power) {
  if (power == 0) {
    return num;
  }
  for (int i = 0; i < power; i++) {
    num += "0";
  }
  return num;
}

// Function to convert a hexadecimal string back to a decimal string
// This version attempts to handle larger numbers more accurately
String hexToDecimal(String hexInput) {
  if (hexInput == "0" || hexInput == "") {
    return "0";
  }

  String decimalString = "0";
  String currentPowerOf16 = "1"; // Start with 16^0

  // Remove any "0x" prefix if present
  if (hexInput.startsWith("0x") || hexInput.startsWith("0X")) {
    hexInput = hexInput.substring(2);
  }

  // Convert hex to uppercase for consistency
  hexInput.toUpperCase();

  // Process each hexadecimal digit from right to left
  for (int i = hexInput.length() - 1; i >= 0; i--) {
    unsigned long hexDigitValue = hexCharToDec(hexInput.charAt(i));

    // Calculate the decimal value of the current term (digit * 16^power) as a string
    String termValue = stringMultiplyDigit(currentPowerOf16, hexDigitValue);

    // Add the term value to the running decimal string
    decimalString = stringAdd(decimalString, termValue);

    // Calculate the next power of 16 (multiply currentPowerOf16 by 16) as a string
    currentPowerOf16 = stringMultiplyDigit(currentPowerOf16, 16);
  }

  return decimalString;
}

String decimalToHex(String decimalInput) {
  if (decimalInput == "0") {
    return "0";
  }

  String hexOutput = "";
  String quotient = decimalInput;

  while (quotient != "0") {
    int remainder = 0;
    String nextQuotient = "";

    // Perform long division by 16 on the string
    for (int i = 0; i < quotient.length(); i++) {
      int digit = quotient.charAt(i) - '0';
      int current = remainder * 10 + digit; // Combine remainder with current digit
      remainder = current % 16;             // New remainder
      nextQuotient += String(current / 16); // New quotient digit
    }

    // Remove leading zeros from the nextQuotient
    while (nextQuotient.length() > 1 && nextQuotient.charAt(0) == '0') {
      nextQuotient = nextQuotient.substring(1);
    }
    // If the quotient becomes empty, set it to "0"
    if (nextQuotient.length() == 0) {
      nextQuotient = "0";
    }

    // Convert remainder to hex character
    if (remainder < 10) {
      hexOutput = String(remainder) + hexOutput; // Prepend digit
    } else {
      hexOutput = String((char)('A' + (remainder - 10))) + hexOutput; // Prepend char A-F
    }

    quotient = nextQuotient; // Update quotient for next iteration
  }

  return hexOutput;
}


  String largeDecimal = "";
  String hexResult = decimalToHex(largeDecimal);


void setup() {


  Serial.begin(9600);

  ESerial.begin(9600);
  Serial.println("Starting Reader");
  Transceiver.init();
  Transceiver.PrintParameters();


}

void loop() {

String content;
int startIndex ;
int endIndex;

  if (ESerial.available()) {
    Transceiver.GetStruct(&MyData, sizeof(MyData));
    
          Serial.print("CUBESAT RAW:,");
          Serial.print(MyData.DataPack);
          
          String dataStr = String(MyData.DataPack);
             if(MyData.mode == '6'){
                startIndex = dataStr.indexOf("HA");
                endIndex = dataStr.indexOf("FA");

              if (startIndex != -1 && endIndex != -1 && endIndex > startIndex + 2) {
                  content = dataStr.substring(startIndex + 3, endIndex); 
                  Serial.print(",EXTRACTED CONTENT:,");
                  Serial.print(content);
                } else {
                  Serial.println("Pattern not found or malformed.");
                }
                
             }

             
             if (MyData.mode == '1') {
  int startIndex = dataStr.indexOf("MAACM");

  if (startIndex != -1) {
    // Extract everything after "MAACM"
    String content = dataStr.substring(startIndex + 5);  // +5 skips "MAACM"
    Serial.print(",EXTRACTED CONTENT:,");
    Serial.println(content);
  } else {
    Serial.println("Pattern 'MAACM' not found.");
  }
}

            
          

          String Decoded =  hexToDecimal(content);
          Serial.print(",DECODED:,");Serial.println(Decoded);

    
    Last = millis();

  }
  else {
    // if the time checker is over some prescribed amount
    // let the user know there is no incoming data
    if ((millis() - Last) > 1000) {
      Serial.println("Searching: ");
      Last = millis();
    }

  }
}
