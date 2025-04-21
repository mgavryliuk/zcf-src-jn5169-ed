#include <jendefs.h>
#include "dbg.h"
#include "bdb_api.h"
#include "pdum_gen.h"
#include "zcl.h"
#include "zps_apl_af.h"
#include "zps_gen.h"
#include "AppHardwareApi.h"
#include "PDM.h"

#include "app_device.h"
#include "app_basic_endpoint.h"
#include "app_main.h"
#include "app_node.h"
#include "app_on_off_endpoint.h"
#include "app_polling.h"
#include "app_reporting.h"

#define MAX_JOIN_ATTEMPTS 5
#define JOIN_ATTEMPTS_DELAY 5

PRIVATE teNodeState eNodeState;

PRIVATE void vAppHandleAfEvent(BDB_tsZpsAfEvent *psZpsAfEvent);
PRIVATE void vAppHandleZdoEvents(BDB_tsZpsAfEvent *psZpsAfEvent);
PRIVATE void APP_ZCL_vInitialise(void);
PRIVATE void APP_vBdbInit(void);
PRIVATE void handleNetworkJoinAndRejoin(void);
PRIVATE void handleNeworkJoinFailed(void);

PRIVATE void APP_ZCL_cbGeneralCallback(tsZCL_CallBackEvent *psEvent) {}

PUBLIC teNodeState eGetNodeState(void)
{
    return eNodeState;
}

PUBLIC bool_t bNodeJoined(void)
{
    return (eGetNodeState() == E_JOINED) ? TRUE : FALSE;
}

PUBLIC void APP_vInitialiseNode(void)
{
    uint16 u16ByteRead;
    eNodeState = E_NO_NETWORK;
    PDM_eReadDataFromRecord(PDM_ID_APP_END_DEVICE, &eNodeState, sizeof(teNodeState), &u16ByteRead);
    DBG_vPrintf(TRACE_NODE, "NODE: Status - %d\n", eNodeState);
    APP_ZCL_vInitialise();
    ZPS_eAplAfInit();
    ZPS_bAplAfSetEndDeviceTimeout(ZED_TIMEOUT_8192_MIN);
    vMakeSupportedAttributesReportable();
    APP_vBdbInit();
}

PRIVATE void APP_ZCL_vInitialise(void)
{
    teZCL_Status eZCL_Status;
    eZCL_Status = eZCL_Initialise(&APP_ZCL_cbGeneralCallback, apduZCL);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_NODE, "NODE: eZLO_Initialise failed with status :%d\n", eZCL_Status);
    }

    APP_vRegisterBasicEndPoint();
    APP_vRegisterOnOffEndPoints();
    ZTIMER_eStart(u8TimerTick, ZCL_TICK_TIME);
}

PRIVATE void APP_vBdbInit(void)
{

    DBG_vPrintf(TRACE_NODE, "NODE: APP_vBdbInit\n");
    BDB_tsInitArgs sInitArgs;
    if (bNodeJoined())
    {
        DBG_vPrintf(TRACE_NODE, "NODE: Device is in network. Changing status to E_JOINING...\n");
        eNodeState = E_JOINING;
        sBDB.sAttrib.bbdbNodeIsOnANetwork = TRUE;
    }
    sInitArgs.hBdbEventsMsgQ = &APP_msgBdbEvents;
    BDB_vInit(&sInitArgs);
}

PUBLIC void APP_vBdbCallback(BDB_tsBdbEvent *psBdbEvent)
{
    switch (psBdbEvent->eEventType)
    {
    case BDB_EVENT_NONE:
        break;

    case BDB_EVENT_ZPSAF:
        vAppHandleAfEvent(&psBdbEvent->uEventData.sZpsAfEvent);
        break;

    case BDB_EVENT_INIT_SUCCESS:
        DBG_vPrintf(TRACE_NODE_BDB, "NODE CALLBACK: BDB_EVENT_INIT_SUCCESS\n");
        break;

    case BDB_EVENT_REJOIN_FAILURE:
        DBG_vPrintf(TRACE_NODE_BDB, "NODE CALLBACK: BDB_EVENT_REJOIN_FAILURE\n");
        handleNeworkJoinFailed();
        break;

    case BDB_EVENT_REJOIN_SUCCESS:
        DBG_vPrintf(TRACE_NODE_BDB, "NODE CALLBACK: BDB_EVENT_REJOIN_SUCCESS\n");
        handleNetworkJoinAndRejoin();
        APP_vStartPolling(POLL_FAST);
        break;

    case BDB_EVENT_NWK_STEERING_SUCCESS:
        DBG_vPrintf(TRACE_NODE_BDB, "NODE CALLBACK: BDB_EVENT_NWK_STEERING_SUCCESS\n");
        handleNetworkJoinAndRejoin();
        break;

    case BDB_EVENT_NO_NETWORK:
        DBG_vPrintf(TRACE_NODE_BDB, "NODE CALLBACK: BDB_EVENT_NO_NETWORK\n");
        handleNeworkJoinFailed();
        break;

    case BDB_EVENT_APP_START_POLLING:
        DBG_vPrintf(TRUE, "NODE CALLBACK: BDB_EVENT_APP_START_POLLING\n");
        APP_vStartPolling(POLL_COMMISSIONING);
        break;

    default:
        DBG_vPrintf(TRACE_NODE_BDB, "NODE CALLBACK: evt %d", psBdbEvent->eEventType);
        break;
    }
}

