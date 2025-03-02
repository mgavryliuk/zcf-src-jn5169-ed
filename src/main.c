#include <jendefs.h>
#include "dbg.h"
#include "dbg_uart.h"
#include "string.h"
#include "portmacro.h"
#include "pwrm.h"
#include "PDM.h"
#include "ZQueue.h"
#include "ZTimer.h"
#include "app_main.h"
#define APP_BUTTON_DIO 1
#define APP_BUTTONS_DIO_MASK (1 << APP_BUTTON_DIO)

PRIVATE uint8 keepAliveTime = 10;
PRIVATE pwrm_tsWakeTimerEvent wakeStruct;

PRIVATE void vSetUpWakeUpConditions();
PUBLIC void wakeCallBack(void);
static PWRM_DECLARE_CALLBACK_DESCRIPTOR(PreSleep);
static PWRM_DECLARE_CALLBACK_DESCRIPTOR(Wakeup);

void vAppRegisterPWRMCallbacks(void){
    PWRM_vRegisterPreSleepCallback(PreSleep);
    PWRM_vRegisterWakeupCallback(Wakeup);
}

PUBLIC void APP_vScheduleActivity(void) {
    uint8 u8Status;
    u8Status = PWRM_eScheduleActivity(&wakeStruct, keepAliveTime * 32000, wakeCallBack);
    DBG_vPrintf(TRUE, "PWRM_eScheduleActivity - %d\n", u8Status);
}

PUBLIC void vISR_SystemController(void)
{
    uint8 u8WakeInt = u8AHI_WakeTimerFiredStatus();

    if(u32AHI_DioWakeStatus() & APP_BUTTONS_DIO_MASK)
    {
        DBG_vPrintf(TRUE, "Button interrupt\n");
        vAHI_DioInterruptEnable(0, APP_BUTTONS_DIO_MASK);
    }

    if(u8WakeInt & E_AHI_WAKE_TIMER_MASK_1)
    {
        /* wake timer interrupt got us here */
        DBG_vPrintf(TRUE, "APP: Wake Timer 1 Interrupt\n");

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
    APP_vSetUpHardware();
    APP_vInitResources();
    APP_vInitialise();
    BDB_vStart();
    // Do this only on specific button click
    // BDB_eNsStartNwkSteering();
    // APP_vScheduleActivity();
    APP_vMainLoop();
}

PUBLIC void wakeCallBack(void)
{
    DBG_vPrintf(TRUE, "\nwakeCallBack()\n");
}

PRIVATE void vSetUpWakeUpConditions()
{
    u32AHI_DioWakeStatus();
    vAHI_DioSetDirection(APP_BUTTONS_DIO_MASK, 0);
    DBG_vPrintf(TRUE, "Going to sleep: Buttons:%08x Mask:%08x\n", u32AHI_DioReadInput() & APP_BUTTONS_DIO_MASK, APP_BUTTONS_DIO_MASK);
    vAHI_DioWakeEdge(0, APP_BUTTONS_DIO_MASK);
    vAHI_DioWakeEnable(APP_BUTTONS_DIO_MASK, 0);
}

PWRM_CALLBACK(PreSleep)
{
    DBG_vPrintf(TRUE, "\nPreSleep\n");
    vAppApiSaveMacSettings();
    DBG_vPrintf(TRUE, "PreSleep: 2\n");
    ZTIMER_vSleep();
    /* Set up wake up dio input */
    vSetUpWakeUpConditions();
    DBG_vUartFlush();
    vAHI_UartDisable(E_AHI_UART_0);
}

PWRM_CALLBACK(Wakeup)
{
    waitForXTAL();
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
    DBG_vPrintf(TRUE, "\nWakeup\n");
    vMAC_RestoreSettings();
    DBG_vPrintf(TRUE, "Wakeup: 2\n");
    APP_vSetUpHardware();
    ZTIMER_vWake();
    /* Activate the SleepTask, that would start the SW timer and polling would continue * */
    // APP_vStartUpHW();
}
