// BMP085_test1
// by Filipe Vieira - 2010
// Simple test of BMP085 output using default settings.
// This example requires AUTO_UPDATE_TEMPERATURE to be true in bmp085.h otherwise temperature will not update.

#include <Wire.h>
#include "BMP085.h"

BMP085 dps;      // Digital Pressure Sensor 

float Temperature = 0, Pressure = 0, Altitude = 0;

void setup(void) {
  Serial.begin(9600);
  Wire.begin();
  delay(1000);
  
 // uncomment for different initialization settings

  //dps.init();     // QFE (Field Elevation above ground level) is set to 0 meters.
                  // same as init(MODE_STANDARD, 0.0, true);
  //dps.init(MODE_STANDARD, 1018.50, false);  //  false = using hpa units
                  // this initialization is useful for normalizing pressure to specific datum.
                  // OR setting current local hPa information from a weather station/local airport (QNH).
  dps.init(MODE_STANDARD, 250.0, true);  // true = using meter units
                  // this initialization is useful if current altitude is known,
                  // pressure will be calculated based on TruePressure and known altitude.

  // note: use zeroCal only after initialization.
  // dps.zeroCal(1018.00, 0.0);    // set zero point
}            

void loop(void) { 
  dps.getPressure(&Pressure);
  dps.getAltitude(&Altitude);
  
  Serial.print("  Alt(m):");
  Serial.print(Altitude);
  Serial.print("  Pressure(hPa):");
  Serial.println(Pressure);
}