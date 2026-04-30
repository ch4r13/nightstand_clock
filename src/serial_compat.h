#pragma once
// Diagnostic: abort with a clear message if the flag isn't reaching us
#if !ARDUINO_USB_CDC_ON_BOOT
#error "serial_compat.h: ARDUINO_USB_CDC_ON_BOOT is 0 or not defined here"
#endif
#include "HWCDC.h"
