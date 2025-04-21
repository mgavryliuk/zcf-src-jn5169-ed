#include <jendefs.h>
#include "dbg.h"
#include "bdb_start.h"
#include "portmacro.h"
#include "pwrm.h"
#include "zps_gen.h"
#include "PDM.h"
#include "OnOff.h"
#include "zcl_options.h"

#include "app_button.h"
#include "app_device.h"
#include "app_events.h"
#include "app_led.h"
#include "app_main.h"
#include "app_node.h"
#include "ConfigurationCluster.h"

PRIVATE uint16 u16ButtonIdleCycles = 0;
PRIVATE teCLD_ButtonMode eButtonMode;
PRIVATE tsPressedState sResetPressedState = {
    .bPressed = FALSE,
    .u16Cycles = 0,
    .u8Debounce = APP_BTN_DEBOUNCE_MASK,
};

APP_tsButtonTracker sButtonTracker = {
    {
        .u16Endpoint = APP_BTN1_ENDPOINT,
        .eBlinkMode = APP_BTN1_BLINK_MODE,
        .u8ButtonDIO = APP_BTN1_DIO,
        .eState = IDLE,
        .sPressedState = {
            .bPressed = FALSE,
            .u16Cycles = 0,
            .u8Debounce = APP_BTN_DEBOUNCE_MASK,
        },
    },
#ifdef TARGET_WXKG07LM
    {
        .u16Endpoint = APP_BTN2_ENDPOINT,
        .eBlinkMode = APP_BTN2_BLINK_MODE,
        .u8ButtonDIO = APP_BTN2_DIO,
        .eState = IDLE,
        .sPressedState = {
            .bPressed = FALSE,
            .u16Cycles = 0,
            .u8Debounce = APP_BTN_DEBOUNCE_MASK,
        },
    },
#endif
};

PRIVATE void vResetButtonsState(tsButtonState *sButtonState);
PRIVATE bool_t bHandleButtonState(tsButtonState *sButtonState, uint32 u32DIOState);
PRIVATE bool_t bHandleResetButtonPressed(uint32 u32DIOState);
PRIVATE void vHandleButtonModeToogle(tsButtonState *sButtonState);
PRIVATE void vHandleButtonModeMomentaryOnOff(tsButtonState *sButtonState);
PRIVATE void vHandleButtonModeMultistate(tsButtonState *sButtonState);
PRIVATE teButtonAction getMultiStateButtonAction(tsButtonState *sButtonState);

PUBLIC void APP_vConfigureButtons(void)
{
    DBG_vPrintf(TRACE_BUTTON, "BUTTON: Configuring buttons. Mask: %x\n", APP_BTN_CTRL_MASK);
    vAHI_DioSetDirection(APP_BTN_CTRL_MASK, 0);
    DBG_vPrintf(TRACE_BUTTON, "BUTTON: Disabling pull-up for buttons\n");
    vAHI_DioSetPullup(0, APP_BTN_CTRL_MASK);
    vAHI_DioInterruptEdge(0, APP_BTN_CTRL_MASK);
    APP_cbTimerButtonScan(NULL);
}

PUBLIC void setButtonMode(teCLD_ButtonMode mode)
{
    DBG_vPrintf(TRACE_BUTTON, "BUTTON: Setting mode %d\n", mode);
    eButtonMode = mode;
    vResetButtonsState(&sButtonTracker.sBtn1State);
#ifdef TARGET_WXKG07LM
    vResetButtonsState(&sButtonTracker.sBtn2State);
#endif
}

PUBLIC teCLD_ButtonMode getButtonMode(void)
{
    return eButtonMode;
}

PUBLIC void APP_cbTimerButtonScan(void *pvParam)
{
    (void)u32AHI_DioInterruptStatus();
    uint32 u32DIOState = u32AHI_DioReadInput() & APP_BTN_CTRL_MASK;
    bool_t bBtn1Pressed = bHandleButtonState(&sButtonTracker.sBtn1State, u32DIOState);
    bool_t bResetBtnPressed = bHandleResetButtonPressed(u32DIOState);
#ifdef TARGET_WXKG07LM
    bool_t bBtn2Pressed = bHandleButtonState(&sButtonTracker.sBtn2State, u32DIOState);
    bool_t bAnyBtnPressed = bBtn1Pressed || bBtn2Pressed || bResetBtnPressed;
#elif defined(TARGET_WXKG06LM) || defined(TARGET_WXKG11LM)
    bool_t bAnyBtnPressed = bBtn1Pressed || bResetBtnPressed;
#else
    #error "Unknown target device"
#endif
    if (bAnyBtnPressed)
    {
        u16ButtonIdleCycles = 0;
    }
    else
    {
        u16ButtonIdleCycles++;
#ifdef TRACE_BUTTON
        if (u16ButtonIdleCycles % 50 == 0)
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: IDLE cycle %d\n", u16ButtonIdleCycles);
#endif
    }

    if (u16ButtonIdleCycles == APP_BTN_IDLE_CYCLES_MAX)
    {
        u16ButtonIdleCycles = 0;
        DBG_vPrintf(TRACE_BUTTON, "BUTTON: Stopping scan\n");
        vAHI_DioInterruptEnable(APP_BTN_CTRL_MASK, 0);
        ZTIMER_eStop(u8TimerButtonScan);
    }
    else
    {
        ZTIMER_eStop(u8TimerButtonScan);
        ZTIMER_eStart(u8TimerButtonScan, APP_BTN_TIMER_MSEC);
    }
}

