# PAA3905

 These sketches configure and read data from the PAA3905 optical flow sensor. 
 The sensor uses standard SPI for communications at a maximum serial port speed of 2 MHz. The sensor data ready
 is signaled by an active LOW interrupt.
 
 This sensor offers two sensitivities: standard detection and enhanced detection for rough terrain 
 at > 15 cm height. The sensor can automatically switch between bright (>60 lux), low light (>30 lux), 
 and super low light (> 5 lux) conditions. Bright and low light modes work at 126 frames per second. The super 
 low light mode is limited to 50 frames per second. 
 
 The sensor uses typically 3.5 mA in operation and has a 12 uA shutdown mode The sensor can operate 
 in navigate mode producing delta X and Y values which are proportional to lateral velocity. 
 The limiting speed is determined by the maximum 7.2 rads/sec flow rate and by distance to the measurement 
 surface; 80 mm is the minimum measurement distance. So at 80 mm the maximum speed is 0.576 m/s (1.25 mph), 
 at 2 meter distance (~drone height) the maximum speed is 14.4 m/s (32 mph), etc. 

 ![image1](https://user-images.githubusercontent.com/6698410/130867936-83a9b875-73ed-4f13-b8b0-949b0c427e26.jpg)
 
 I am using the STM32L432 [Ladybug](https://www.tindie.com/products/tleracorp/ladybug-stm32l432-development-board/) development board for testing. 

 The sensor can also operate in raw data (frame grab) mode producing 35 x 35 pixel images from the 
 sensor at a frame rate of ~18 Hz. This makes the PAA3905 an inexpensive, low-resolution, infrared-sensitive 
 video camera. I tested this using Adafruit's 160 x 128 pixel rgb TFT [display](https://learn.adafruit.com/1-8-tft-display) 
 on an STM32L476 [Dragonfly](https://www.tindie.com/products/tleracorp/dragonfly-stm32l47696-development-board/) development board. 
 
 In the main loop it takes 56 ms to grab a 35 x 35 pixel (each pixel is 1  byte) frame but the display rate seems more like about ~2 Hz or so. 
 This is due to the fact that I am using the same 2 MHz SPI bus for both the sensor and the display. 
 I assume if I were to run the display from a separate SPI bus at 20 MHz the display speed would keep up with the frame rate. TBD.
 
 I am plotting the data on the TFT display by mapping rgb colors to intensity. Since I am using the Dragonfly with its embedded 16 MByte SPI NOR flash 
 I could also just store the 1225 bytes of data on the flash for later download. I could record ~10 minutes of video at the ~18 Hz frame rate. 
 
 In the image below the PAA3905 is pointed toward a window with my hand in the field of view (basically backlit). 
 In addition to faster display speed, using the PAA3905 as a video camera would benefit from using an led and the led sync pin for scene illumination. I might try this next.
 
 ![image2](https://user-images.githubusercontent.com/6698410/132047219-fcb7dcb0-287c-4db8-a6fe-3b6ec7886a98.jpg)
 
 The PAA3905 breakout board [design](https://oshpark.com/shared_projects/lCUt7xVA) is available at OSH Park. 
 
 Contact [PixArt](https://www.pixart.com/products-comparison/16/Optical_Motion_Tracking) for data sheet and to order the PAA3905 and appropriate lenses.
 
 Breakout board available for sale on [Tindie](https://www.tindie.com/products/onehorse/paa3905-optical-flow-sensor/).
