#include "Wire.h"
#include <MPU6050_light.h>
#include <I2C_RTC.h>
#include <Adafruit_INA219.h>
#include <SoftwareSerial.h>
#include "EBYTE.h"
#include <limits.h>
#include <string.h>

#define PIN_RX 3
#define PIN_TX 2
#define PIN_M0 5
#define PIN_M1 4
#define PIN_AX 6

Adafruit_INA219 ina219;
MPU6050 mpu6050(Wire);
static DS3231 RTC;

int Solar1pin = 13;
int Solar2pin = 2;
int Solar3pin = 3;
int Solar4pin = 4;
int Solar5pin = 5;

int Solar1Status = 0;
int Solar2Status = 0;
int Solar3Status = 0;
int Solar4Status = 0;
int Solar5Status = 0;



String TotalData;

String Header = "MAACM";
String Footer = "0xB2";
char OperMode = '6'; // opermode 1 - 6

struct DATA {
  char mode = '6';
  char DataPack[50];

};


int Chan;
DATA MyData;
String receivedData; 

SoftwareSerial ESerial(PIN_RX, PIN_TX);
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
  Wire.begin();
  ESerial.begin(9600);
   Serial.println("Starting Sender");

   Transceiver.init();
  Chan = 5;
   Transceiver.SetChannel(Chan);
  Transceiver.SaveParameters(PERMANENT);
  Transceiver.PrintParameters();

  byte status = mpu6050.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ } // stop everything if could not connect to MPU6050
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu6050.calcOffsets(true,true); // gyro and accelero
  Serial.println("Done!\n");

  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    //while (1)
      //delay(10); 
  }
  
 
  
  RTC.begin();  
  RTC.setHourMode(CLOCK_H12);

  
}




void loop() {
  MyData.mode = OperMode;
  //strcpy(MyData.DataPack,"asds");
  
      Transceiver.SendStruct(&MyData, sizeof(MyData));
  
   mpu6050.update();
    //if (Serial.available()) {
      //  char UserInput = Serial.read();
        //if (UserInput != '\n' && UserInput != '\r') {
        //OperMode = UserInput;
       //}
    //}

            if (Serial.available() > 0) {
                receivedData = Serial.readStringUntil('\n'); // Read the incoming data
    
            //    Serial.println("EPS: " + receivedData+ "\n\n");
            }
            
  float battVolt = 80 ,battCur = 2.3, busvoltage = 0, shuntvoltage = 0;;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  battCur = abs(ina219.getCurrent_mA());
  battVolt = busvoltage + (shuntvoltage / 1000);
  
 
  double ADCSX = mpu6050.getAccX(), ADCSY = mpu6050.getAccY(), ADCSZ = mpu6050.getAccZ();
  
  int resets=0, Uplinkstatus = 0, MissionsStatus = 0, ImagesTaken = 0;
 
       
    battVolt*= 10;
    
                                                    //--- makes the fucking accel values 1 decimal and removes the motherfucking decimal-----/////////
                                                          ADCSX = abs(round(ADCSX * 10) / 10.0);
                                                          ADCSY = abs(round(ADCSY * 10) / 10.0);
                                                          ADCSZ = abs(round(ADCSZ * 10) / 10.0);
                                                          
                                                          int adcsxInt = (int)(ADCSX*10);
                                                          int adcsyInt = (int)(ADCSY*10);
                                                          int adcszInt = (int)(ADCSZ*10);
                                                        
                                                        String adcsxStr,adcsyStr,adcszStr;
                                                        
                                                        if (adcsxInt < 10 && adcsxInt >= 0) 
                                                          adcsxStr = "0" + String(adcsxInt); 
                                                        else 
                                                          adcsxStr = String(adcsxInt);      
                                                        
                                                        
                                                        if (adcsyInt < 10 && adcsyInt >= 0) 
                                                          adcsyStr = "0" + String(adcsyInt); 
                                                        else 
                                                          adcsyStr = String(adcsyInt);      
                                                        
                                                        
                                                        if (adcszInt < 10 && adcszInt >= 0) 
                                                          adcszStr = "0" + String(adcszInt); 
                                                        else 
                                                          adcszStr = String(adcszInt);     

                                                        int secondss = RTC.getSeconds(), minutes = RTC.getMinutes(), hours = RTC.getHours();
                                                        String secondsStr, minutesStr, hoursStr;
                                                        
                                                        if( secondss < 10 )
                                                          secondsStr = "0" + String(secondss);
                                                        else
                                                          secondsStr = String(secondss);
                                                          
                                                        if( minutes < 10 )
                                                          minutesStr = "0" + String(minutes);
                                                        else
                                                          minutesStr = String(minutes);
                                                          
                                                        if( hours < 10 )
                                                          hoursStr = "0" + String(hours);
                                                        else
                                                          hoursStr = String(hours);
                                                        



        //1 = Nominal Mode; 2 = Power Saving Mode; 3 = Emergency Mode; 4 = Capture Mode; 5 = Image Data Downlink; 6 = HK Telemetry Downlink
    
        if( OperMode == '1'){
        
          Serial.print("Operational Mode: ");
          Serial.println(OperMode);

            
         // receivedData.remove(receivedData.length() - 1);
          String BeaconData = Header + String((int)battVolt) + String((int)battCur) + "'" + receivedData +"'" + String(resets) + adcsxStr + adcsyStr + adcszStr + String(Uplinkstatus) + String(MissionsStatus) + String(ImagesTaken);
          largeDecimal = String((int)battVolt) + String((int)battCur) + receivedData + String(resets) + adcsxStr + adcsyStr + adcszStr + String(Uplinkstatus) + String(MissionsStatus) + String(ImagesTaken);
          hexResult = "MAACM" +decimalToHex(largeDecimal);
          hexResult.toCharArray(MyData.DataPack,50);
          Serial.print(hexResult);Serial.print(" ");Serial.print(BeaconData);Serial.print(" Beacon Data: "); Serial.println(MyData.DataPack); 
          
        }
    
        if( OperMode == '2'){
          Serial.print("Operational Mode: ");
          Serial.println(mpu6050.getAccX());
          
        }
        
        if( OperMode == '3'){
          Serial.print("Operational Mode: ");
          Serial.println(OperMode);
        }
        
        if( OperMode == '4'){
          Serial.print("Operational Mode: ");
          Serial.println(OperMode);
        }
        
        if( OperMode == '5'){
          Serial.print("Operational Mode: ");
          Serial.println(OperMode);
        }
        
        if( OperMode == '6'){
          Serial.print("Operational Mode: ");
          Serial.println(OperMode);
          
         String HKfulData = Header + secondsStr + minutesStr + hoursStr +  String((int)battVolt) + String((int)battCur) +"'"+ receivedData +"'"+ String(resets) + adcsxStr + adcsyStr + adcszStr + String(Uplinkstatus) + String(MissionsStatus) + String(ImagesTaken) + Footer;
          largeDecimal = secondsStr + minutesStr + hoursStr +  String((int)battVolt) + String((int)battCur) + receivedData + String(resets) + adcsxStr + adcsyStr + adcszStr + String(Uplinkstatus) + String(MissionsStatus) + String(ImagesTaken);
          hexResult = "MAACM" + decimalToHex(largeDecimal) + "END";
          hexResult.toCharArray(MyData.DataPack,50);
          Serial.print(hexResult);Serial.print(" ");Serial.print(HKfulData);Serial.print(" HK Data: "); Serial.println(MyData.DataPack); 
          
          
        }    


  delay(1000);

}
