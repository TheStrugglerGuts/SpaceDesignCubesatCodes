#include <SFE_BMP180.h>
#include <MPU9250_asukiaaa.h>
#include <Wire.h>


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

  


void setup()
{ 
  Serial.begin(9600);
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

  
char status;
  double battTemp, battVolt=0, battCur=0;
  Solar1Status = digitalRead(Solar1pin);
  Solar2Status = digitalRead(Solar2pin);
  Solar3Status = digitalRead(Solar3pin);
  Solar4Status = digitalRead(Solar4pin);
  Solar5Status = 5;// digitalRead(Solar5pin);
  
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
    String EPSValues = String((int)battTemp) + String(Solar1Status) + String(Solar2Status) + String(Solar3Status) + String(Solar4Status) + String(Solar5Status) + adcsxStr + adcsyStr + adcszStr ; //times 10 batt volt and curr for no decimal PLace
    Serial.println(EPSValues);

    delay(1000);
}