PRIVATE void vResetButtonsState(tsButtonState *sButtonState)
{
    sButtonState->eState = IDLE;
    sButtonState->sPressedState.u16Cycles = 0;
}

PRIVATE void vHandleButtonModeToogle(tsButtonState *sButtonState)
{
    switch (sButtonState->sPressedState.u8Debounce)
    {
    case 0:
        if (!sButtonState->sPressedState.bPressed)
        {
            APP_vBlinkLed(sButtonState->eBlinkMode, 1);
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Pressed. Endpoint %d\n", sButtonState->u8ButtonDIO, sButtonState->u16Endpoint);
            sButtonState->sPressedState.bPressed = TRUE;
            sendButtonEvent(sButtonState->u16Endpoint, BUTTON_TOGGLE_ACTION);
        }
        break;

    case APP_BTN_DEBOUNCE_MASK:
        if (sButtonState->sPressedState.bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Released. Endpoint %d\n", sButtonState->u8ButtonDIO, sButtonState->u16Endpoint);
            sButtonState->sPressedState.bPressed = FALSE;
            vResetButtonsState(sButtonState);
        }

    default:
        break;
    }
}

PRIVATE void vHandleButtonModeMomentaryOnOff(tsButtonState *sButtonState)
{
    switch (sButtonState->sPressedState.u8Debounce)
    {
    case 0:
        if (!sButtonState->sPressedState.bPressed)
        {
            APP_vBlinkLed(sButtonState->eBlinkMode, 1);
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Pressed. Endpoint %d\n", sButtonState->u8ButtonDIO, sButtonState->u16Endpoint);
            sButtonState->sPressedState.bPressed = TRUE;
            sendButtonEvent(sButtonState->u16Endpoint, BUTTON_MOMENTRAY_PRESSED_ACTION);
        }
        break;

    case APP_BTN_DEBOUNCE_MASK:
        if (sButtonState->sPressedState.bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Released. Endpoint %d\n", sButtonState->u8ButtonDIO, sButtonState->u16Endpoint);
            sButtonState->sPressedState.bPressed = FALSE;
            vResetButtonsState(sButtonState);
            sendButtonEvent(sButtonState->u16Endpoint, BUTTON_MOMENTARY_RELEASED_ACTION);
        }

    default:
        break;
    }
}

