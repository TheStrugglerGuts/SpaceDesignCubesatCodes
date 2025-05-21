#include <SFE_BMP180.h>
#include <MPU9250_asukiaaa.h>
#include <Wire.h>
#include <SoftwareSerial.h>


SoftwareSerial ss(8, 9);

SFE_BMP180 pressure;
MPU9250_asukiaaa mySensor;
float aX, aY, aZ, aSqrt, gX, gY, gZ;

int Solar1pin = 2;
int Solar2pin = 3;
int Solar3pin = 4;
int Solar4pin = 5;
int Solar5pin = 6;

int Solar1Status = 0;
int Solar2Status = 0;
int Solar3Status = 0;
int Solar4Status = 0;
int Solar5Status = 0;

String Sep = "";

                    String readNextGPGLL() {
                      String currentSentence = "";
                    
                      while (true) {
                        while (ss.available() > 0) {
                          char c = ss.read();
                          currentSentence += c;
                    
                          if (c == '\n') {
                            if (currentSentence.startsWith("$GPGLL")) {
                              return currentSentence;
                            }
                            currentSentence = "";
                          }
                        }
                      }
                    }

                    float convertToDecimalDegrees(String raw, String direction) {
                    if (raw.length() < 4) return 0.0;
                  
                    int dotIndex = raw.indexOf('.');
                    int degreeDigits = (dotIndex > 2) ? dotIndex - 2 : 2;
                    
                    float degrees = raw.substring(0, degreeDigits).toFloat();
                    float minutes = raw.substring(degreeDigits).toFloat();
                  
                    float decimal = degrees + (minutes / 60.0);
                  
                    if (direction == "S" || direction == "W") {
                      decimal = -decimal;
                    }
                  
                    return decimal;
                  }
                  
                  String extractLatLongDecimal(String gpgll) {
                    String fields[7];
                    int index = 0;
                    int start = 0;
                  
                    for (int i = 0; i < gpgll.length(); i++) {
                      if (gpgll.charAt(i) == ',' || gpgll.charAt(i) == '*') {
                        fields[index++] = gpgll.substring(start, i);
                        start = i + 1;
                      }
                    }
                  
                    String latRaw = fields[1];
                    String latDir = fields[2];
                    String lonRaw = fields[3];
                    String lonDir = fields[4];
                  
                    if (latRaw == "" || latDir == "" || lonRaw == "" || lonDir == "") {
                      return "0";
                    }
                  
                    float lat = convertToDecimalDegrees(latRaw, latDir);
                    float lon = convertToDecimalDegrees(lonRaw, lonDir);
                  
                    return String(lat, 6) + "," + String(lon, 6);
                  }
                    


void setup()
{ 
  Serial.begin(9600);
  ss.begin(9600);
  pinMode(Solar1pin, INPUT_PULLUP);
  pinMode(Solar2pin, INPUT_PULLUP);
  pinMode(Solar3pin, INPUT_PULLUP);
  pinMode(Solar4pin, INPUT_PULLUP);
  pinMode(Solar5pin, INPUT_PULLUP);
  mySensor.beginAccel();
  mySensor.beginGyro();
  
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {    Serial.println("BMP180 init fail\n\n");
    //while(1); // Pause forever.
  }
}

void loop() {
  // put your main code here, to run repeatedly:

   String GPS = readNextGPGLL();
   String latLongDecimal = extractLatLongDecimal(GPS);
char status;
  double battTemp, battVolt=0, battCur=0;
  Solar1Status = digitalRead(Solar1pin);
  Solar2Status = digitalRead(Solar2pin);
  Solar3Status = digitalRead(Solar3pin);
  Solar4Status = digitalRead(Solar4pin);
  Solar5Status = digitalRead(Solar5pin);
  
                                                      mySensor.accelUpdate();
                                                        aX = mySensor.accelX();
                                                        aY = mySensor.accelY();
                                                        aZ = mySensor.accelZ();
                                                        //FOr Testing Serial.print("X: ");Serial.print(aX);Serial.print("Y: ");Serial.print(aY);Serial.print("Z: ");Serial.print(aZ);Serial.print("\n\n");
                                                        
                                                        aX = abs(round(aX * 10) / 10.0);
                                                        aY = abs(round(aY * 10) / 10.0);
                                                        aZ = abs(round(aZ * 10) / 10.0);
                                                        int adcsxInt = (int)(aX*10);
                                                        int adcsyInt = (int)(aY*10);
                                                        int adcszInt = (int)(aZ*10);

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

    
    status = pressure.startTemperature();
          if (status != 0){
            delay(status);
            status = pressure.getTemperature(battTemp);
          }
          else //Serial.println("error starting temperature measurement\n");
    
     
    battVolt*= 10;
    battCur*=  10;
    //Serial.println(Solar5Status);
    String EPSValues = String((int)battTemp) + String(Solar1Status) + String(Solar2Status) + String(Solar3Status) + String(Solar4Status) + String(Solar5Status) + adcsxStr + adcsyStr + adcszStr + latLongDecimal ; //times 10 batt volt and curr for no decimal PLace
    Serial.println(EPSValues);

    delay(1000);
}
