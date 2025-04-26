#ifndef APP_LED_H
#define APP_LED_H

#include <jendefs.h>
#include "ZTimer.h"

#ifdef DEBUG_LED
#define TRACE_LED TRUE
#else
#define TRACE_LED FALSE
#endif

#define LED_BLINK_INTERVAL ZTIMER_TIME_MSEC(100)

typedef struct
{
    uint32 u32Mask;
    uint8 u8Cycles;
    bool_t bIsOn;
} ts_BlinkState;

PUBLIC void APP_vSetupLeds(void);
PUBLIC void APP_vBlinkLed(uint32 u32LedMask, uint8 u8BlinkCount);

#endif /* APP_LED_H */
