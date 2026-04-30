#pragma once
// On ESP32-C3 Arduino 2.x with ARDUINO_USB_CDC_ON_BOOT=1:
//   - HardwareSerial.h removes the 'Serial' HardwareSerial extern
//   - HWCDC.h provides 'USBSerial' (HWCDC instance) but in this framework
//     version it does NOT alias it to 'Serial'
// Add the alias manually so all code can use 'Serial' as usual.
#if ARDUINO_USB_CDC_ON_BOOT
#include "HWCDC.h"
#ifndef Serial
#define Serial USBSerial
#endif
#endif
