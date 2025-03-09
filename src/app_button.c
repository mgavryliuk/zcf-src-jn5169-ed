#include <jendefs.h>
#include "dbg.h"
#include "portmacro.h"
#include "pwrm.h"

#include "app_button.h"
#include "app_common.h"
#include "app_led.h"
#include "app_main.h"

PRIVATE uint16 u16ButtonIdleCycles = 0;
APP_tsButtonTracker sButtonTracker = {
    {APP_BTN_LEFT_DIO, IDLE, FALSE, 0, APP_BTN_DEBOUNCE_MASK},
    {APP_BTN_RIGHT_DIO, IDLE, FALSE, 0, APP_BTN_DEBOUNCE_MASK},
    FALSE};

PUBLIC void APP_vConfigureButtons(void)
{
    DBG_vPrintf(TRACE_APP, "APP: Configuring buttons. Mask: %x\n", APP_BTN_CTRL_MASK);
    vAHI_DioSetDirection(APP_BTN_CTRL_MASK, 0);
    vAHI_DioSetPullup(APP_BTN_CTRL_MASK, 0);
    vAHI_DioInterruptEdge(0, APP_BTN_CTRL_MASK);
    APP_cbTimerButtonScan(NULL);
}

PRIVATE void APP_vResetButtonState(APP_tsButtonState *sButton, APP_tsButtonState *sSecondButton)
{
    sButton->eState = IDLE;
    sButton->u16Cycles = 0;
    if (sSecondButton)
    {
        sSecondButton->eState = IDLE;
        sSecondButton->u16Cycles = 0;
    }
}

