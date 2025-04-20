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
#include "app_events.h"
#include "app_led.h"
#include "app_main.h"
#include "app_node.h"
#include "ConfigurationCluster.h"

PRIVATE uint16 u16ButtonIdleCycles = 0;
PRIVATE teCLD_ButtonMode eButtonMode;
PRIVATE uint16 u16ResetButtonPressedCycles = 0;

APP_tsButtonTracker sButtonTracker = {
    {
        LEFT_BUTTON_ENDPOINT,
        LEFT_BUTTON_BLINK_MODE,
        APP_BTN_LEFT_DIO,
        IDLE,
        FALSE,
        0,
        APP_BTN_DEBOUNCE_MASK,
    },
#ifdef TARGET_WXKG07LM
    {
        WXKG07LM_RIGHTBUTTON_ENDPOINT,
        BLINK_RIGHT,
        APP_BTN_RIGHT_DIO,
        IDLE,
        FALSE,
        0,
        APP_BTN_DEBOUNCE_MASK,
    },
#endif
};

PRIVATE void vResetButtonsState(tsButtonState *sButtonState);
PRIVATE bool_t bHandleButtonState(tsButtonState *sButtonState, uint32 u32DIOState);
PRIVATE void handleResetButtonPressed(void);
PRIVATE void vHandleButtonModeToogle(tsButtonState *sButtonState);
PRIVATE void vHandleButtonModeMomentaryOnOff(tsButtonState *sButtonState);
PRIVATE void vHandleButtonModeMultistate(tsButtonState *sButtonState);
PRIVATE teButtonAction getMultiStateButtonAction(tsButtonState *sButtonState);

PUBLIC void APP_vConfigureButtons(void)
{
    DBG_vPrintf(TRACE_BUTTON, "BUTTON: Configuring buttons. Mask: %x\n", APP_BTN_CTRL_MASK);
    vAHI_DioSetDirection(APP_BTN_CTRL_MASK, 0);
    vAHI_DioSetPullup(0, APP_BTN_CTRL_MASK);
    vAHI_DioInterruptEdge(0, APP_BTN_CTRL_MASK);
    APP_cbTimerButtonScan(NULL);
}

PUBLIC void setButtonMode(teCLD_ButtonMode mode)
{
    DBG_vPrintf(TRACE_BUTTON, "BUTTON: Setting mode %d\n", mode);
    eButtonMode = mode;
    vResetButtonsState(&sButtonTracker.sLeftState);
#ifdef TARGET_WXKG07LM
    vResetButtonsState(&sButtonTracker.sRightState);
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
    bool_t bLeftBtnPressed = bHandleButtonState(&sButtonTracker.sLeftState, u32DIOState);
#ifdef TARGET_WXKG07LM
    bool_t bRightBtnPressed = bHandleButtonState(&sButtonTracker.sRightState, u32DIOState);
    bool_t bAnyBtnPressed = bLeftBtnPressed || bRightBtnPressed;
    bool_t bResetBtnPressed = bLeftBtnPressed && bRightBtnPressed;
#elif defined(TARGET_WXKG06LM)
    bool_t bAnyBtnPressed = bLeftBtnPressed;
    bool_t bResetBtnPressed = bLeftBtnPressed;
#endif
    if (bResetBtnPressed)
    {
        handleResetButtonPressed();
    }

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
        u16ResetButtonPressedCycles = 0;
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
    sButtonState->u16Cycles = 0;
}

PRIVATE void vHandleButtonModeToogle(tsButtonState *sButtonState)
{
    switch (sButtonState->u8Debounce)
    {
    case 0:
        if (!sButtonState->bPressed)
        {
            APP_vBlinkLed(sButtonState->eBlinkMode, 1);
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Pressed. Endpoint %d\n", sButtonState->u8ButtonDIO, sButtonState->u16Endpoint);
            sButtonState->bPressed = TRUE;
            sendButtonEvent(sButtonState->u16Endpoint, BUTTON_TOGGLE_ACTION);
        }
        break;

    case APP_BTN_DEBOUNCE_MASK:
        if (sButtonState->bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Released. Endpoint %d\n", sButtonState->u8ButtonDIO, sButtonState->u16Endpoint);
            sButtonState->bPressed = FALSE;
            vResetButtonsState(sButtonState);
        }

    default:
        break;
    }
}

