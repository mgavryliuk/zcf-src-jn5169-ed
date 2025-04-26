#include <jendefs.h>

#include "dbg.h"
#include "zcl_customcommand.h"
#include "AppHardwareApi.h"
#include "ZTimer.h"

#include "device_config.h"
#include "ConfigurationCluster.h"
#include "app_buttons.h"
#include "app_button_actions.h"
#include "app_events.h"
#include "app_resources.h"

PRIVATE uint16 u16ButtonIdleCycles = 0;
PRIVATE tsButtonState sResetButtonState = {
    .u.sResetBtnConfig = &sDeviceConfig.sResetButtonConfig,
    .bPressed = FALSE,
    .u16Cycles = 0,
    .u8Debounce = APP_BTN_DEBOUNCE_MASK,
};

PRIVATE tsButtonState sButtonsTrackers[BUTTON_COUNT];

PRIVATE void vResetButtonsState(tsButtonState *sButtonState);
PRIVATE bool_t bHandleButtonState(tsButtonState *sButtonState, uint32 u32DIOState);
PRIVATE bool_t bHandleResetButtonPressed(uint32 u32DIOState);
PRIVATE void vHandleButtonModeToogle(tsButtonState *sButtonState);
PRIVATE void vHandleButtonModeMomentaryOnOff(tsButtonState *sButtonState);
PRIVATE void vHandleButtonModeMultistate(tsButtonState *sButtonState);
PRIVATE teButtonAction eGetMultiStateButtonAction(tsButtonState *sButtonState);

PUBLIC void APP_vConfigureButtons(void)
{
    DBG_vPrintf(TRACE_BUTTON, "BUTTON: Configuring buttons. Mask: %x\n", sDeviceConfig.sDioConfig.u32DioMask);
    vAHI_DioSetDirection(sDeviceConfig.sDioConfig.u32DioMask, 0);
    DBG_vPrintf(TRACE_BUTTON, "BUTTON: Disabling pull-up for buttons\n");
    vAHI_DioSetPullup(0, sDeviceConfig.sDioConfig.u32DioMask);
    vAHI_DioInterruptEdge(0, sDeviceConfig.sDioConfig.u32DioMask);

    DBG_vPrintf(TRACE_BUTTON, "BUTTON: Initializing button trackers\n");
    uint8 i;
    for (i = 0; i < BUTTON_COUNT; i++) {
        sButtonsTrackers[i].u.sEndpointBtnConfig = &sDeviceConfig.psEndpointButtons[i];
        sButtonsTrackers[i].bPressed = FALSE;
        sButtonsTrackers[i].u16Cycles = 0;
        sButtonsTrackers[i].u8Debounce = APP_BTN_DEBOUNCE_MASK;
    }

    APP_cbTimerButtonScan(NULL);
}