PRIVATE void vAppHandleAfEvent(BDB_tsZpsAfEvent *psZpsAfEvent)
{
    if (psZpsAfEvent->u8EndPoint == BASIC_ENDPOINT)
    {
        DBG_vPrintf(TRACE_NODE_AF_CB, "NODE AF CALLBACK: Event endpoint: BASIC_ENDPOINT\n");
        tsZCL_CallBackEvent sCallBackEvent;
        sCallBackEvent.pZPSevent = &psZpsAfEvent->sStackEvent;
        sCallBackEvent.eEventType = E_ZCL_CBET_ZIGBEE_EVENT;
        vZCL_EventHandler(&sCallBackEvent);
    }
    else if (psZpsAfEvent->u8EndPoint == ZDO_ENDPOINT)
    {
        DBG_vPrintf(TRACE_NODE_AF_CB, "NODE AF CALLBACK: Events endpoint: ZDO_ENDPOINT\n");
        vAppHandleZdoEvents(psZpsAfEvent);
    }

    if (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        DBG_vPrintf(TRACE_NODE_AF_CB, "NODE AF CALLBACK: ZPS_EVENT_APS_DATA_INDICATION. Freeing APduInstance\n");
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
    }
    else if (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION)
    {
        DBG_vPrintf(TRACE_NODE_AF_CB, "NODE AF CALLBACK: ZPS_EVENT_APS_INTERPAN_DATA_INDICATION. Freeing APduInstance\n");
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);
    }
}

