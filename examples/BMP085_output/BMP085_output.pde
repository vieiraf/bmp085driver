// BMP085_output
// by Filipe Vieira - 2010
// Simple example of library usage with almost every BMP085 and lib features being used.

#include <Wire.h>
#include "BMP085.h"

BMP085 dps;
float Temperature = 0, Pressure = 0, Altitude = 0;

void setup(void) {
  Serial.begin(9600);
  Wire.begin();
  delay(1000);
	
  dps.init();   
  
  showall();
  
  Serial.println("Registers dump");
  Serial.println("==========================================================");
  dps.dumpRegisters();
  Serial.println("Calibration data");
  Serial.println("==========================================================");
  dps.dumpCalData();
  
  delay(5000);
}
 

void loop(void) {
  dps.getTemperature(&Temperature); 
  dps.getPressure(&Pressure);
  dps.getAltitude(&Altitude);
  
  Serial.print("Temp(C):");
  Serial.print(Temperature);
  Serial.print("  Alt(m):");
  Serial.print(Altitude);
  Serial.print("  Pressure(hPa):");
  Serial.println(Pressure);
}

void showall(void) { 
  Serial.println("Current BMP085 settings");
  Serial.println("==========================================================");
  Serial.print("device address                  = 0x");
  Serial.println(dps.getDevAddr(), HEX);
  Serial.print("Mode                            = ");
  switch (dps.getMode()) {
    case MODE_ULTRA_LOW_POWER: 
      Serial.println("MODE_ULTRA_LOW_POWER");
      break;
    case MODE_STANDARD: 
      Serial.println("MODE_STANDARD");
      break;    
    case MODE_HIGHRES: 
      Serial.println("MODE_HIGHRES");
      break;    
    case MODE_ULTRA_HIGHRES:     
      Serial.println("MODE_ULTRA_HIGHRES");
      break; 
  }  
}