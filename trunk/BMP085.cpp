/****************************************************************************
* BMP085.cpp - BMP085/I2C (Digital Pressure Sensor) library for Arduino     *
* Copyright 2010 Filipe Vieira                                              *
*                                                                           *
* This file is part of BMP085 Arduino library.                              *
*                                                                           *
* This library is free software: you can redistribute it and/or modify      *
* it under the terms of the GNU Lesser General Public License as published  *
* by the Free Software Foundation, either version 3 of the License, or      *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU Lesser General Public License for more details.                       *
*                                                                           *
* You should have received a copy of the GNU Lesser General Public License  *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
****************************************************************************/
/****************************************************************************
* Tested on Arduino Mega with BMP085 Breakout                               *
* SDA   -> pin 20   (no pull up resistors)                                  *
* SCL   -> pin 21   (no pull up resistors)                                  *
* XCLR  -> not connected                                                    *
* EOC   -> not connected                                                    *
* GND   -> pin GND                                                          *
* VCC   -> pin 3.3V                                                         *
* NOTE: SCL and SDA needs pull-up resistors for each I2C bus.               *
*  2.2kOhm..10kOhm, typ. 4.7kOhm                                            *
*****************************************************************************/
#include "BMP085.h"
#include <Wire.h>

BMP085::BMP085() {
  _dev_address = BMP085_DEFAULT_ADDR;
  _pressure_waittime[0] = 4500; // These are maximum convertion times.
  _pressure_waittime[1] = 7500; // It is possible to use pin EOC (End Of Conversion)
  _pressure_waittime[2] = 13500;  // to check if conversion is finished (logic 1) 
  _pressure_waittime[3] = 25500;  // or running (logic 0) insted of waiting for convertion times.
  _meter_Offset = 0.0;
  _hPa_Offset = 0.0;
}

void BMP085::init() {  
  init(MODE_STANDARD, 0.0, true);
}

void BMP085::init(byte _BMPMode, float _initVal, bool _Unitmeters){     
  getCalData();               // initialize cal data
  calcTrueTemperature();      // initialize b5
  setMode(_BMPMode);
  _Unitmeters ? setLocalAbsAlt(_initVal) : setLocalPressure(_initVal); 
}

byte BMP085::getDevAddr() {   
  return _dev_address;
}

byte BMP085::getMode(){
  return _oss;
}

void BMP085::setMode(byte _BMPMode){
  _oss = _BMPMode;
}

void BMP085::setLocalPressure(float _hPa){   
  float tmp_alt;
 
  _param_datum = _hPa;   
  getAltitude(&tmp_alt);    // calc altitude based on current pressure   
  _param_meters = tmp_alt;
}

void BMP085::setLocalAbsAlt(float _meters){  
  float tmp_hpa;
 
  _param_meters = _meters;   
  getPressure(&tmp_hpa);    // calc pressure based on current altitude
  _param_datum = tmp_hpa;
}

void BMP085::setAltOffset(float _meters){
  _meter_Offset = _meters;
}

void BMP085::sethPaOffset(float _hPa){
  _hPa_Offset = _hPa;
}

void BMP085::zeroCal(float _hPa, float _meters){
  setAltOffset(_meters - _param_meters);    
  sethPaOffset(_hPa - _param_datum);    
}

void BMP085::getPressure(float *_hPa){   
  long TruePressure = 0;

  calcTruePressure(&TruePressure); 
  *_hPa = TruePressure / 100.0 / pow((1.0 - _param_meters / 44330.0), 5.255) + _hPa_Offset;
}

void BMP085::getAltitude(float *_meters){  
  long TruePressure = 0;

  calcTruePressure(&TruePressure);  
  *_meters = 44330.0 * (1.0 - pow(((TruePressure / 100.0) / _param_datum), 0.190295)) + _meter_Offset;  
}

void BMP085::getTemperature(float *_Temperature) {
  calcTrueTemperature();                            // force b5 update
  *_Temperature = ((b5 + 8) >> 4) / 10.0;
}

void BMP085::calcTrueTemperature(){
  long ut,x1,x2;

  //read Raw Temperature
  writemem(CONTROL, READ_TEMPERATURE);
  delayMicroseconds(4500);                          // min. 4.5ms read Temp delay
  readmem(CONTROL_OUTPUT, 2, _buff); 
  ut = ((long)_buff[0] << 8 | ((long)_buff[1]));    // uncompensated temperature value
  
  // calculate temperature
  x1 = ((long)ut - ac6) * ac5 >> 15;
  x2 = ((long)mc << 11) / (x1 + md);
  b5 = x1 + x2;
}

