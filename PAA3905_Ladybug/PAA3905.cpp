/* PAA3905 Optical Flow Sensor
 * Copyright (c) 2021 Tlera Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include "PAA3905.h"
#include <SPI.h>

PAA3905::PAA3905(uint8_t cspin)
  : _cs(cspin)
{ }


boolean PAA3905::begin(void) 
{
  // Setup SPI port
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE3)); // 2 MHz max SPI clock frequency

  // Make sure the SPI bus is reset
  digitalWrite(_cs, HIGH);
  delay(1);
  digitalWrite(_cs, LOW);
  delay(1);
  digitalWrite(_cs, HIGH);
  delay(1);

  SPI.endTransaction();
}


void PAA3905::setMode(uint8_t mode, uint8_t autoMode) 
{
 _mode = mode;
 reset();
 initRegisters(mode);
 
 if(autoMode == autoMode012){
  writeByteDelay(0x7F, 0x08);
  writeByteDelay(0x68, 0x02);
  writeByteDelay(0x7F, 0x00);
 }
 else
 {
  writeByteDelay(0x7F, 0x08);
  writeByteDelay(0x68, 0x01);
  writeByteDelay(0x7F, 0x00);
 }
}


uint8_t PAA3905::getMode() 
{
 uint8_t _mode = readByte(PAA3905_OBSERVATION);
 _mode &= 0xC0;  // only look at bits 6 and 7 for mode
 return _mode;
}


void PAA3905::setResolution(uint8_t res) 
{
 writeByte(PAA3905_RESOLUTION, res);
}


uint8_t PAA3905::getResolution() 
{
 uint8_t temp = readByte(PAA3905_RESOLUTION);
 return temp;
}


void PAA3905::setOrientation(uint8_t orient) 
{
 writeByte(PAA3905_ORIENTATION, orient);
}


uint8_t PAA3905::getOrientation() 
{
 uint8_t temp = readByte(PAA3905_ORIENTATION);
 return temp;
}



void PAA3905::initRegisters(uint8_t mode)
{
  switch(mode)
  {
  case 0: // standard detection
  standardDetection();
  break;
  
  case 1: // enhanced detection
  enhancedDetection();
  break;
  }
}


boolean PAA3905::checkID()
{
  // check device ID
  uint8_t product_ID = readByte(PAA3905_PRODUCT_ID);
  uint8_t revision_ID = readByte(PAA3905_REVISION_ID);
  uint8_t inverse_product_ID = readByte(PAA3905_INVERSE_PRODUCT_ID);

  Serial.print("Product ID = 0x"); Serial.print(product_ID, HEX); Serial.println(" should be 0xA2");
  Serial.print("Revision ID = 0x0"); Serial.println(revision_ID, HEX); 
  Serial.print("Inverse Product ID = 0x"); Serial.print(inverse_product_ID, HEX); Serial.println(" should be 0x5D"); 

  if (product_ID != 0xA2 && inverse_product_ID != 0x5D) return false;
  else return true;
}


void PAA3905::reset()
{
  // Power up reset
  writeByte(PAA3905_POWER_UP_RESET, 0x5A);
  delay(1); 
  // Read the motion registers one time to clear
  for (uint8_t ii = 0; ii < 5; ii++)
  {
    readByte(PAA3905_MOTION + ii);
    delayMicroseconds(2);
  }
}


void PAA3905::shutdown()
{
  // Enter shutdown mode
  writeByte(PAA3905_SHUTDOWN, 0xB6);
}


void PAA3905::powerup()
{ // exit from shutdown mode
  digitalWrite(_cs, HIGH);
  delay(1);
  digitalWrite(_cs, LOW); // reset the SPI port
  delay(1);
  // Wakeup
  writeByte(PAA3905_SHUTDOWN, 0xC7); // exit shutdown mode
  delay(1);
  writeByte(PAA3905_SHUTDOWN, 0x00); // clear shutdown register
  delay(1);
  // Read the motion registers one time to clear
  for (uint8_t ii = 0; ii < 5; ii++)
  {
    readByte(0x02 + ii);
    delayMicroseconds(2);
  }
}


uint8_t PAA3905::status()
{
  uint8_t temp = readByte(PAA3905_MOTION); // clears motion interrupt
  return temp;
}


void PAA3905::readMotionCount(int16_t *deltaX, int16_t *deltaY, uint8_t *SQUAL, uint32_t *Shutter)
{
  *deltaX =  ((int16_t) readByte(PAA3905_DELTA_X_H) << 8) | readByte(PAA3905_DELTA_X_L);
  *deltaY =  ((int16_t) readByte(PAA3905_DELTA_Y_H) << 8) | readByte(PAA3905_DELTA_X_L);
  *SQUAL =              readByte(PAA3905_SQUAL);
  *Shutter = ((uint32_t)readByte(PAA3905_SHUTTER_H) << 16) | ((uint32_t)readByte(PAA3905_SHUTTER_M) << 8) | readByte(PAA3905_SHUTTER_L);
}


void PAA3905::readBurstMode(uint8_t * dataArray)
{
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE3));
  
  digitalWrite(_cs, LOW);
  delayMicroseconds(1);
  
  SPI.transfer(PAA3905_MOTION_BURST); // start burst mode
  digitalWrite(MOSI, HIGH); // hold MOSI high during burst read
  delayMicroseconds(2);
  
  for(uint8_t ii = 0; ii < 14; ii++)
  {
    dataArray[ii] = SPI.transfer(0);
  }
  digitalWrite(MOSI, LOW); // return MOSI to LOW
  digitalWrite(_cs, HIGH);
  delayMicroseconds(1);
  
  SPI.endTransaction();
}


void PAA3905::writeByte(uint8_t reg, uint8_t value) 
{
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE3));
  digitalWrite(_cs, LOW);
  delayMicroseconds(1);
  
  SPI.transfer(reg | 0x80);
  delayMicroseconds(10);
  SPI.transfer(value);
  delayMicroseconds(1);
  
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}


void PAA3905::writeByteDelay(uint8_t reg, uint8_t value)
{
  writeByte(reg, value);
  delayMicroseconds(11);
}


uint8_t PAA3905::readByte(uint8_t reg) 
{
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE3));
  digitalWrite(_cs, LOW);
  delayMicroseconds(1);
  
  SPI.transfer(reg & 0x7F);
  delayMicroseconds(2);
  
  uint8_t temp = SPI.transfer(0);
  delayMicroseconds(1);
  
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
  
  return temp;
}


void PAA3905::enterFrameCaptureMode()
{
  setMode(standardDetectionMode, autoMode01); // make sure not in superlowlight mode for frame capture
  
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x67, 0x25);
  writeByteDelay(0x55, 0x20);
  writeByteDelay(0x7F, 0x13);
  writeByteDelay(0x42, 0x01);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x0F, 0x11);
  writeByteDelay(0x0F, 0x13);
  writeByteDelay(0x0F, 0x11);
}

  
uint8_t PAA3905::captureFrame(uint8_t * frameArray)
{  
  uint8_t tempStatus = 0;
  while( !(tempStatus & 0x01) ) {
    tempStatus = readByte(PAA3905_RAWDATA_GRAB_STATUS); // wait for grab status bit 0 to equal 1
    } 
    
  writeByteDelay(PAA3905_RAWDATA_GRAB, 0xFF); // start frame capture mode
  
  for(uint8_t ii = 0; ii < 35; ii++)
  {
    for(uint8_t jj = 0; jj < 35; jj++)
    {
      frameArray[ii*35 + jj] = readByte(PAA3905_RAWDATA_GRAB); // read the 1225 data into array
    }
  }
}


void PAA3905::exitFrameCaptureMode()
{
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x55, 0x00);
  writeByteDelay(0x7F, 0x13);
  writeByteDelay(0x42, 0x00);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x67, 0xA5);
}


// Performance optimization registers for the three different modes
void PAA3905::standardDetection() // default
{
  writeByteDelay(0x7F, 0x00); // 1
  writeByteDelay(0x51, 0xFF);
  writeByteDelay(0x4E, 0x2A);
  writeByteDelay(0x66, 0x3E);
  writeByteDelay(0x7F, 0x14);
  writeByteDelay(0x7E, 0x71);
  writeByteDelay(0x55, 0x00);
  writeByteDelay(0x59, 0x00);
  writeByteDelay(0x6F, 0x2C);
  writeByteDelay(0x7F, 0x05); // 10

  writeByteDelay(0x4D, 0xAC); // 11
  writeByteDelay(0x4E, 0x32);
  writeByteDelay(0x7F, 0x09);
  writeByteDelay(0x5C, 0xAF);
  writeByteDelay(0x5F, 0xAF);
  writeByteDelay(0x70, 0x08);
  writeByteDelay(0x71, 0x04);
  writeByteDelay(0x72, 0x06);
  writeByteDelay(0x74, 0x3C);
  writeByteDelay(0x75, 0x28); // 20
  
  writeByteDelay(0x76, 0x20); //  21
  writeByteDelay(0x4E, 0xBF);
  writeByteDelay(0x7F, 0x03);
  writeByteDelay(0x64, 0x14);
  writeByteDelay(0x65, 0x0A);
  writeByteDelay(0x66, 0x10);
  writeByteDelay(0x55, 0x3C);
  writeByteDelay(0x56, 0x28);
  writeByteDelay(0x57, 0x20);
  writeByteDelay(0x4A, 0x2D); // 30
  
  writeByteDelay(0x4B, 0x2D); // 31
  writeByteDelay(0x4E, 0x4B);
  writeByteDelay(0x69, 0xFA);
  writeByteDelay(0x7F, 0x05);
  writeByteDelay(0x69, 0x1F);
  writeByteDelay(0x47, 0x1F);
  writeByteDelay(0x48, 0x0C);
  writeByteDelay(0x5A, 0x20);
  writeByteDelay(0x75, 0x0F);
  writeByteDelay(0x4A, 0x0F);  // 40
  
  writeByteDelay(0x42, 0x02);  // 41
  writeByteDelay(0x45, 0x03);
  writeByteDelay(0x65, 0x00);
  writeByteDelay(0x67, 0x76);
  writeByteDelay(0x68, 0x76);
  writeByteDelay(0x6A, 0xC5);
  writeByteDelay(0x43, 0x00);
  writeByteDelay(0x7F, 0x06);
  writeByteDelay(0x4A, 0x18);
  writeByteDelay(0x4B, 0x0C); // 50

  writeByteDelay(0x4C, 0x0C); // 51 
  writeByteDelay(0x4D, 0x0C);  
  writeByteDelay(0x46, 0x0A);
  writeByteDelay(0x59, 0xCD);
  writeByteDelay(0x7F, 0x0A);
  writeByteDelay(0x4A, 0x2A);
  writeByteDelay(0x48, 0x96);
  writeByteDelay(0x52, 0xB4);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x5B, 0xA0); // 60
}


 void PAA3905::enhancedDetection()    
{
  writeByteDelay(0x7F, 0x00); // 1
  writeByteDelay(0x51, 0xFF);
  writeByteDelay(0x4E, 0x2A);
  writeByteDelay(0x66, 0x26);
  writeByteDelay(0x7F, 0x14);
  writeByteDelay(0x7E, 0x71);
  writeByteDelay(0x55, 0x00);
  writeByteDelay(0x59, 0x00);
  writeByteDelay(0x6F, 0x2C);
  writeByteDelay(0x7F, 0x05); // 10

  writeByteDelay(0x4D, 0xAC); // 11
  writeByteDelay(0x4E, 0x65);
  writeByteDelay(0x7F, 0x09);
  writeByteDelay(0x5C, 0xAF);
  writeByteDelay(0x5F, 0xAF);
  writeByteDelay(0x70, 0x00);
  writeByteDelay(0x71, 0x00);
  writeByteDelay(0x72, 0x00);
  writeByteDelay(0x74, 0x14);
  writeByteDelay(0x75, 0x14); // 20
  
  writeByteDelay(0x76, 0x06); //  21
  writeByteDelay(0x4E, 0x8F);
  writeByteDelay(0x7F, 0x03);
  writeByteDelay(0x64, 0x00);
  writeByteDelay(0x65, 0x00);
  writeByteDelay(0x66, 0x00);
  writeByteDelay(0x55, 0x14);
  writeByteDelay(0x56, 0x14);
  writeByteDelay(0x57, 0x06);
  writeByteDelay(0x4A, 0x20); // 30
  
  writeByteDelay(0x4B, 0x20); // 31
  writeByteDelay(0x4E, 0x32);
  writeByteDelay(0x69, 0xFE);
  writeByteDelay(0x7F, 0x05);
  writeByteDelay(0x69, 0x14);
  writeByteDelay(0x47, 0x14);
  writeByteDelay(0x48, 0x1C);
  writeByteDelay(0x5A, 0x20);
  writeByteDelay(0x75, 0xE5);
  writeByteDelay(0x4A, 0x05);  // 40
  
  writeByteDelay(0x42, 0x04);  // 41
  writeByteDelay(0x45, 0x03);
  writeByteDelay(0x65, 0x00);
  writeByteDelay(0x67, 0x50);
  writeByteDelay(0x68, 0x50);
  writeByteDelay(0x6A, 0xC5);
  writeByteDelay(0x43, 0x00);
  writeByteDelay(0x7F, 0x06);
  writeByteDelay(0x4A, 0x1E);
  writeByteDelay(0x4B, 0x1E); // 50

  writeByteDelay(0x4C, 0x34C); // 51 
  writeByteDelay(0x4D, 0x34C);  
  writeByteDelay(0x46, 0x32);
  writeByteDelay(0x59, 0x0D);
  writeByteDelay(0x7F, 0x0A);
  writeByteDelay(0x4A, 0x2A);
  writeByteDelay(0x48, 0x96);
  writeByteDelay(0x52, 0xB4);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x5B, 0xA0); // 60
}