PUBLIC void APP_cbTimerButtonScan(void *pvParam)
{
    bool_t bAnyBtnPressed = FALSE;
    (void)u32AHI_DioInterruptStatus();
    uint32 u32DIOState = u32AHI_DioReadInput() & sDeviceConfig.sDioConfig.u32DioMask;
    uint8 i;
    for (i = 0; i < BUTTON_COUNT; i++) {
        bAnyBtnPressed |= bHandleButtonState(&sButtonsTrackers[i], u32DIOState);
    }

    bool_t bResetBtnPressed = bHandleResetButtonPressed(u32DIOState);

    if (bAnyBtnPressed || bResetBtnPressed)
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
        vAHI_DioInterruptEnable(sDeviceConfig.sDioConfig.u32DioMask, 0);
        for (i = 0; i < BUTTON_COUNT; i++) {
            vResetButtonsState(&sButtonsTrackers[i]);
        }
        ZTIMER_eStop(u8TimerButtonScan);
    }
    else
    {
        ZTIMER_eStop(u8TimerButtonScan);
        ZTIMER_eStart(u8TimerButtonScan, BUTTON_SCAN_TIME_MSEC);
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
            // APP_vBlinkLed(sButtonState->eBlinkMode, 1);
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO for mask %x pressed. Endpoint %d\n", sButtonState->u.sEndpointBtnConfig->u32DioMask, sButtonState->u.sEndpointBtnConfig->u16Endpoint);
            sButtonState->bPressed = TRUE;
            APP_vSendButtonEvent(sButtonState->u.sEndpointBtnConfig->u16Endpoint, BUTTON_TOGGLE_ACTION);
        }
        break;

    case APP_BTN_DEBOUNCE_MASK:
        if (sButtonState->bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO for mask %x released. Endpoint %d\n", sButtonState->u.sEndpointBtnConfig->u32DioMask, sButtonState->u.sEndpointBtnConfig->u16Endpoint);
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
            // APP_vBlinkLed(sButtonState->eBlinkMode, 1);
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO for mask %x pressed. Endpoint %d\n", sButtonState->u.sEndpointBtnConfig->u32DioMask, sButtonState->u.sEndpointBtnConfig->u16Endpoint);
            sButtonState->bPressed = TRUE;
            APP_vSendButtonEvent(sButtonState->u.sEndpointBtnConfig->u16Endpoint, BUTTON_MOMENTRAY_PRESSED_ACTION);
        }
        break;

    case APP_BTN_DEBOUNCE_MASK:
        if (sButtonState->bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO for mask %x released. Endpoint %d\n", sButtonState->u.sEndpointBtnConfig->u32DioMask, sButtonState->u.sEndpointBtnConfig->u16Endpoint);
            sButtonState->bPressed = FALSE;
            vResetButtonsState(sButtonState);
            APP_vSendButtonEvent(sButtonState->u.sEndpointBtnConfig->u16Endpoint, BUTTON_MOMENTARY_RELEASED_ACTION);
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
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO for mask %x pressed\n", sButtonState->u.sEndpointBtnConfig->u32DioMask);
            sButtonState->bPressed = TRUE;
            // APP_vBlinkLed(sButtonState->eBlinkMode, 1);
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
                APP_vSendButtonEvent(sButtonState->u.sEndpointBtnConfig->u16Endpoint, BUTTON_LONG_PRESSED_ACTION);
                break;
            }
        case DOUBLE_CLICK:
            if (sButtonState->u16Cycles == 0)
                sButtonState->eState++;
        case TRIPLE_CLICK:
            if (sButtonState->u16Cycles == APP_BTN_REGISTER_WINDOW_CYCLES && sButtonState->eState != SINGLE_CLICK)
            {
                APP_vSendButtonEvent(sButtonState->u.sEndpointBtnConfig->u16Endpoint, eGetMultiStateButtonAction(sButtonState));
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
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: DIO for mask %x released\n", sButtonState->u.sEndpointBtnConfig->u32DioMask);
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
                APP_vSendButtonEvent(sButtonState->u.sEndpointBtnConfig->u16Endpoint, eGetMultiStateButtonAction(sButtonState));
                vResetButtonsState(sButtonState);
            }
            break;
        case LONG_CLICK:
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: Emiting LONG_CLICK release action\n");
            APP_vSendButtonEvent(sButtonState->u.sEndpointBtnConfig->u16Endpoint, BUTTON_LONG_RELEASED_ACTION);
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
    uint8 u8ButtonUp = (u32DIOState & sButtonState->u.sEndpointBtnConfig->u32DioMask) ? 1 : 0;
    sButtonState->u8Debounce <<= 1;
    sButtonState->u8Debounce |= u8ButtonUp;
    sButtonState->u8Debounce &= APP_BTN_DEBOUNCE_MASK;

    tsZCL_ClusterInstance *psZCL_ClusterInstance;
    teZCL_Status eStatus = eZCL_SearchForClusterEntry(sDeviceConfig.u8BasicEndpoint, GENERAL_CLUSTER_ID_CONFIGURATION, TRUE, &psZCL_ClusterInstance);
    if (eStatus != E_ZCL_SUCCESS) {
        DBG_vPrintf(TRACE_BUTTON, "BUTTON: Search for cluster entry %d in endpoint %d failed with status: %d\n", GENERAL_CLUSTER_ID_CONFIGURATION, sDeviceConfig.u8BasicEndpoint, eStatus);
    }
    switch (((tsCLD_Configuration *)psZCL_ClusterInstance->pvEndPointSharedStructPtr)->eButtonMode)
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

PRIVATE bool_t bHandleResetButtonPressed(uint32 u32DIOState)
{
    uint8 u8ButtonUp = (u32DIOState & sResetButtonState.u.sResetBtnConfig->u32DioMask) ? 1 : 0;
    sResetButtonState.u8Debounce <<= 1;
    sResetButtonState.u8Debounce |= u8ButtonUp;
    sResetButtonState.u8Debounce &= APP_BTN_DEBOUNCE_MASK;

    switch (sResetButtonState.u8Debounce)
    {
    case 0:
        sResetButtonState.u16Cycles++;
        if (!sResetButtonState.bPressed)
        {
            // APP_vBlinkLed(APP_RESET_DEVICE_BLINK_MODE, 1);
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: Reset device combination pressed. Reset mask: %x\n", sResetButtonState.u.sResetBtnConfig->u32DioMask);
            sResetButtonState.bPressed = TRUE;
        }

        if (sResetButtonState.u16Cycles == APP_RESET_DEVICE_CYCLES)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: Sending reset device event\n");
            APP_vSendResetDeviceEvent();
        }
        break;

    case APP_BTN_DEBOUNCE_MASK:
        if (sResetButtonState.bPressed)
        {
            DBG_vPrintf(TRACE_BUTTON, "BUTTON: Reset device combination released\n");
            sResetButtonState.bPressed = FALSE;
            sResetButtonState.u16Cycles = 0;
        }

    default:
        break;
    }

    return sResetButtonState.bPressed;
}

PRIVATE teButtonAction eGetMultiStateButtonAction(tsButtonState *sButtonState)
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
