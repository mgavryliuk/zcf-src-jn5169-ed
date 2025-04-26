#include <jendefs.h>

#include "dbg.h"
#include "bdb_api.h"
#include "zcl.h"
#include "zps_apl_af.h"
#include "zps_nwk_nib.h"
#include "AppHardwareApi.h"
#include "PDM.h"

#include "pdum_gen.h"

#include "device_config.h"
#include "ConfigurationCluster.h"
#include "app_basic_endpoint.h"
#include "app_node.h"
#include "app_on_off_endpoint.h"
#include "app_polling.h"
#include "app_reporting.h"
#include "app_resources.h"

PRIVATE teNodeState eNodeState;

PRIVATE void vInitialiseZCL(void);
PRIVATE void cbGeneralZCLCallback(tsZCL_CallBackEvent *psEvent);
PRIVATE void vInitialiseBDB(void);
PRIVATE void vHandleNetworkJoinAndRejoin(void);
PRIVATE void vHandleNeworkJoinFailed(void);
PRIVATE void vHandleAFEvent(BDB_tsZpsAfEvent *psZpsAfEvent);
PRIVATE void vHandleZDOEvents(BDB_tsZpsAfEvent *psZpsAfEvent);

PUBLIC void APP_vInitialiseNode(void)
{
    uint16 u16ByteRead;
    eNodeState = E_NO_NETWORK;
    PDM_eReadDataFromRecord(PDM_ID_APP_END_DEVICE, &eNodeState, sizeof(teNodeState), &u16ByteRead);
    DBG_vPrintf(TRACE_NODE, "NODE: Status - %d\n", eNodeState);
    vInitialiseZCL();
    vMakeSupportedAttributesReportable();
    ZPS_eAplAfInit();
    ZPS_bAplAfSetEndDeviceTimeout(ZED_TIMEOUT_8192_MIN);
    vInitialiseBDB();
}

PUBLIC bool_t bNodeJoined(void)
{
    return eNodeState == E_JOINED;
}

PUBLIC teNodeState eGetNodeState(void)
{
    return eNodeState;
}

PUBLIC void APP_vBdbCallback(BDB_tsBdbEvent *psBdbEvent)
{
    switch (psBdbEvent->eEventType)
    {
    case BDB_EVENT_NONE:
        break;

    case BDB_EVENT_ZPSAF:
        vHandleAFEvent(&psBdbEvent->uEventData.sZpsAfEvent);
        break;

    case BDB_EVENT_INIT_SUCCESS:
        DBG_vPrintf(TRACE_NODE, "BDB CALLBACK: BDB_EVENT_INIT_SUCCESS\n");
        break;

    case BDB_EVENT_REJOIN_FAILURE:
        DBG_vPrintf(TRACE_NODE, "BDB CALLBACK: BDB_EVENT_REJOIN_FAILURE\n");
        vHandleNeworkJoinFailed();
        break;

    case BDB_EVENT_REJOIN_SUCCESS:
        DBG_vPrintf(TRACE_NODE, "BDB CALLBACK: BDB_EVENT_REJOIN_SUCCESS\n");
        vHandleNetworkJoinAndRejoin();
        APP_vStartPolling(POLL_FAST);
        break;

    case BDB_EVENT_NWK_STEERING_SUCCESS:
        DBG_vPrintf(TRACE_NODE, "BDB CALLBACK: BDB_EVENT_NWK_STEERING_SUCCESS\n");
        vHandleNetworkJoinAndRejoin();
        break;

    case BDB_EVENT_NO_NETWORK:
        DBG_vPrintf(TRACE_NODE, "BDB CALLBACK: BDB_EVENT_NO_NETWORK\n");
        vHandleNeworkJoinFailed();
        break;

    case BDB_EVENT_APP_START_POLLING:
        DBG_vPrintf(TRACE_NODE, "BDB CALLBACK: BDB_EVENT_APP_START_POLLING\n");
        APP_vStartPolling(POLL_COMMISSIONING);
        break;

    default:
        DBG_vPrintf(TRACE_NODE, "BDB CALLBACK: evt %d", psBdbEvent->eEventType);
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

    vResetConfigurationCluster();
    DBG_vPrintf(TRACE_NODE, "NODE: Factory Reset - Finished. Reseting device...\n");
}


PRIVATE void vInitialiseZCL(void)
{
    teZCL_Status eZCL_Status;
    eZCL_Status = eZCL_Initialise(&cbGeneralZCLCallback, apduZCL);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_NODE, "NODE: eZLO_Initialise failed with status :%d\n", eZCL_Status);
    }

    APP_vRegisterBasicEndPoint();
    APP_vRegisterOnOffEndPoints();
    ZTIMER_eStart(u8TimerZclTick, ZCL_TICK_TIME_MSEC);
}

PRIVATE void cbGeneralZCLCallback(tsZCL_CallBackEvent *psEvent)
{
    DBG_vPrintf(TRACE_NODE, "NODE: cbGeneralZCLCallback event type: %d\n", psEvent->eEventType);
}