PRIVATE void vHandleButtonModeMomentaryOnOff(tsButtonState *sButtonState)
{
    switch (sButtonState->u8Debounce)
    {
    case 0:
        if (!sButtonState->bPressed)
        {
            APP_vBlinkLed(sButtonState->eBlinkMode, 1);
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Pressed. Endpoint %d\n", sButtonState->u8ButtonDIO, sButtonState->u16Endpoint);
            sButtonState->bPressed = TRUE;
            sendButtonEvent(sButtonState->u16Endpoint, BUTTON_MOMENTRAY_PRESSED_ACTION);
        }
        break;

    case APP_BTN_DEBOUNCE_MASK:
        if (sButtonState->bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Released. Endpoint %d\n", sButtonState->u8ButtonDIO, sButtonState->u16Endpoint);
            sButtonState->bPressed = FALSE;
            vResetButtonsState(sButtonState);
            sendButtonEvent(sButtonState->u16Endpoint, BUTTON_MOMENTARY_RELEASED_ACTION);
        }

    default:
        break;
    }
}

PRIVATE void vHandleButtonModeMultistate(tsButtonState *sButtonState)
{
    switch (sButtonState->u8Debounce)
    {
    case 0:
        sButtonState->u16Cycles++;
        if (!sButtonState->bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Pressed\n", sButtonState->u8ButtonDIO);
            sButtonState->bPressed = TRUE;
            APP_vBlinkLed(sButtonState->eBlinkMode, 1);
            sButtonState->u16Cycles = 0;
        }
        switch (sButtonState->eState)
        {
        case IDLE:
            if (sButtonState->u16Cycles == 0)
                sButtonState->eState = SINGLE_CLICK;
            break;
        case SINGLE_CLICK:
            if (sButtonState->u16Cycles == APP_BTN_LONG_PRESS_REGISTER_CYCLES)
            {
                sButtonState->eState = LONG_CLICK;
                DBG_vPrintf(TRACE_BUTTON, "BUTTON: Changing state to LONG_CLICK and emitting action\n");
                sendButtonEvent(sButtonState->u16Endpoint, BUTTON_LONG_PRESSED_ACTION);
                break;
            }
        case DOUBLE_CLICK:
            if (sButtonState->u16Cycles == 0)
                sButtonState->eState++;
        case TRIPLE_CLICK:
            if (sButtonState->u16Cycles == APP_BTN_REGISTER_WINDOW_CYCLES && sButtonState->eState != SINGLE_CLICK)
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
        sButtonState->u16Cycles++;
        if (sButtonState->bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO %d Released\n", sButtonState->u8ButtonDIO);
            sButtonState->bPressed = FALSE;
            sButtonState->u16Cycles = 0;
        }

        switch (sButtonState->eState)
        {
        case IDLE:
            break;
        case SINGLE_CLICK:
        case DOUBLE_CLICK:
        case TRIPLE_CLICK:
            if (sButtonState->u16Cycles == APP_BTN_REGISTER_WINDOW_CYCLES)
            {
                DBG_vPrintf(TRACE_BUTTON, "BUTTON: Emmiting event for state %d. Cycles: %d\n", sButtonState->eState, sButtonState->u16Cycles);
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
    sButtonState->u8Debounce <<= 1;
    sButtonState->u8Debounce |= u8ButtonUp;
    sButtonState->u8Debounce &= APP_BTN_DEBOUNCE_MASK;

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
    return sButtonState->bPressed;
}

PRIVATE void handleResetButtonPressed(void)
{
    u16ResetButtonPressedCycles++;
#ifdef TRACE_BUTTON
    if (u16ResetButtonPressedCycles > 0 && u16ResetButtonPressedCycles % 100 == 0)
        DBG_vPrintf(TRACE_BUTTON, "BUTTON: Both button cycles %d\n", u16ResetButtonPressedCycles);
#endif
    if (u16ResetButtonPressedCycles == APP_RESET_DEVICE_CYCLES)
    {
        DBG_vPrintf(TRACE_BUTTON, "BUTTON: Sending reset device event\n");
        sendResetDeviceEvent();
    }
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