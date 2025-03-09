#include <jendefs.h>
#include "dbg.h"
#include "AppHardwareApi.h"

#include "app_common.h"
#include "app_led.h"
#include "app_main.h"

ts_BlinkConfig sBlinkConfigs[] = {
    {APP_LED_LEFT_MASK, 0},
    {APP_LED_RIGHT_MASK, 0},
};

PUBLIC void APP_vSetupLeds(void)
{
    DBG_vPrintf(TRACE_APP, "APP: Configure LEDs control MASK: %08x\n", APP_LEDS_CTRL_MASK);
    vAHI_DioSetDirection(0, APP_LEDS_CTRL_MASK);
}

PUBLIC void APP_vBlinkLed(te_BlinkMode eBlinkMode, uint8 u8Amount)
{
    if (u8Amount <= 0)
    {
        return;
    }

    uint16 u16CallbackCycles = u8Amount << 1;

    switch (eBlinkMode)
    {
    case BLINK_LEFT:
    case BLINK_RIGHT:
        if (sBlinkConfigs[eBlinkMode].u8Amount == 0)
            sBlinkConfigs[eBlinkMode].u8Amount = u16CallbackCycles;
        break;

    case BLINK_BOTH:
        vAHI_DioSetOutput(0, APP_LEDS_CTRL_MASK);
        sBlinkConfigs[BLINK_LEFT].u8Amount = u16CallbackCycles;
        sBlinkConfigs[BLINK_RIGHT].u8Amount = u16CallbackCycles;
        break;

    default:
        break;
    }

    ZTIMER_teState eZtimerState = ZTIMER_eGetState(u8LedBlinkTimer);
    if (eZtimerState == E_ZTIMER_STATE_RUNNING || eZtimerState == E_ZTIMER_STATE_EXPIRED)
    {
        DBG_vPrintf(TRACE_APP, "APP: Blink Led timer is already running\n");
        return;
    }

    bool bNeedToStart = FALSE;
    uint8 i;
    for (i = 0; i < sizeof(sBlinkConfigs) / sizeof(ts_BlinkConfig); i++)
    {
        if (sBlinkConfigs[i].u8Amount > 0)
        {
            bNeedToStart = TRUE;
            break;
        }
    }

    if (bNeedToStart)
    {
        DBG_vPrintf(TRACE_APP, "APP: Starting blink Led timer\n");
        ZTIMER_eStart(u8LedBlinkTimer, APP_LED_BLINK_INTERVAL);
    }
}

PUBLIC void APP_cbBlinkLed(void *pvParam)
{
    DBG_vPrintf(TRACE_APP, "APP: APP_cbBlinkLed - Enter\n");
    uint32 u32ToggleMask = 0;
    bool bNeedAnotherCycle = FALSE;
    uint8 i;
    for (i = 0; i < sizeof(sBlinkConfigs) / sizeof(ts_BlinkConfig); i++)
    {
        if (sBlinkConfigs[i].u8Amount > 0)
        {
            u32ToggleMask |= sBlinkConfigs[i].u32Mask;
            sBlinkConfigs[i].u8Amount--;
            if (sBlinkConfigs[i].u8Amount > 0)
            {
                bNeedAnotherCycle = TRUE;
            }
        }
    }

    if (u32ToggleMask > 0)
    {
        uint32 u32CurrentState = u32AHI_DioReadInput();
        vAHI_DioSetOutput(u32CurrentState ^ u32ToggleMask, u32CurrentState & u32ToggleMask);
    }

    switch (bNeedAnotherCycle)
    {
    case TRUE:
        DBG_vPrintf(TRACE_APP, "APP: APP_cbBlinkLed - Restart timer\n");
        ZTIMER_eStart(u8LedBlinkTimer, APP_LED_BLINK_INTERVAL);
        break;

    default:
        DBG_vPrintf(TRACE_APP, "APP: APP_cbBlinkLed - Stop timer\n");
        vAHI_DioSetOutput(0, APP_LEDS_CTRL_MASK);
        ZTIMER_eStop(u8LedBlinkTimer);
        break;
    }
}
