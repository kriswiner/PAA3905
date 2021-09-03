/* 08/25/2021 Copyright Tlera Corporation
 *  
 *  Created by Kris Winer
 *  
 This sketch configures and reads data from the PAA3905 optical flow sensor. 
 The sensor uses standard SPI for communications at a maximum serial port speed of 2 MHz. The sensor data ready
 is signaled by an active LOW interrupt.
 
 This sensor offers two sensitivities: standard detection and enhanced detection for rough terrain 
 at > 15 cm height. The sensor can automatically switch between bright (>60 lux), low light (>30 lux), 
 and super low light (> 5 lux) conditions. Bright and low light modes work at 126 frames per second. The super 
 low light mode is limited to 50 frames per second. 
 
 The sensor uses typically 3.5 mA in operation and has a 12 uA shutdown mode The sensor can operate 
 in navigate mode producing delta X and Y values which are proportional to lateral velocity. 
 The limiting speed is determined the maximum 7.2 rads/sec flow rate and by distance to the measurement 
 surface; 80 mm is the minimum measurement distance. So at 80 mm the maxium speed is 0.576 m/s (1.25 mph), 
 at 2 meter distance (~drone height) the maximum speed is 14.4 m/s (32 mph), etc. 
 
 The sensor can also operate in raw data (frame grab) mode producing 35 x 35 pixel images from the 
 sensor at a frame rate of ~15 Hz. This makes the PAA3905 an inexpensive, low-resolution, infrared-sensitive 
 video camera.
 
 The sketch uses default SPI pins on the Ladybug development board.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 Library may be used freely and without limit with proper attribution including this entire preamble.
*/
#include "PAA3905.h"
#include <Adafruit_GFX.h>    // Core graphics library, install from Arduino IDE board manager
#include <Adafruit_ST7735.h> // Hardware-specific library, install from Arduino IDE board manager
#include "SPI.h"
#include "ColorDisplay.h"

// Pin definitions
#define CSPIN  10  // chip select for SPI 
#define MOSI   11  // SPI MOSI pin on Dragonfly required for frame capture
#define MOT    31  // use as data ready interrupt
#define myLed  25  // red led

// Dragonfly development board connections for display
#define cs   8   // CS & DC can use pins 2, 6, 9, 10, 15, 20, 21, 22, 23
#define dc   7   // but certain pairs must NOT be used: 2+10, 6+9, 20+23, 21+22
#define rst  9   // RST can use any pin
#define sdcs 1   // CS for SD card, can use any pin

// PAA3905 configuration
uint8_t mode = standardDetectionMode; // mode choices are standardDetectionMode (default) or enhancedDetectionMode
uint8_t autoMode = autoMode01;        // choices are autoMode01 and autoMode012 (includes superLowLight mode)
uint8_t pixelRes = 0x2A;              // choices are from 0x00 to 0xFF
float resolution;                     // calculated (approximate) resolution (counts per delta) per meter of height
uint8_t orientation, orient = 0x00;   // for X invert 0x80, for Y invert 0x40, for X and Y swap, 0x20, for all three 0XE0 (bits 5 - 7 only)
int16_t deltaX = 0, deltaY = 0;
uint32_t Shutter = 0;
volatile bool motionDetect = false;
uint8_t statusCheck = 0;
uint8_t frameArray[1225], dataArray[14], SQUAL, RawDataSum = 0, RawDataMin = 0, RawDataMax = 0;
uint8_t iterations = 0;
uint32_t frameTime = 0;

PAA3905 opticalFlow(CSPIN); // Instantiate PAA3905


// Configure color display
uint16_t color;
uint8_t rgb, red, green, blue;

Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);


void setup() {
  Serial.begin(115200);
  delay(4000);
  Serial.println("Serial enabled!");

  pinMode(MOT, INPUT); // data ready interrupt

  pinMode(myLed, OUTPUT);
  digitalWrite(myLed, HIGH);

  pinMode(sdcs, INPUT_PULLUP);          // don't touch the SD card

  // Configure SPI Flash chip select
  pinMode(CSPIN, OUTPUT);
  digitalWrite(CSPIN, HIGH);

  SPI.begin(); // initiate SPI 
  delay(1000);

  tft.initR(INITR_BLACKTAB);            // initialize a ST7735S chip, black tab
  
  //tft.begin();                        // initialize a ST7735S chip, black tab
  tft.setRotation(3);                   // 0, 2 are portrait mode, 1,3 are landscape mode
  Serial.println("initialize display");


  opticalFlow.begin();  // Prepare SPI port 

  digitalWrite(myLed, LOW);
  
  // Check device ID as a test of SPI communications
  if (!opticalFlow.checkID()) {
  Serial.println("Initialization of the opticalFlow sensor failed");
  while(1) { }
  }

  opticalFlow.reset(); // Reset PAA3905 to return all registers to default before configuring
  
  opticalFlow.setMode(mode, autoMode);         // set modes
  
  opticalFlow.setResolution(pixelRes);         // set resolution fraction of default 0x2A
  resolution = (opticalFlow.getResolution() + 1) * 200.0f/8600.0f; // == 1 if pixelRes == 0x2A
  Serial.print("Resolution is: "); Serial.print(resolution * 11.914f, 1); Serial.println(" CPI per meter height"); Serial.println(" ");
  
  opticalFlow.setOrientation(orient);
  orientation = opticalFlow.getOrientation();
  if(orientation & 0x80) Serial.println("X direction inverted!"); Serial.println(" ");
  if(orientation & 0x40) Serial.println("Y direction inverted!"); Serial.println(" ");
  if(orientation & 0x20) Serial.println("X and Y swapped!"); Serial.println(" ");
 
//  attachInterrupt(MOT, myIntHandler, FALLING); // data ready interrupt active LOW 
  
//  statusCheck = opticalFlow.status();          // clear interrupt before entering main loop

//  opticalFlow.shutdown();                    // enter lowest power mode until ready to use
  /* end of setup */
}

void loop() {

  // Frame capture
//    frameTime = millis();
    opticalFlow.enterFrameCaptureMode();   
    opticalFlow.captureFrame(frameArray);
    opticalFlow.exitFrameCaptureMode(); // exit fram capture mode
//    Serial.print("Frame time = "); Serial.print(millis() - frameTime); Serial.println(" ms"); Serial.println(" ");
/*
    for(uint8_t ii = 0; ii < 35; ii++) // plot the frame data on the serial monitor (TFT display would be better)
      {
        Serial.print(ii); Serial.print(" "); 
        for(uint8_t jj = 0; jj < 35; jj++)
        {
        Serial.print(frameArray[ii*35 + jj]); Serial.print(" ");  
        }
        Serial.println(" ");
    }
        Serial.println(" ");
*/
  for(int y=0; y<35; y++){ //go through all the rows
  for(int x=0; x<35; x++){ //go through all the columns
  
    rgb = frameArray[y+x*35];

    red   = rgb_colors[rgb*3] >> 3;          // keep 5 MS bits
    green = rgb_colors[rgb*3 + 1] >> 2;      // keep 6 MS bits
    blue  = rgb_colors[rgb*3 + 2] >> 3;      // keep 5 MS bits
    color = red << 11 | green << 5 | blue;   // construct rgb565 color for tft display

    tft.fillRect(x*4, y*4, 4, 4, color); // data on 140 x 140 pixels of a 160 x 128 pixel display
  }
  }

   digitalWrite(myLed, LOW); delay(1); digitalWrite(myLed, HIGH);   
    
} // end of main loop


void myIntHandler()
{
  motionDetect = true;
}