PRIVATE void vAppHandleZdoEvents(BDB_tsZpsAfEvent *psZpsAfEvent)
{
    ZPS_tsAfEvent *psAfEvent = &(psZpsAfEvent->sStackEvent);
    switch (psAfEvent->eType)
    {
    case ZPS_EVENT_APS_DATA_INDICATION:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Data Indication Status %02x from %04x Src Ep Dst %d Ep %d Profile %04x Cluster %04x\n",
                    psAfEvent->uEvent.sApsDataIndEvent.eStatus,
                    psAfEvent->uEvent.sApsDataIndEvent.uSrcAddress.u16Addr,
                    psAfEvent->uEvent.sApsDataIndEvent.u8SrcEndpoint,
                    psAfEvent->uEvent.sApsDataIndEvent.u8DstEndpoint,
                    psAfEvent->uEvent.sApsDataIndEvent.u16ProfileId,
                    psAfEvent->uEvent.sApsDataIndEvent.u16ClusterId);
        break;

    case ZPS_EVENT_APS_DATA_CONFIRM:
        break;

    case ZPS_EVENT_APS_DATA_ACK:
        break;

    case ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Joined Network Addr %04x Rejoin %d\n",
                    psAfEvent->uEvent.sNwkJoinedEvent.u16Addr,
                    psAfEvent->uEvent.sNwkJoinedEvent.bRejoin);
        break;

    case ZPS_EVENT_NWK_FAILED_TO_JOIN:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Failed To Join 0x%02x Rejoin %d\n",
                    psAfEvent->uEvent.sNwkJoinFailedEvent.u8Status,
                    psAfEvent->uEvent.sNwkJoinFailedEvent.bRejoin);
        break;

    case ZPS_EVENT_NWK_DISCOVERY_COMPLETE:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Discovery Complete 0x%02x\n", psAfEvent->uEvent.sNwkDiscoveryEvent.eStatus);
        break;

    case ZPS_EVENT_NWK_LEAVE_INDICATION:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Leave Indication %016llx Rejoin %d\n",
                    psAfEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr,
                    psAfEvent->uEvent.sNwkLeaveIndicationEvent.u8Rejoin);
        if ((psAfEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr == 0UL) &&
            (psAfEvent->uEvent.sNwkLeaveIndicationEvent.u8Rejoin == 0))
        {
            DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: LEAVE IND -> For Us No Rejoin\n");
            APP_vFactoryResetRecords();
            vAHI_SwReset();
        }
        break;

    case ZPS_EVENT_NWK_LEAVE_CONFIRM:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Leave Confirm status %02x Addr %016llx\n",
                    psAfEvent->uEvent.sNwkLeaveConfirmEvent.eStatus,
                    psAfEvent->uEvent.sNwkLeaveConfirmEvent.u64ExtAddr);
        if (psAfEvent->uEvent.sNwkLeaveConfirmEvent.u64ExtAddr == 0UL)
        {
            DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Leave -> Reset Data Structures\n");
            APP_vFactoryResetRecords();
            vAHI_SwReset();
        }
        break;

    case ZPS_EVENT_NWK_STATUS_INDICATION:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Network status Indication %02x addr %04x\n",
                    psAfEvent->uEvent.sNwkStatusIndicationEvent.u8Status,
                    psAfEvent->uEvent.sNwkStatusIndicationEvent.u16NwkAddr);
        break;

    case ZPS_EVENT_NWK_POLL_CONFIRM:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO:  ZPS_EVENT_NWK_POLL_CONFIRM: %d\n", psAfEvent->uEvent.sNwkPollConfirmEvent.u8Status);
        APP_vHandlePollConfirm(&psAfEvent->uEvent.sNwkPollConfirmEvent);
        break;

    case ZPS_EVENT_NWK_ED_SCAN:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Energy Detect Scan %02x\n", psAfEvent->uEvent.sNwkEdScanConfirmEvent.u8Status);
        break;
    case ZPS_EVENT_ZDO_BIND:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Zdo Bind event\n");
        break;

    case ZPS_EVENT_ZDO_UNBIND:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Zdo Unbiind Event\n");
        break;

    case ZPS_EVENT_ZDO_LINK_KEY:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Zdo Link Key Event Type %d Addr %016llx\n",
                    psAfEvent->uEvent.sZdoLinkKeyEvent.u8KeyType,
                    psAfEvent->uEvent.sZdoLinkKeyEvent.u64IeeeLinkAddr);
        break;

    case ZPS_EVENT_BIND_REQUEST_SERVER:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Bind Request Server Event\n");
        break;

    case ZPS_EVENT_ERROR:
        break;

    case ZPS_EVENT_TC_STATUS:
        break;

    default:
        DBG_vPrintf(TRACE_NODE_ZDO, "APP-ZDO: Unhandled Event %d\n", psAfEvent->eType);
        break;
    }
}

PUBLIC void APP_vFactoryResetRecords(void)
{
    DBG_vPrintf(TRACE_NODE, "NODE: Factory Reset\n");
    ZPS_eAplAibSetApsUseExtendedPanId(0);
    ZPS_vDefaultStack();
    ZPS_vSetKeys();
    eNodeState = E_NO_NETWORK;
    PDM_eSaveRecordData(PDM_ID_APP_END_DEVICE, &eNodeState, sizeof(teNodeState));
    ZPS_vSaveAllZpsRecords();

    resetConfigurationCluster();
    DBG_vPrintf(TRACE_NODE, "NODE: Factory Reset - Finished. Reseting device...\n");
}

PUBLIC void APP_cbTimerZclTick(void *pvParam)
{

    DBG_vPrintf(TRACE_NODE, "NODE: APP_cbTimerZclTick\n");
    ZPS_tsAfEvent sStackEvent;
    tsZCL_CallBackEvent sCallBackEvent;
    sCallBackEvent.pZPSevent = &sStackEvent;
    sCallBackEvent.eEventType = E_ZCL_CBET_TIMER;
    vZCL_EventHandler(&sCallBackEvent);
    ZTIMER_eStart(u8TimerTick, ZCL_TICK_TIME);
}

PRIVATE void handleNetworkJoinAndRejoin(void)
{
    DBG_vPrintf(TRACE_NODE, "NODE: Device successfully joined network. Saving state to PDM\n");
    eNodeState = E_JOINED;
    PDM_eSaveRecordData(PDM_ID_APP_END_DEVICE, &eNodeState, sizeof(teNodeState));
    ZPS_vSaveAllZpsRecords();
}

PRIVATE void handleNeworkJoinFailed(void)
{
    DBG_vPrintf(TRACE_NODE, "NODE: Device failed to join the network\n");
}
