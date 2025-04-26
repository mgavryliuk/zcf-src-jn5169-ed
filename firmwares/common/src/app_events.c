#include <jendefs.h>

#include "dbg.h"
#include "bdb_api.h"
#include "zcl.h"
#include "zcl_customcommand.h"
#include "AppHardwareApi.h"
#include "OnOff.h"
#include "MultistateInputBasic.h"

#include "app_battery.h"
#include "app_button_actions.h"
#include "app_events.h"
#include "app_led.h"
#include "app_node.h"
#include "app_polling.h"
#include "app_resources.h"
#include "device_config.h"
#include "ConfigurationCluster.h"

PRIVATE void sendAppEvent(APP_tsEvent *sEvent);
PRIVATE void handleButtonEvent(APP_tsEvent *sAppEvent);
PRIVATE teZCL_Status eSendCommand(uint8 u8Endpoint, teCLD_OnOff_Command eOnOffCommand);
PRIVATE teZCL_Status eReportAction(uint8 u8Endpoint, teButtonAction eButtonAction);

PUBLIC void APP_vSendWakeUpEvent(void)
{
    APP_tsEvent sWakeUpEvent;
    sWakeUpEvent.eType = APP_WAKE_UP_EVENT;
    sendAppEvent(&sWakeUpEvent);
}

PUBLIC void APP_vSendResetDeviceEvent(void)
{
    APP_tsEvent sResetDeviceEvent;
    sResetDeviceEvent.eType = APP_RESET_DEVICE_EVENT;
    sendAppEvent(&sResetDeviceEvent);
}

PUBLIC void APP_vSendButtonEvent(uint8 u8Endpoint, teButtonAction eButtonAction)
{
    APP_tsEvent sButtonEvent;
    sButtonEvent.eType = APP_BUTTON_EVENT;
    sButtonEvent.uEvent.sButton.u8Endpoint = u8Endpoint;
    sButtonEvent.uEvent.sButton.eButtonAction = eButtonAction;
    sendAppEvent(&sButtonEvent);
}

PUBLIC void APP_vProcessEvents(void)
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
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Sending ZIGBEE keep alive\n");
                ZPS_eAplAfSendKeepAlive();
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Starting ZCL tick timer\n");
                ZTIMER_eStart(u8TimerZclTick, ZCL_TICK_TIME_MSEC);
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Starting polling in POLL_REGULAR mode\n");
                APP_vStartPolling(POLL_REGULAR);
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Calling APP_vMeasureRemainingBattery\n");
                APP_vMeasureRemainingBattery();
            }
            else
            {
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Node is not joined. Doing nothing\n");
            }
            break;

        case APP_RESET_DEVICE_EVENT:
            if (eGetNodeState() == E_NO_NETWORK)
            {
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Device is not in network. Starting NWK Steering...\n");
                if (sDeviceConfig.sResetButtonConfig.bHasLed) {
                    APP_vBlinkLed(sDeviceConfig.sResetButtonConfig.u32LedMask, 5);
                }
                BDB_eNsStartNwkSteering();
                break;
            }
            else
            {
                tsZCL_ClusterInstance *psZCL_ClusterInstance;
                teZCL_Status eStatus = eZCL_SearchForClusterEntry(sDeviceConfig.u8BasicEndpoint, GENERAL_CLUSTER_ID_CONFIGURATION, TRUE, &psZCL_ClusterInstance);
                DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Search for cluster entry %d in endpoint %d status: %d\n", GENERAL_CLUSTER_ID_CONFIGURATION, sDeviceConfig.u8BasicEndpoint, eStatus);

                bool_t bPreventReset = ((tsCLD_Configuration *)psZCL_ClusterInstance->pvEndPointSharedStructPtr)->bPreventReset;
                if (bNodeJoined() && bPreventReset)
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
    DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Got button event %d for endpoint %d\n", sAppEvent->uEvent.sButton.eButtonAction, sAppEvent->uEvent.sButton.u8Endpoint);
    switch (sAppEvent->uEvent.sButton.eButtonAction)
    {
    case BUTTON_TOGGLE_ACTION:
        eSendCommand(sAppEvent->uEvent.sButton.u8Endpoint, E_CLD_ONOFF_CMD_TOGGLE);
        eReportAction(sAppEvent->uEvent.sButton.u8Endpoint, BUTTON_TOGGLE_ACTION);
        break;

    case BUTTON_MOMENTRAY_PRESSED_ACTION:
        eSendCommand(sAppEvent->uEvent.sButton.u8Endpoint, E_CLD_ONOFF_CMD_ON);
        eReportAction(sAppEvent->uEvent.sButton.u8Endpoint, BUTTON_MOMENTRAY_PRESSED_ACTION);
        break;

    case BUTTON_MOMENTARY_RELEASED_ACTION:
        eSendCommand(sAppEvent->uEvent.sButton.u8Endpoint, E_CLD_ONOFF_CMD_OFF);
        eReportAction(sAppEvent->uEvent.sButton.u8Endpoint, BUTTON_MOMENTARY_RELEASED_ACTION);
        break;

    case BUTTON_SINGLE_CLICK_ACTION:
    case BUTTON_DOUBLE_CLICK_ACTION:
    case BUTTON_TRIPLE_CLICK_ACTION:
    case BUTTON_LONG_PRESSED_ACTION:
    case BUTTON_LONG_RELEASED_ACTION:
        eReportAction(sAppEvent->uEvent.sButton.u8Endpoint, sAppEvent->uEvent.sButton.eButtonAction);
        break;

    default:
        break;
    }
}

