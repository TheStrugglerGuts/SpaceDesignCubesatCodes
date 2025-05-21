#include "Wire.h"
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

/*--- struct data for the ebyte mode of transmission idk this is just how it works its nice rin----*/
SoftwareSerial ESerial(PIN_RX, PIN_TX);
EBYTE Transceiver(&ESerial, PIN_M0, PIN_M1, PIN_AX);

struct DATA {
  char mode = '6';
  char DataPack[50];

};

int Chan;
DATA MyData;
String receivedData; 

/*--- struct data for the ebyte mode of transmission idk this is just how it works its nice rin----*/



                          //*--Hex converter thingy---*/
                            
                            unsigned long hexCharToDec(char hexChar) {
                              if (hexChar >= '0' && hexChar <= '9') return hexChar - '0';
                              else if (hexChar >= 'A' && hexChar <= 'F') return 10 + (hexChar - 'A');
                              else if (hexChar >= 'a' && hexChar <= 'f') return 10 + (hexChar - 'a');
                              return 0;
                            }
                            
                            String stringAdd(String num1, String num2) {
                              String result = "";
                              int carry = 0;
                              int i = num1.length() - 1, j = num2.length() - 1;
                              while (i >= 0 || j >= 0 || carry > 0) {
                                int digit1 = (i >= 0) ? (num1.charAt(i--) - '0') : 0;
                                int digit2 = (j >= 0) ? (num2.charAt(j--) - '0') : 0;
                                int sum = digit1 + digit2 + carry;
                                result = char('0' + (sum % 10)) + result;
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
                                result = char('0' + (product % 10)) + result;
                                carry = product / 10;
                              }
                              if (carry > 0) result = String(carry) + result;
                              return result;
                            }
                            
                            
  String divideBy16(String number, int &remainderOut) {
  String quotient = "";
  int remainder = 0;

  for (int i = 0; i < number.length(); i++) {
    int current = remainder * 10 + (number.charAt(i) - '0');
    quotient += String(current / 16);
    remainder = current % 16;
  }

  // Trim leading zeros
  int firstNonZero = 0;
  while (firstNonZero < quotient.length() && quotient.charAt(firstNonZero) == '0') {
    firstNonZero++;
  }
  if (firstNonZero == quotient.length()) {
    quotient = "0";
  } else {
    quotient = quotient.substring(firstNonZero);
  }

  remainderOut = remainder;
  return quotient;
}

String decimalToHex(String decimalInput) {
  if (decimalInput == "0" || decimalInput.length() == 0) return "0";

  String hexOutput = "";
  String current = decimalInput;

  while (current != "0") {
    int remainder;
    current = divideBy16(current, remainder);
    char hexDigit = (remainder < 10) ? ('0' + remainder) : ('A' + remainder - 10);
    hexOutput = hexDigit + hexOutput;
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
  
      Transceiver.SendStruct(&MyData, sizeof(MyData)); // sends the data struct its neat
  
 
    //if (Serial.available()) {
      //  char UserInput = Serial.read();
        //if (UserInput != '\n' && UserInput != '\r') {
        //OperMode = UserInput;
       //}
    //}

            if (Serial.available() > 0) {
                receivedData = Serial.readStringUntil('\n'); // Read the incoming data from eps
    
            //    Serial.println("EPS: " + receivedData+ "\n\n");
            }
            
  float battVolt,battCur, busvoltage = 0, shuntvoltage = 0;;

  shuntvoltage = ina219.getShuntVoltage_mV(); 
  busvoltage = ina219.getBusVoltage_V(); 
  battCur = abs(ina219.getCurrent_mA()); 
  battCur/=10; // so if reading is 22 its .22 A 
  //Serial.print("batcur: "); Serial.println(battCur);  
  battVolt = busvoltage + (shuntvoltage / 1000);
  battVolt*= 10; 
  
  int resets=0, Uplinkstatus = 0, MissionsStatus = 0, ImagesTaken = 0;
 
       
    
    
                                                    /*--FOR RTC----*/

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
                                                        
                                                         /*--FOR RTC----*/


        //1 = Nominal Mode; 2 = Power Saving Mode; 3 = Emergency Mode; 4 = Capture Mode; 5 = Image Data Downlink; 6 = HK Telemetry Downlink
    
        if( OperMode == '1'){
        
          Serial.print("Operational Mode: ");
          Serial.println(OperMode);

            
         receivedData.remove(receivedData.length() - 1);
         //SPacers //putting '' shit to see if mag gawa ng shit
         
          String BeaconData = Header + String((int)battVolt) + String((int)battCur) +"'" + receivedData + "'" + String(resets) + String(Uplinkstatus) + String(MissionsStatus) + String(ImagesTaken);
          largeDecimal = String((int)battVolt) + String((int)battCur) + receivedData + String(resets) + String(Uplinkstatus) + String(MissionsStatus) + String(ImagesTaken);
          hexResult = "MAACM" + decimalToHex(largeDecimal);
          hexResult.toCharArray(MyData.DataPack,50);
          Serial.print(hexResult);Serial.print(" ");Serial.print(BeaconData);Serial.print(" Beacon Data: "); Serial.println(MyData.DataPack); 
          
        }
    
        if( OperMode == '2'){
          Serial.print("Operational Mode: ");Serial.println(OperMode);
          
          
        }
        
        if( OperMode == '3'){
          Serial.print("Operational Mode: ");Serial.println(OperMode);
        
        }
        
        if( OperMode == '4'){
          Serial.print("Operational Mode: ");Serial.println(OperMode);
         
        }
        
        if( OperMode == '5'){
          Serial.print("Operational Mode: "); Serial.println(OperMode);
        }
        
        if( OperMode == '6'){
          Serial.print("Operational Mode: ");
          Serial.println(OperMode);
          receivedData.remove(receivedData.length() - 1);
         String HKfulData = "HA" + secondsStr + minutesStr + hoursStr + "AA" +  String((int)battVolt) + String((int)battCur) +"'"+ receivedData + "'" + String(resets) + "FA"; // GPS KULANG
          largeDecimal = secondsStr + minutesStr + hoursStr +  String((int)battVolt) + String((int)battCur) + receivedData + String(resets);
          hexResult = "HA" + decimalToHex(largeDecimal) + "FA";
          hexResult.toCharArray(MyData.DataPack,50);
          Serial.print(hexResult);Serial.print(" ");Serial.print(HKfulData);Serial.print(" HK Data: "); Serial.println(MyData.DataPack); 
          
          
        }    


  delay(1000);

}
