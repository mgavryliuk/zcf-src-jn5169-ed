#ifndef ISR_SYSTEM_H
#define ISR_SYSTEM_H

#include <jendefs.h>
#include "device_config.h"

#ifdef DEBUG_ISR_SYSTEM
#define TRACE_ISR_SYSTEM TRUE
#else
#define TRACE_ISR_SYSTEM FALSE
#endif

#ifndef BTN_CTRL_MASK
#error "BTN_CTRL_MASK is not defined. Make sure your device configuration defines it."
#endif

#endif /* ISR_SYSTEM_H */
