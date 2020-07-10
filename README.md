# ARC Demo Day
## Introduction

* Here is The Quarantine Buddy, a measuring tool for body temperature & breathing sound.

## HW/SW Setup

* [ARC IoTDK](https://embarc.org/embarc_osp/doc/build/html/board/iotdk.html)
* [GYMCU680](https://drive.google.com/open?id=1Q55L0tDwbszPV7ZY3uVGkPOP02z2hccC)
* [MLX90615](https://drive.google.com/open?id=1geWPibnqc0NhDlHxl0I-lTOQClOoQZ-Q)
* HD44780
* INMP441

## User manual

* Install MetaWare to begin, basic EM library is required.
* To compile the firmware
  ```C
  	gmake BOARD=iotdk TOOLCHAIN=mw gui
  ```