PRIVATE bool_t APP_bHandleButtonState(uint8 u8ButtonDIO, uint32 u32DIOState)
{
    APP_tsButtonState *sButton;
    APP_tsButtonState *sSecondButton;
    te_BlinkMode eBlinkMode;

    uint8 u8ButtonUp = (uint8)((u32DIOState >> u8ButtonDIO) & 1);
    if (u8ButtonDIO == APP_BTN_LEFT_DIO)
    {
        sButton = &sButtonTracker.sLeftState;
        sSecondButton = &sButtonTracker.sRightState;
        eBlinkMode = BLINK_LEFT;
    }
    else
    {
        sButton = &sButtonTracker.sRightState;
        sSecondButton = &sButtonTracker.sLeftState;
        eBlinkMode = BLINK_RIGHT;
    }

    sButton->u8Debounce <<= 1;
    sButton->u8Debounce |= u8ButtonUp;
    sButton->u8Debounce &= APP_BTN_DEBOUNCE_MASK;

    switch (sButton->u8Debounce)
    {
    case 0:
        if (!sButton->bPresssed)
        {
            DBG_vPrintf(TRACE_APP, "APP: Btn %d Pressed\n", u8ButtonDIO);
            sButton->bPresssed = TRUE;
            APP_vBlinkLed(eBlinkMode, 1);
            sButton->u16Cycles = 0;
        }
        switch (sButton->eState)
        {
        case IDLE:
            if (sButton->u16Cycles == 0)
                sButton->eState = SINGLE_CLICK;

            if (!sButtonTracker.bBothPressedMode && sSecondButton->eState == SINGLE_CLICK && sSecondButton->u16Cycles < APP_BTN_BOTH_REGISTER_CYCLES)
            {
                DBG_vPrintf(TRACE_APP, "APP: Second btn pressed. Changing BOTH mode to TRUE\n");
                sButtonTracker.bBothPressedMode = TRUE;
            }
            break;
        case SINGLE_CLICK:
            if (!sButtonTracker.bBothPressedMode && sButton->u16Cycles == APP_BTN_LONG_PRESS_REGISTER_CYCLES)
            {
                sButton->eState = LONG_CLICK;
                DBG_vPrintf(TRACE_APP, "APP: Changing state to LONG_CLICK and emitting action\n");
            }
            else if (sButtonTracker.bBothPressedMode && sButton->u16Cycles == APP_RESET_NETWORK_CYCLES)
            {
                DBG_vPrintf(TRACE_APP, "APP: Changing state to BOTH LONG_CLICK and emitting action\n");
                sButton->eState = LONG_CLICK;
                if (sSecondButton->eState == LONG_CLICK)
                {
                    DBG_vPrintf(TRACE_APP, "Resetting network...\n");
                    // TODO: move blink to event handler
                    APP_vBlinkLed(BLINK_BOTH, 10);
                }
            }
        case DOUBLE_CLICK:
            if (sButton->u16Cycles == 0)
                sButton->eState++;
            break;
        default:
            break;
        }
        sButton->u16Cycles++;
        break;

    case APP_BTN_DEBOUNCE_MASK:
        if (sButton->bPresssed)
        {
            DBG_vPrintf(TRACE_APP, "APP: Btn %d Released\n", u8ButtonDIO);
            sButton->bPresssed = FALSE;
            sButton->u16Cycles = 0;
        }
        switch (sButton->eState)
        {
        case IDLE:
            break;
        case SINGLE_CLICK:
        case DOUBLE_CLICK:
        case TRIPLE_CLICK:
            if (sButton->u16Cycles >= APP_BTN_REGISTER_WINDOW_CYCLES)
            {
                if (!sButtonTracker.bBothPressedMode)
                {
                    DBG_vPrintf(TRACE_APP, "APP: Emmiting event for state %d. Cycles: %d\n", sButton->eState, sButton->u16Cycles);
                    APP_vResetButtonState(sButton, NULL);
                }
                else if (sButton->eState == sSecondButton->eState)
                {
                    DBG_vPrintf(TRACE_APP, "APP: Emmiting event for BOTH state %d\n", sButton->eState);
                    APP_vResetButtonState(sButton, sSecondButton);
                    sButtonTracker.bBothPressedMode = FALSE;
                }
            }
            break;
        case LONG_CLICK:
            if (!sButtonTracker.bBothPressedMode)
            {
                DBG_vPrintf(TRACE_APP, "APP: Emiting LONG_CLICK release action\n");
                APP_vResetButtonState(sButton, NULL);
            }
            else if (sButton->eState == sSecondButton->eState)
            {
                DBG_vPrintf(TRACE_APP, "APP: Both buttons were release. Turning off BOTH mode\n");
                APP_vResetButtonState(sButton, sSecondButton);
                sButtonTracker.bBothPressedMode = FALSE;
            }
            break;
        default:
            break;
        }
        sButton->u16Cycles++;
        break;

    default:
        break;
    }
    return sButton->bPresssed;
}

PUBLIC void APP_cbTimerButtonScan(void *pvParam)
{
    (void)u32AHI_DioInterruptStatus();
    uint32 u32DIOState = u32AHI_DioReadInput() & APP_BTN_CTRL_MASK;
    bool_t bLeftBtnPressed = APP_bHandleButtonState(APP_BTN_LEFT_DIO, u32DIOState);
    bool_t bLeftRightPressed = APP_bHandleButtonState(APP_BTN_RIGHT_DIO, u32DIOState);
    if (bLeftBtnPressed || bLeftRightPressed)
    {
        u16ButtonIdleCycles = 0;
    }
    else
    {
        u16ButtonIdleCycles++;
    }

#ifdef TRACE_APP
    if (u16ButtonIdleCycles > 0 && u16ButtonIdleCycles % 100 == 0)
    {
        DBG_vPrintf(TRACE_APP, "APP: IDLE cycle %d\n", u16ButtonIdleCycles);
    }
#endif

    if (u16ButtonIdleCycles > APP_BTN_IDLE_CYCLES_MAX)
    {
        DBG_vPrintf(TRACE_APP, "APP: Stopping button scan\n");
        vAHI_DioInterruptEnable(APP_BTN_CTRL_MASK, 0);
        ZTIMER_eStop(u8TimerButtonScan);
    }
    else
    {
        ZTIMER_eStop(u8TimerButtonScan);
        ZTIMER_eStart(u8TimerButtonScan, APP_BTN_TIMER_MSEC);
    }
}
