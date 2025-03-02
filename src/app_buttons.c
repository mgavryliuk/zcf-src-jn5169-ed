#include <jendefs.h>
#include "dbg.h"
#include "portmacro.h"
#include "pwrm.h"

#include "app_buttons.h"
#include "app_common.h"
#include "app_led.h"

#define APP_BUTTON_DIO 1
#define APP_BUTTON_DIO_MASK 1
#define APP_BUTTONS_DIO_MASK (1 << APP_BUTTON_DIO)

PRIVATE uint8 keepAlive = 10;
PRIVATE pwrm_tsWakeTimerEvent wakeStruct;


PUBLIC void vISR_SystemController(void)
{
    uint8 u8WakeInt = u8AHI_WakeTimerFiredStatus();

    // if(u32AHI_DioWakeStatus() & APP_BUTTONS_DIO_MASK)
    // {
    //     DBG_vPrintf(TRACE_APP, "Button interrupt\n");
    //     vAHI_DioInterruptEnable(0, APP_BUTTONS_DIO_MASK);
    // }

    if(u8WakeInt & E_AHI_WAKE_TIMER_MASK_1)
    {
        /* wake timer interrupt got us here */
        DBG_vPrintf(TRACE_APP, "APP: Wake Timer 1 Interrupt\n");
        APP_vBlinkLed();
        PWRM_vWakeInterruptCallback();
        APP_vScheduleActivity();
    }
}

PUBLIC void APP_vConfigureButtons() 
{
    DBG_vPrintf(TRACE_APP, "APP: Configuring buttons\n");
    // u32AHI_DioWakeStatus();
    // vAHI_DioSetDirection(APP_BUTTONS_DIO_MASK, 0);
    // DBG_vPrintf(TRACE_APP, "Going to sleep: Buttons:%08x Mask:%08x\n", u32AHI_DioReadInput() & APP_BUTTONS_DIO_MASK, APP_BUTTONS_DIO_MASK);
    // vAHI_DioWakeEdge(0, APP_BUTTONS_DIO_MASK);
    // vAHI_DioWakeEnable(APP_BUTTONS_DIO_MASK, 0);
}


PUBLIC void APP_vScheduleActivity(void) {
    uint8 u8Status;
    u8Status = PWRM_eScheduleActivity(&wakeStruct, keepAlive * 1000 * 32, APP_vWakeCallBack);
    DBG_vPrintf(TRACE_APP, "APP: PWRM_eScheduleActivity status: %d\n", u8Status);
}

PUBLIC void APP_vWakeCallBack(void)
{
    DBG_vPrintf(TRACE_APP, "APP: wake callback triggered\n");
}
