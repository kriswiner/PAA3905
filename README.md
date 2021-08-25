# PAA3905

 This sketch is for the PAA3905 optical flow sensor. This sensor offers two sensitivities:
 standard detection and an enhanced detection. The sensor can automatically switch between bright, 
 low light, and super low light conditions. Bright and low light modes work at 60 and 30 lux, respectively,
 where the frame rate is 126 frames per second. The super low light mode is for light conditions below 5 lux 
 and is limited to 50 frames per second. The sensor uses typicall 3.5 mA in operation and has a 12 uA shutdown mode.
 Lastly, the sensor can operate in a raw data mode producing 35 x 35 pixel images from the sensor at a frame rate of
 ~15 Hz. This makes the PAA3905 an inexpensive, low-resolution, infrared-sensitive video camera.
 
 ![image]()
 
 I am using the STM32L432 [Ladybug](https://www.tindie.com/products/tleracorp/ladybug-stm32l432-development-board/) development board for testing. The PAA3905 breakout board [design](https://oshpark.com/shared_projects/lCUt7xVA) is available at OSH Park.
