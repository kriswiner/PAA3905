# PAA3905

 This sketch is for the PAA3905 optical flow sensor. This sensor offers two sensitivities:
 standard detection and enhanced detection. The sensor can automatically switch between bright (>60 lux), 
 low light (>30 lux), and super low light (> 5 lux) conditions. Bright and low light modes work at 126 frames per second. 
 The super low light mode is limited to 50 frames per second. The sensor uses typically 3.5 mA in operation and has a 12 uA shutdown mode.
 The sensor can operate in raw data (frame grab) mode producing 35 x 35 pixel images from the sensor at a frame rate of
 ~15 Hz. This makes the PAA3905 an inexpensive, low-resolution, infrared-sensitive video camera.
 
 ![image](https://user-images.githubusercontent.com/6698410/130867936-83a9b875-73ed-4f13-b8b0-949b0c427e26.jpg)
 
 I am using the STM32L432 [Ladybug](https://www.tindie.com/products/tleracorp/ladybug-stm32l432-development-board/) development board for testing. The PAA3905 breakout board [design](https://oshpark.com/shared_projects/lCUt7xVA) is available at OSH Park.
