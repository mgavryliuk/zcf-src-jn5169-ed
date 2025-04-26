#include <jendefs.h>

#include "dbg.h"
#include "AppHardwareApi.h"

#include "device_config.h"
#include "app_resources.h"
#include "app_led.h"
#include "app_main.h"

#define MAX_BLINKS (5)

PRIVATE ts_BlinkState sBlinkStates[MAX_BLINKS];

PUBLIC void APP_vSetupLeds(void)
{
    DBG_vPrintf(TRACE_LED, "LED: Configure control mask: %08x\n", sDeviceConfig.sDioConfig.u32LedMask);
    vAHI_DioSetDirection(0, sDeviceConfig.sDioConfig.u32LedMask);
    vAHI_DioSetOutput(sDeviceConfig.sDioConfig.u32LedMask, 0);
}

PUBLIC void APP_vBlinkLed(uint32 u32LedMask, uint8 u8BlinkCount)
{
    uint8 i;
    for (i = 0; i < MAX_BLINKS; i++)
    {
        if (sBlinkStates[i].u32Mask & u32LedMask)
        {
            DBG_vPrintf(TRACE_LED, "LED: Blink already in progress for mask: %08x\n", u32LedMask);
            return;
        }
    }

    DBG_vPrintf(TRACE_LED, "LED: Starting Blink for mask: %08x\n", u32LedMask);
    for (i = 0; i < MAX_BLINKS; i++)
    {
        if (sBlinkStates[i].u32Mask == 0)
        {
            sBlinkStates[i].u32Mask = u32LedMask;
            sBlinkStates[i].u8Cycles = u8BlinkCount << 1;
            sBlinkStates[i].bIsOn = FALSE;
            break;
        }
    }

    ZTIMER_teState eZtimerState = ZTIMER_eGetState(u8TimerLedBlink);
    if (eZtimerState == E_ZTIMER_STATE_RUNNING)
    {
        DBG_vPrintf(TRACE_LED, "LED: Blink Timer is already running\n");
        return;
    }
    DBG_vPrintf(TRACE_LED, "LED: Starting Blink Timer\n");
    ZTIMER_eStart(u8TimerLedBlink, LED_BLINK_INTERVAL);
}

PUBLIC void APP_cbTimerLedBlink(void *pvParam)
{
    DBG_vPrintf(TRACE_LED, "LED: Blink Timer Callback - Enter\n");
    uint32 u32TurnOnMask = 0;
    uint32 u32TurnOffMask = 0;
    uint8 i;

    for (i = 0; i < MAX_BLINKS; i++)
    {
        if (sBlinkStates[i].u8Cycles > 0)
        {
            if (sBlinkStates[i].bIsOn)
            {
                u32TurnOffMask |= sBlinkStates[i].u32Mask;
                sBlinkStates[i].bIsOn = FALSE;
            }
            else
            {
                u32TurnOnMask |= sBlinkStates[i].u32Mask;
                sBlinkStates[i].bIsOn = TRUE;
            }
            sBlinkStates[i].u8Cycles--;

            if (sBlinkStates[i].u8Cycles == 0)
            {
                sBlinkStates[i].u32Mask = 0;
            }
        }
    }

    if (u32TurnOffMask || u32TurnOnMask)
    {
        vAHI_DioSetOutput(u32TurnOnMask, u32TurnOffMask);
        DBG_vPrintf(TRACE_LED, "LED: Blink Timer Callback - Restart\n");
        ZTIMER_eStart(u8TimerLedBlink, LED_BLINK_INTERVAL);
    }
    else
    {
        DBG_vPrintf(TRACE_LED, "LED: Blink Timer Callback - Stop\n");
        vAHI_DioSetOutput(sDeviceConfig.sDioConfig.u32LedMask, 0);
    }
}
