// BMP085_test2
// by Filipe Vieira - 2010
// Simple test of BMP085 output using default settings using dynamic update.
// NOTE: in order to take full advantage of dynamic measurement, automatic temperature updates must be disabled.
// Changing AUTO_UPDATE_TEMPERATURE to false in bmp085.h will do it.
// Note that Serial is set to 115200.

#include <Wire.h>
#include "BMP085.h"

BMP085 dps;      // Digital Pressure Sensor 

float Pressure = 0, Altitude = 0;
unsigned long time1=0;

void setup(void) {
  Serial.begin(115200);
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
  
  // calculate temperature every 1 sec (dynamic measurement)
  // NOTE: in order to take full advantage of dynamic measurement set AUTO_UPDATE_TEMPERATURE to false in bmp085.h
  if (((millis() - time1)/1000.0) >= 1.0) {     
     dps.calcTrueTemperature();
     time1 = millis();      
  }
 
  dps.getPressure(&Pressure);
  dps.getAltitude(&Altitude);

  Serial.print("  Alt(m):");
  Serial.print(Altitude);
  Serial.print("  Pressure(hPa):");
  Serial.println(Pressure);
}