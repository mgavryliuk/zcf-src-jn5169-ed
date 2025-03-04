#include <jendefs.h>
#include "dbg.h"
#include "dbg_uart.h"
#include "string.h"
#include "portmacro.h"
#include "pwrm.h"

#include "app_entrypoint.h"
#include "app_common.h"
#include "app_button.h"
#include "app_led.h"
#include "app_main.h"

PRIVATE uint8 keepAlive = 10;
PRIVATE pwrm_tsWakeTimerEvent wakeStruct;

static PWRM_DECLARE_CALLBACK_DESCRIPTOR(PreSleep);
static PWRM_DECLARE_CALLBACK_DESCRIPTOR(Wakeup);

void vAppRegisterPWRMCallbacks(void){
    PWRM_vRegisterPreSleepCallback(PreSleep);
    PWRM_vRegisterWakeupCallback(Wakeup);
}

PUBLIC void vISR_SystemController(void)
{
    uint8 u8WakeInt = u8AHI_WakeTimerFiredStatus();

    APP_vHandleBtnInterrupts();
    if(u8WakeInt & E_AHI_WAKE_TIMER_MASK_1)
    {
        /* wake timer interrupt got us here */
        DBG_vPrintf(TRACE_APP, "APP: Wake Timer 1 Interrupt\n");
        APP_vBlinkLed(BLINK_BOTH, 5);
        PWRM_vWakeInterruptCallback();
        APP_vScheduleActivity();
    }
}

PUBLIC void waitForXTAL(void) {
    while (bAHI_GetClkSource() == TRUE);
    bAHI_SetClockRate(3);
}

PUBLIC void APP_vSetUpHardware(void)
{
    TARGET_INITIALISE();
    SET_IPL(0);
    portENABLE_INTERRUPTS();
}

PUBLIC void vAppMain(void)
{
    waitForXTAL();
    
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
    DBG_vPrintf(TRACE_APP, "vAppMain: Start\n");
    APP_vSetUpHardware();
    APP_vInitResources();
    APP_vInitialise();
    BDB_vStart();
    // Do this only on specific button click
    // BDB_eNsStartNwkSteering();
    APP_vScheduleActivity();
    DBG_vPrintf(TRACE_APP, "vAppMain: Entering main loop...\n");
    APP_vMainLoop();
}

PWRM_CALLBACK(PreSleep)
{
    DBG_vPrintf(TRACE_APP, "PWRM_CALLBACK(PreSleep): Callback triggered\n");
    vAppApiSaveMacSettings();
    ZTIMER_vSleep();
    DBG_vUartFlush();
    vAHI_UartDisable(E_AHI_UART_0);
}

PWRM_CALLBACK(Wakeup)
{
    waitForXTAL();
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
    DBG_vPrintf(TRACE_APP, "\nPWRM_CALLBACK(Wakeup): Callback triggered\n");
    vMAC_RestoreSettings();
    APP_vSetUpHardware();
    ZTIMER_vWake();
    /* Activate the SleepTask, that would start the SW timer and polling would continue * */
    // APP_vStartUpHW();
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
