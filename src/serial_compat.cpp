// Provide a weak definition of Serial so the linker can resolve it even when
// the board's sdkconfig has ARDUINO_USB_CDC_ON_BOOT disabled in the framework's
// own HWCDC.cpp (which gates its definition behind that same flag).
// __attribute__((weak)) means: if the framework already provides a strong
// HWCDC Serial, the linker uses that; this definition only fills the gap.
#if ARDUINO_USB_CDC_ON_BOOT
#include "HWCDC.h"
HWCDC Serial __attribute__((weak));
#endif