PRIVATE void vHandleButtonModeMultistate(tsButtonState *sButtonState)
{
    switch (sButtonState->sPressedState.u8Debounce)
    {
    case 0:
        sButtonState->sPressedState.u16Cycles++;
        if (!sButtonState->sPressedState.bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Pressed\n", sButtonState->u8ButtonDIO);
            sButtonState->sPressedState.bPressed = TRUE;
            APP_vBlinkLed(sButtonState->eBlinkMode, 1);
            sButtonState->sPressedState.u16Cycles = 0;
        }
        switch (sButtonState->eState)
        {
        case IDLE:
            if (sButtonState->sPressedState.u16Cycles == 0)
                sButtonState->eState = SINGLE_CLICK;
            break;
        case SINGLE_CLICK:
            if (sButtonState->sPressedState.u16Cycles == APP_BTN_LONG_PRESS_REGISTER_CYCLES)
            {
                sButtonState->eState = LONG_CLICK;
                DBG_vPrintf(TRACE_BUTTON, "BUTTON: Changing state to LONG_CLICK and emitting action\n");
                sendButtonEvent(sButtonState->u16Endpoint, BUTTON_LONG_PRESSED_ACTION);
                break;
            }
        case DOUBLE_CLICK:
            if (sButtonState->sPressedState.u16Cycles == 0)
                sButtonState->eState++;
        case TRIPLE_CLICK:
            if (sButtonState->sPressedState.u16Cycles == APP_BTN_REGISTER_WINDOW_CYCLES && sButtonState->eState != SINGLE_CLICK)
            {
                sendButtonEvent(sButtonState->u16Endpoint, getMultiStateButtonAction(sButtonState));
                vResetButtonsState(sButtonState);
            }
            break;
        default:
            break;
        }
        break;

    case APP_BTN_DEBOUNCE_MASK:
        sButtonState->sPressedState.u16Cycles++;
        if (sButtonState->sPressedState.bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Released\n", sButtonState->u8ButtonDIO);
            sButtonState->sPressedState.bPressed = FALSE;
            sButtonState->sPressedState.u16Cycles = 0;
        }

        switch (sButtonState->eState)
        {
        case IDLE:
            break;
        case SINGLE_CLICK:
        case DOUBLE_CLICK:
        case TRIPLE_CLICK:
            if (sButtonState->sPressedState.u16Cycles == APP_BTN_REGISTER_WINDOW_CYCLES)
            {
                DBG_vPrintf(TRACE_BUTTON, "BUTTON: Emmiting event for state %d. Cycles: %d\n", sButtonState->eState, sButtonState->sPressedState.u16Cycles);
                sendButtonEvent(sButtonState->u16Endpoint, getMultiStateButtonAction(sButtonState));
                vResetButtonsState(sButtonState);
            }
            break;
        case LONG_CLICK:
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: Emiting LONG_CLICK release action\n");
            sendButtonEvent(sButtonState->u16Endpoint, BUTTON_LONG_RELEASED_ACTION);
            vResetButtonsState(sButtonState);
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}

PRIVATE bool_t bHandleButtonState(tsButtonState *sButtonState, uint32 u32DIOState)
{
    uint8 u8ButtonUp = (uint8)((u32DIOState >> sButtonState->u8ButtonDIO) & 1);
    sButtonState->sPressedState.u8Debounce <<= 1;
    sButtonState->sPressedState.u8Debounce |= u8ButtonUp;
    sButtonState->sPressedState.u8Debounce &= APP_BTN_DEBOUNCE_MASK;

    switch (eButtonMode)
    {
    case E_CLD_BUTTON_MODE_TOGGLE:
        vHandleButtonModeToogle(sButtonState);
        break;
    case E_CLD_BUTTON_MODE_MOMENTARY_ON_OFF:
        vHandleButtonModeMomentaryOnOff(sButtonState);
        break;
    case E_CLD_BUTTON_MODE_MULTISTATE_INPUT:
        vHandleButtonModeMultistate(sButtonState);
        break;
    default:
        break;
    }
    return sButtonState->sPressedState.bPressed;
}

PRIVATE bool_t bHandleResetButtonPressed(uint32 u32DIOState)
{
    uint8 u8ButtonUp = (u32DIOState & APP_BTN_RESET_MASK) ? 1 : 0;
    sResetPressedState.u8Debounce <<= 1;
    sResetPressedState.u8Debounce |= u8ButtonUp;
    sResetPressedState.u8Debounce &= APP_BTN_DEBOUNCE_MASK;

    switch (sResetPressedState.u8Debounce)
    {
    case 0:
        sResetPressedState.u16Cycles++;
        if (!sResetPressedState.bPressed)
        {
            APP_vBlinkLed(APP_RESET_DEVICE_BLINK_MODE, 1);
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: Reset device combination pressed. Reset mask: %x\n", APP_BTN_RESET_MASK);
            sResetPressedState.bPressed = TRUE;
        }

        if (sResetPressedState.u16Cycles == APP_RESET_DEVICE_CYCLES)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: Sending reset device event\n");
            sendResetDeviceEvent();
        }
        break;

    case APP_BTN_DEBOUNCE_MASK:
        if (sResetPressedState.bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: Reset device combination released\n");
            sResetPressedState.bPressed = FALSE;
            sResetPressedState.u16Cycles = 0;
        }

    default:
        break;
    }

    return sResetPressedState.bPressed;
}

PRIVATE teButtonAction getMultiStateButtonAction(tsButtonState *sButtonState)
{
    teButtonAction eButtonAction = BUTTON_NONE_ACTION;
    switch (sButtonState->eState)
    {
    case SINGLE_CLICK:
        eButtonAction = BUTTON_SINGLE_CLICK_ACTION;
        break;

    case DOUBLE_CLICK:
        eButtonAction = BUTTON_DOUBLE_CLICK_ACTION;
        break;

    case TRIPLE_CLICK:
        eButtonAction = BUTTON_TRIPLE_CLICK_ACTION;
        break;

    default:
        break;
    }
    return eButtonAction;
}
