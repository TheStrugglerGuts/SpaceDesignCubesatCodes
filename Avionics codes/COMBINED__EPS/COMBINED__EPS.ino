#include <SFE_BMP180.h>
#include <Wire.h>


SFE_BMP180 pressure;

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

String Sep = "";


void setup()
{ 
  Serial.begin(9600);
  pinMode(Solar1pin, INPUT_PULLUP);
  pinMode(Solar2pin, INPUT_PULLUP);
  pinMode(Solar3pin, INPUT_PULLUP);
  pinMode(Solar4pin, INPUT_PULLUP);
  pinMode(Solar5pin, INPUT_PULLUP);
  
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

  
    status = pressure.startTemperature();
          if (status != 0){
            delay(status);
            status = pressure.getTemperature(battTemp);
          }
          else //Serial.println("error starting temperature measurement\n");
    
     
    battVolt*= 10;
    battCur*=  10;
    //Serial.println(Solar5Status);
    String EPSValues = String((int)battTemp) + String(Solar1Status) + String(Solar2Status) + String(Solar3Status) + String(Solar4Status) + String(Solar5Status); //times 10 batt volt and curr for no decimal PLace
    Serial.println(EPSValues);

    delay(1000);
}