void BMP085::calcTruePressure(long *_TruePressure) {
  long up,x1,x2,x3,b3,b6,p;
  unsigned long b4,b7;

  #if AUTO_UPDATE_TEMPERATURE
  calcTrueTemperature();        // b5 update 
  #endif 
 
 //read Raw Pressure
  writemem(CONTROL, READ_PRESSURE+(_oss << 6));
  delayMicroseconds(_pressure_waittime[_oss]);    
  readmem(CONTROL_OUTPUT, 3, _buff);  
  up = ((((long)_buff[0] <<16) | ((long)_buff[1] <<8) | ((long)_buff[2])) >> (8-_oss)); // uncompensated pressure value
  
  // calculate true pressure
  b6 = b5 - 4000;             // b5 is updated by calcTrueTemperature().
  x1 = (b2* (b6 * b6 >> 12)) >> 11;
  x2 = ac2 * b6 >> 11;
  x3 = x1 + x2;
  b3 = (((int32_t)ac1 * 4 + x3) << _oss) + 2 >> 2;
  x1 = ac3 * b6 >> 13;
  x2 = (b1 * (b6 * b6 >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;
  b4 = (ac4 * (uint32_t) (x3 + 32768)) >> 15;
  b7 = ((uint32_t) up - b3) * (50000 >> _oss);
  p = b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;
  x1 = (p >> 8) * (p >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * p) >> 16;
  *_TruePressure = p + ((x1 + x2 + 3791) >> 4);
}

void BMP085::dumpRegisters() {
  int ValidRegisterAddr[]={0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,0xF6,0xF7,0xF8,0xF9}; 
  byte _b, i, totregisters = sizeof(ValidRegisterAddr)/sizeof(int *);
  Serial.println("---dump start---");
  Serial.println("Register address|Resgister data");
  Serial.println("hex dec | 76543210 dec hex");
  for (i=0;i<totregisters;i++){    
    Serial.print("0x");
    Serial.print(ValidRegisterAddr[i], HEX);
    Serial.print(" ");
    Serial.print(ValidRegisterAddr[i], DEC);
    Serial.print(" |");
    readmem(ValidRegisterAddr[i], 1, &_b);
    Serial.print("b");
    print_bits(_b);
    Serial.print(" ");
    Serial.print(_b,DEC);
    Serial.print(" 0x");
    Serial.print(_b,HEX);
    Serial.println("");    
  }
  Serial.println("---dump end---");
}


void BMP085::dumpCalData() {
  Serial.println("---cal data start---");
  Serial.print("ac1:");
  Serial.println(ac1,DEC);
  Serial.print("ac2:");
  Serial.println(ac2,DEC);
  Serial.print("ac3:");
  Serial.println(ac3,DEC);
  Serial.print("ac4:");
  Serial.println(ac4,DEC);
  Serial.print("ac5:");
  Serial.println(ac5,DEC);
  Serial.print("ac6:");
  Serial.println(ac6,DEC); 
  Serial.print("b1:");
  Serial.println(b1,DEC);
  Serial.print("b2:");
  Serial.println(b2,DEC); 
  Serial.print("mb:");
  Serial.println(mb,DEC);
  Serial.print("mc:");
  Serial.println(mc,DEC);
  Serial.print("md:");
  Serial.println(md,DEC);
  Serial.println("---cal data end---");
}

//PRIVATE methods

void BMP085::getCalData() {
  readmem(CAL_AC1, 2, _buff);
  ac1 = ((int)_buff[0] <<8 | ((int)_buff[1]));
  readmem(CAL_AC2, 2, _buff);
  ac2 = ((int)_buff[0] <<8 | ((int)_buff[1]));
  readmem(CAL_AC3, 2, _buff);
  ac3 = ((int)_buff[0] <<8 | ((int)_buff[1]));
  readmem(CAL_AC4, 2, _buff);
  ac4 = ((int)_buff[0] <<8 | ((int)_buff[1]));
  readmem(CAL_AC5, 2, _buff);
  ac5 = ((int)_buff[0] <<8 | ((int)_buff[1]));
  readmem(CAL_AC6, 2, _buff);
  ac6 = ((int)_buff[0] <<8 | ((int)_buff[1])); 
  readmem(CAL_B1, 2, _buff);
  b1 = ((int)_buff[0] <<8 | ((int)_buff[1])); 
  readmem(CAL_B2, 2, _buff);
  b2 = ((int)_buff[0] <<8 | ((int)_buff[1])); 
  readmem(CAL_MB, 2, _buff);
  mb = ((int)_buff[0] <<8 | ((int)_buff[1]));
  readmem(CAL_MC, 2, _buff);
  mc = ((int)_buff[0] <<8 | ((int)_buff[1]));
  readmem(CAL_MD, 2, _buff);
  md = ((int)_buff[0] <<8 | ((int)_buff[1])); 
}

void BMP085::writemem(byte _addr, byte _val) {
  Wire.beginTransmission(_dev_address);  // start transmission to device 
  Wire.send(_addr);                // send register address
  Wire.send(_val);                // send value to write
  Wire.endTransmission();         // end transmission
}

void BMP085::readmem(byte _addr, int _nbytes, byte __buff[]) {
  Wire.beginTransmission(_dev_address);  // start transmission to device 
  Wire.send(_addr);                  // sends register address to read from
  Wire.endTransmission();             // end transmission
  Wire.beginTransmission(_dev_address);   // start transmission to device 
  Wire.requestFrom(_dev_address, _nbytes);// send data n-bytes read
 
  byte i = 0; 
  while (Wire.available()) {    
    __buff[i] = Wire.receive();        // receive DATA
    i++;
  }
  Wire.endTransmission();         // end transmission
}


void print_bits(byte val){
  int i;
  for(i=7; i>=0; i--) 
    Serial.print(val >> i & 1, BIN);
}
/* void print_unit16(uint16_t val){
  int i;
  for(i=15; i>=0; i--) 
    Serial.print(val >> i & 1, BIN);
} 
*/