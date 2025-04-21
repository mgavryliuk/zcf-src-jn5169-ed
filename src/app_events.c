#include "dbg.h"
#include "bdb_api.h"
#include "AppHardwareApi.h"
#include "OnOff.h"

#include "app_basic_endpoint.h"
#include "app_button.h"
#include "app_events.h"
#include "app_main.h"
#include "app_node.h"
#include "app_polling.h"
#include "app_on_off_endpoint.h"

PRIVATE void sendAppEvent(APP_tsEvent *sEvent);
PRIVATE void handleButtonEvent(APP_tsEvent *sAppEvent);

PRIVATE uint8 vWakeUpCount = 0;

PUBLIC void sendWakeUpEvent(void)
{
    APP_tsEvent sWakeUpEvent;
    sWakeUpEvent.eType = APP_WAKE_UP_EVENT;
    sendAppEvent(&sWakeUpEvent);
}

PUBLIC void sendResetDeviceEvent(void)
{
    APP_tsEvent sResetDeviceEvent;
    sResetDeviceEvent.eType = APP_RESET_DEVICE_EVENT;
    sendAppEvent(&sResetDeviceEvent);
}

PUBLIC void sendButtonEvent(uint16 u16Endpoint, teButtonAction eButtonAction)
{
    APP_tsEvent sButtonEvent;
    sButtonEvent.eType = APP_BUTTON_EVENT;
    sButtonEvent.uEvent.sButton.u16Endpoint = u16Endpoint;
    sButtonEvent.uEvent.sButton.eButtonAction = eButtonAction;
    sendAppEvent(&sButtonEvent);
}

PUBLIC void APP_processEvents(void)
{
    APP_tsEvent sAppEvent;
    sAppEvent.eType = APP_NO_EVENT;
    while (ZQ_bQueueReceive(&APP_msgAppEvents, &sAppEvent) == TRUE)
    {
        DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Found event %d in AppEvents queue\n", sAppEvent.eType);
        switch (sAppEvent.eType)
        {
        case APP_WAKE_UP_EVENT:
            if (bNodeJoined())
            {
                vWakeUpCount++;
                DBG_vPrintf(TRUE, "APP EVENTS: Wake UP count: %d\n", vWakeUpCount);
                if (vWakeUpCount == SYNC_BATTERY_VALUES_WAKE_UP_CYCLES)
                {
                    DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Called syncBatteryStatus\n");
                    syncBatteryStatus();
                    vWakeUpCount = 0;
                }
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Starting ZCL tick timer\n");
                ZTIMER_eStart(u8TimerTick, ZCL_TICK_TIME);
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Starting polling in POLL_REGULAR mode\n");
                APP_vStartPolling(POLL_REGULAR);
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Sending ZIGBEE keep alive\n");
                // this functions should send keep alive but I do not see it in wireshark sniffer
                ZPS_eAplAfSendKeepAlive();
            }
            else
            {
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Node is not joined. Doing nothing\n");
            }
            break;

        case APP_RESET_DEVICE_EVENT:
            if (eGetNodeState() == E_NO_NETWORK)
            {
                APP_vFactoryResetRecords();
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Device is not in network. Starting NWK Steering...\n");
                APP_vBlinkLed(BLINK_BOTH, 5);
                BDB_eNsStartNwkSteering();
                break;
            }
            else
            {
                if (bNodeJoined() && bGetPreventReset())
                {
                    DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Reset prevention enabled. Doing nothing...\n");
                    return;
                }

                if (ZPS_eAplZdoLeaveNetwork(0UL, FALSE, FALSE) != ZPS_E_SUCCESS)
                {
                    DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Leave network msg failed. Force resetting device...\n");
                    APP_vFactoryResetRecords();
                    vAHI_SwReset();
                }
            }
            break;

        case APP_BUTTON_EVENT:
            if (bNodeJoined())
                handleButtonEvent(&sAppEvent);
            break;

        default:
            DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Unsupported event %d\n", sAppEvent.eType);
            break;
        }
    }
}

PRIVATE void sendAppEvent(APP_tsEvent *sEvent)
{
    DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Sending event with type %d to AppEvents queue\n", sEvent->eType);
    ZQ_bQueueSend(&APP_msgAppEvents, sEvent);
}

PRIVATE void handleButtonEvent(APP_tsEvent *sAppEvent)
{
    DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Got button event %d for endpoint %d\n", sAppEvent->uEvent.sButton.eButtonAction, sAppEvent->uEvent.sButton.u16Endpoint);
    switch (sAppEvent->uEvent.sButton.eButtonAction)
    {
    case BUTTON_TOGGLE_ACTION:
        eSendCommand(sAppEvent->uEvent.sButton.u16Endpoint, E_CLD_ONOFF_CMD_TOGGLE);
        eReportAction(sAppEvent->uEvent.sButton.u16Endpoint, BUTTON_TOGGLE_ACTION);
        break;

    case BUTTON_MOMENTRAY_PRESSED_ACTION:
        eSendCommand(sAppEvent->uEvent.sButton.u16Endpoint, E_CLD_ONOFF_CMD_ON);
        eReportAction(sAppEvent->uEvent.sButton.u16Endpoint, BUTTON_MOMENTRAY_PRESSED_ACTION);
        break;

    case BUTTON_MOMENTARY_RELEASED_ACTION:
        eSendCommand(sAppEvent->uEvent.sButton.u16Endpoint, E_CLD_ONOFF_CMD_OFF);
        eReportAction(sAppEvent->uEvent.sButton.u16Endpoint, BUTTON_MOMENTARY_RELEASED_ACTION);
        break;

    case BUTTON_SINGLE_CLICK_ACTION:
        eReportAction(sAppEvent->uEvent.sButton.u16Endpoint, sAppEvent->uEvent.sButton.eButtonAction);
        eSendCommand(WXKG11LM_ONBUTTON_ENDPOINT, E_CLD_ONOFF_CMD_ON);
        break;
    case BUTTON_DOUBLE_CLICK_ACTION:
        eReportAction(sAppEvent->uEvent.sButton.u16Endpoint, sAppEvent->uEvent.sButton.eButtonAction);
        eSendCommand(WXKG11LM_OFFBUTTON_ENDPOINT, E_CLD_ONOFF_CMD_OFF);
        break;
    case BUTTON_TRIPLE_CLICK_ACTION:
    case BUTTON_LONG_PRESSED_ACTION:
    case BUTTON_LONG_RELEASED_ACTION:
        eReportAction(sAppEvent->uEvent.sButton.u16Endpoint, sAppEvent->uEvent.sButton.eButtonAction);
        break;

    default:
        break;
    }
}
