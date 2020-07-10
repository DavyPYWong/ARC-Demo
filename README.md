# ARC Demo Day
# Introduction

* Here is The Quarantine Buddy, a measuring tool for respiratory disease.
* Based on body temperature, breathing sound, environment temperature and air quality.

# HW/SW Setup
## Devices
* [ARC IoTDK](https://embarc.org/embarc_osp/doc/build/html/board/iotdk.html)
* [GYMCU680](https://drive.google.com/open?id=1Q55L0tDwbszPV7ZY3uVGkPOP02z2hccC)
* [MLX90615](https://drive.google.com/open?id=1geWPibnqc0NhDlHxl0I-lTOQClOoQZ-Q)
* HD44780 (LCD)
* INMP441 (Voice regconition)
* LM386 (Stethoscope)
## Connection
* I2C: MLX90615, HD44780 (LCD)
* I2S: INMP441
* UART: GYMCU680

# User manual
* Install MetaWare to begin, basic EM library is required.
* To compile the firmware
  ```C
  	gmake BOARD=iotdk TOOLCHAIN=mw gui
  ```