PRIVATE teZCL_Status eSendCommand(uint8 u8Endpoint, teCLD_OnOff_Command eOnOffCommand)
{
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_BOUND_NO_ACK;
    DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Sending On/Off command status...\n");
    uint8 sequenceNo;
    teZCL_Status eStatus = eCLD_OnOffCommandSend(
        u8Endpoint,
        1,
        &addr,
        &sequenceNo,
        eOnOffCommand);
    DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Sending On/Off command status: %02x\n", eStatus);
    return eStatus;
}

PRIVATE teZCL_Status eReportAction(uint8 u8Endpoint, teButtonAction eButtonAction)
{
    teZCL_Status eStatus;
    tsZCL_ClusterInstance *psZCL_ClusterInstance;
    eStatus = eZCL_SearchForClusterEntry(u8Endpoint, GENERAL_CLUSTER_ID_MULTISTATE_INPUT_BASIC, TRUE, &psZCL_ClusterInstance);
    DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Search for cluster entry %d in endpoint %d status: %d\n", GENERAL_CLUSTER_ID_MULTISTATE_INPUT_BASIC, u8Endpoint, eStatus);

    ((tsCLD_MultistateInputBasic *)psZCL_ClusterInstance->pvEndPointSharedStructPtr)->u16PresentValue = (zuint16)eButtonAction;
    DBG_vPrintf(
        TRACE_EVENTS,
        "APP EVENTS: Reporting multistate action EP=%d value=%d...\n",
        u8Endpoint,
        ((tsCLD_MultistateInputBasic *)psZCL_ClusterInstance->pvEndPointSharedStructPtr)->u16PresentValue);

    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_SHORT_NO_ACK;
    PDUM_thAPduInstance myPDUM_thAPduInstance = hZCL_AllocateAPduInstance();
    eStatus = eZCL_ReportAttribute(
        &addr,
        GENERAL_CLUSTER_ID_MULTISTATE_INPUT_BASIC,
        E_CLD_MULTISTATE_INPUT_BASIC_ATTR_ID_PRESENT_VALUE,
        u8Endpoint,
        1,
        myPDUM_thAPduInstance);
    PDUM_eAPduFreeAPduInstance(myPDUM_thAPduInstance);
    DBG_vPrintf(TRACE_EVENTS, "APP EVENTS: Sending report status: %02x\n", eStatus);
    return eStatus;
}
