#pragma once
// Arduino ESP32 2.x for ESP32-C3: when ARDUINO_USB_CDC_ON_BOOT=1,
// HardwareSerial.h removes the 'Serial' extern to avoid conflict, but
// Arduino.h doesn't always pull in HWCDC.h which provides the replacement
// alias (#define Serial USBSerial).  Include it explicitly here.
#if ARDUINO_USB_CDC_ON_BOOT
#include "HWCDC.h"
#endif