PRIVATE void vInitialiseBDB(void)
{

    DBG_vPrintf(TRACE_NODE, "NODE: vInitialiseBDB\n");
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


PRIVATE void vHandleNetworkJoinAndRejoin(void)
{
    DBG_vPrintf(TRACE_NODE, "NODE: Device successfully joined network. Saving state to PDM\n");
    eNodeState = E_JOINED;
    PDM_eSaveRecordData(PDM_ID_APP_END_DEVICE, &eNodeState, sizeof(teNodeState));
    ZPS_vSaveAllZpsRecords();
}

PRIVATE void vHandleNeworkJoinFailed(void)
{
    DBG_vPrintf(TRACE_NODE, "NODE: Device failed to join the network\n");
}


PRIVATE void vHandleAFEvent(BDB_tsZpsAfEvent *psZpsAfEvent)
{
    if (psZpsAfEvent->u8EndPoint == sDeviceConfig.u8BasicEndpoint)
    {
        DBG_vPrintf(TRACE_NODE, "AF CALLBACK: Basic endpoint event received\n");
        tsZCL_CallBackEvent sCallBackEvent;
        sCallBackEvent.pZPSevent = &psZpsAfEvent->sStackEvent;
        sCallBackEvent.eEventType = E_ZCL_CBET_ZIGBEE_EVENT;
        vZCL_EventHandler(&sCallBackEvent);
    }
    else if (psZpsAfEvent->u8EndPoint == sDeviceConfig.u8ZdoEndpoint)
    {
        DBG_vPrintf(TRACE_NODE, "AF CALLBACK: ZDO endpoint event received\n");
        vHandleZDOEvents(psZpsAfEvent);
    }

    if (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        DBG_vPrintf(TRACE_NODE, "AF CALLBACK: ZPS_EVENT_APS_DATA_INDICATION. Freeing APduInstance\n");
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
    }
    else if (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION)
    {
        DBG_vPrintf(TRACE_NODE, "AF CALLBACK: ZPS_EVENT_APS_INTERPAN_DATA_INDICATION. Freeing APduInstance\n");
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);
    }
}

PRIVATE void vHandleZDOEvents(BDB_tsZpsAfEvent *psZpsAfEvent)
{
    ZPS_tsAfEvent *psAfEvent = &(psZpsAfEvent->sStackEvent);
    switch (psAfEvent->eType)
    {
    case ZPS_EVENT_APS_DATA_INDICATION:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Data Indication Status %02x from %04x Src Ep Dst %d Ep %d Profile %04x Cluster %04x\n",
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
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Joined Network Addr %04x Rejoin %d\n",
                    psAfEvent->uEvent.sNwkJoinedEvent.u16Addr,
                    psAfEvent->uEvent.sNwkJoinedEvent.bRejoin);
        break;

    case ZPS_EVENT_NWK_FAILED_TO_JOIN:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Failed To Join 0x%02x Rejoin %d\n",
                    psAfEvent->uEvent.sNwkJoinFailedEvent.u8Status,
                    psAfEvent->uEvent.sNwkJoinFailedEvent.bRejoin);
        break;

    case ZPS_EVENT_NWK_DISCOVERY_COMPLETE:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Discovery Complete 0x%02x\n", psAfEvent->uEvent.sNwkDiscoveryEvent.eStatus);
        break;

    case ZPS_EVENT_NWK_LEAVE_INDICATION:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Leave Indication %016llx Rejoin %d\n",
                    psAfEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr,
                    psAfEvent->uEvent.sNwkLeaveIndicationEvent.u8Rejoin);
        if ((psAfEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr == 0UL) &&
            (psAfEvent->uEvent.sNwkLeaveIndicationEvent.u8Rejoin == 0))
        {
            DBG_vPrintf(TRACE_NODE, "APP-ZDO: LEAVE IND -> For Us No Rejoin\n");
            APP_vFactoryResetRecords();
            vAHI_SwReset();
        }
        break;

    case ZPS_EVENT_NWK_LEAVE_CONFIRM:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Leave Confirm status %02x Addr %016llx\n",
                    psAfEvent->uEvent.sNwkLeaveConfirmEvent.eStatus,
                    psAfEvent->uEvent.sNwkLeaveConfirmEvent.u64ExtAddr);
        if (psAfEvent->uEvent.sNwkLeaveConfirmEvent.u64ExtAddr == 0UL)
        {
            DBG_vPrintf(TRACE_NODE, "APP-ZDO: Leave -> Reset Data Structures\n");
            APP_vFactoryResetRecords();
            vAHI_SwReset();
        }
        break;

    case ZPS_EVENT_NWK_STATUS_INDICATION:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Network status Indication %02x addr %04x\n",
                    psAfEvent->uEvent.sNwkStatusIndicationEvent.u8Status,
                    psAfEvent->uEvent.sNwkStatusIndicationEvent.u16NwkAddr);
        break;

    case ZPS_EVENT_NWK_POLL_CONFIRM:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO:  ZPS_EVENT_NWK_POLL_CONFIRM: %d\n", psAfEvent->uEvent.sNwkPollConfirmEvent.u8Status);
        APP_vHandlePollConfirm(&psAfEvent->uEvent.sNwkPollConfirmEvent);
        break;

    case ZPS_EVENT_NWK_ED_SCAN:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Energy Detect Scan %02x\n", psAfEvent->uEvent.sNwkEdScanConfirmEvent.u8Status);
        break;
    case ZPS_EVENT_ZDO_BIND:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Zdo Bind event\n");
        break;

    case ZPS_EVENT_ZDO_UNBIND:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Zdo Unbiind Event\n");
        break;

    case ZPS_EVENT_ZDO_LINK_KEY:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Zdo Link Key Event Type %d Addr %016llx\n",
                    psAfEvent->uEvent.sZdoLinkKeyEvent.u8KeyType,
                    psAfEvent->uEvent.sZdoLinkKeyEvent.u64IeeeLinkAddr);
        break;

    case ZPS_EVENT_BIND_REQUEST_SERVER:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Bind Request Server Event\n");
        break;

    case ZPS_EVENT_ERROR:
        break;

    case ZPS_EVENT_TC_STATUS:
        break;

    default:
        DBG_vPrintf(TRACE_NODE, "APP-ZDO: Unhandled Event %d\n", psAfEvent->eType);
        break;
    }
}
