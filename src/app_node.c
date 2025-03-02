#include <jendefs.h>
#include "bdb_api.h"
#include "dbg.h"
#include "zcl.h"
#include "zps_apl_af.h"
#include "AppHardwareApi.h"
#include "PDM.h"
#include "app_basic_endpoint.h"
#include "app_main.h"
#include "app_node.h"
#include "app_on_off_endpoint.h"
#include "pdum_gen.h"
#include "zps_gen.h"

PRIVATE void vAppHandleAfEvent(BDB_tsZpsAfEvent *psZpsAfEvent);
PRIVATE void vAppHandleZdoEvents(BDB_tsZpsAfEvent *psZpsAfEvent);
PRIVATE void vAppFactoryResetRecords(void);
PRIVATE void APP_ZCL_vInitialise(void);
PRIVATE void APP_vBdbInit(void);

PRIVATE teNodeState eNodeState;

PRIVATE void APP_ZCL_cbGeneralCallback(tsZCL_CallBackEvent *psEvent) {}

PUBLIC void APP_vInitialiseNode(void)
{
    uint16 u16ByteRead;
    eNodeState = E_STARTUP;
    PDM_eSaveRecordData(PDM_ID_APP_END_DEVICE, &eNodeState, sizeof(teNodeState));
    PDM_eReadDataFromRecord(PDM_ID_APP_END_DEVICE, &eNodeState, sizeof(teNodeState), &u16ByteRead);
    ZPS_eAplAfInit();
    APP_ZCL_vInitialise();
    APP_vBdbInit();
}

PRIVATE void APP_ZCL_vInitialise(void)
{
    teZCL_Status eZCL_Status;

    /* Initialise ZLL */
    eZCL_Status = eZCL_Initialise(&APP_ZCL_cbGeneralCallback, apduZCL);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRUE, "\nErr: eZLO_Initialise:%d\n", eZCL_Status);
    }

    APP_vRegisterBasicEndPoint();
    APP_vRegisterOnOffEndPoints();
}

PUBLIC void APP_vBdbCallback(BDB_tsBdbEvent *psBdbEvent)
{
    switch(psBdbEvent->eEventType)
    {
        case BDB_EVENT_NONE:
            break;
        case BDB_EVENT_ZPSAF:
            vAppHandleAfEvent(&psBdbEvent->uEventData.sZpsAfEvent);
            break;

        case BDB_EVENT_INIT_SUCCESS:
            DBG_vPrintf(TRUE,"APP: BDB_EVENT_INIT_SUCCESS\n");
            break;

        case BDB_EVENT_REJOIN_FAILURE:
            DBG_vPrintf(TRUE,"APP: BDB_EVENT_REJOIN_FAILURE\n");
            // bBDBJoinFailed = TRUE;
            break;

        case BDB_EVENT_REJOIN_SUCCESS:
            eNodeState = E_RUNNING;
            // vStartPolling();
            // bBDBJoinFailed = FALSE;
            DBG_vPrintf(TRUE,"APP: BDB_EVENT_REJOIN_SUCCESS\n");
            break;

        case BDB_EVENT_NWK_STEERING_SUCCESS:
            DBG_vPrintf(TRUE, "APP: NwkSteering Success 0x%016llx\n",ZPS_psAplAibGetAib()->u64ApsUseExtendedPanid);
            ZPS_vSaveAllZpsRecords();
            PDM_eSaveRecordData(PDM_ID_APP_END_DEVICE, &eNodeState, sizeof(teNodeState));
            // vStartPolling();
            break;

        case BDB_EVENT_APP_START_POLLING:
            DBG_vPrintf(TRUE,"APP: Start polling\n");
            // vStartPolling();
            break;
        
        case BDB_EVENT_NO_NETWORK:
            DBG_vPrintf(TRUE, "BDB event callback: No good network to join\n");
            break;
        
        default:
            DBG_vPrintf(TRUE, "BDB event callback: evt %d\n", psBdbEvent->eEventType);
            break;
    }
}

PRIVATE void APP_vBdbInit(void)
{
    BDB_tsInitArgs sInitArgs;

    sBDB.sAttrib.bbdbNodeIsOnANetwork = ((eNodeState >= E_RUNNING)? (TRUE): (FALSE));
    if(sBDB.sAttrib.bbdbNodeIsOnANetwork)
    {
        DBG_vPrintf(TRUE, "APP: NFN Start\n");
        // bFailToJoin = TRUE;
        // ZTIMER_eStart(u8TimerPoll, ZTIMER_TIME_MSEC(1000) );
    }
    else
    {
        DBG_vPrintf(TRUE, "APP: FN Start\n");
        // u8KeepAliveTime = KEEP_ALIVE_FACTORY_NEW;
    }
    // ZTIMER_eStart(u8TimerPoll, ZTIMER_TIME_MSEC(1000) );
    sInitArgs.hBdbEventsMsgQ = &APP_msgBdbEvents;
    BDB_vInit(&sInitArgs);
}

PRIVATE void vAppHandleAfEvent(BDB_tsZpsAfEvent *psZpsAfEvent)
{
    if (psZpsAfEvent->u8EndPoint == WXKG07LM_ALT_BASIC_ENDPOINT)
    {
        if (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
        DBG_vPrintf(TRUE, "BDB_tsZpsAfEvent: WXKG07LM_ALT_BASIC_ENDPOINT && ZPS_EVENT_APS_DATA_INDICATION\n");
        {
            tsZCL_CallBackEvent sCallBackEvent;
            sCallBackEvent.pZPSevent = &psZpsAfEvent->sStackEvent;
            sCallBackEvent.eEventType = E_ZCL_CBET_ZIGBEE_EVENT;
            vZCL_EventHandler(&sCallBackEvent);
        }
    }
    else if(psZpsAfEvent->u8EndPoint == WXKG07LM_ALT_ZDO_ENDPOINT)
    {
        vAppHandleZdoEvents(psZpsAfEvent);
    }

    /* Ensure Freeing of Apdus */
    if (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
    }
    else if ( psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION )
    {
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);
    }
}

PRIVATE void vAppHandleZdoEvents(BDB_tsZpsAfEvent *psZpsAfEvent)
{
    ZPS_tsAfEvent *psAfEvent = &(psZpsAfEvent->sStackEvent);
    switch(psAfEvent->eType)
    {
        case ZPS_EVENT_APS_DATA_INDICATION:
            DBG_vPrintf(TRUE, "APP-ZDO: Data Indication Status %02x from %04x Src Ep Dst %d Ep %d Profile %04x Cluster %04x\n",
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
            DBG_vPrintf(TRUE, "APP-ZDO: Joined Network Addr %04x Rejoin %d\n",
                    psAfEvent->uEvent.sNwkJoinedEvent.u16Addr,
                    psAfEvent->uEvent.sNwkJoinedEvent.bRejoin);

            ZPS_eAplAibSetApsUseExtendedPanId(ZPS_u64NwkNibGetEpid(ZPS_pvAplZdoGetNwkHandle()));

            eNodeState = E_RUNNING;
            PDM_eSaveRecordData(PDM_ID_APP_END_DEVICE, &eNodeState, sizeof(teNodeState));
            break;

        case ZPS_EVENT_NWK_FAILED_TO_JOIN:
            DBG_vPrintf(TRUE, "APP-ZDO: Failed To Join 0x%02x Rejoin %d\n",
                    psAfEvent->uEvent.sNwkJoinFailedEvent.u8Status,
                    psAfEvent->uEvent.sNwkJoinFailedEvent.bRejoin);
            if (ZPS_psAplAibGetAib()->u64ApsUseExtendedPanid != 0)
            {
                DBG_vPrintf(TRUE, "APP-ZDO: Restore epid %016llx\n", ZPS_psAplAibGetAib()->u64ApsUseExtendedPanid);
                ZPS_vNwkNibSetExtPanId(ZPS_pvAplZdoGetNwkHandle(), ZPS_psAplAibGetAib()->u64ApsUseExtendedPanid);
            }
            break;

        case ZPS_EVENT_NWK_DISCOVERY_COMPLETE:
            DBG_vPrintf(TRUE, "APP-ZDO: Discovery Complete 0x%02x\n", psAfEvent->uEvent.sNwkDiscoveryEvent.eStatus);
            break;

        case ZPS_EVENT_NWK_LEAVE_INDICATION:
            DBG_vPrintf(TRUE, "APP-ZDO: Leave Indication %016llx Rejoin %d\n",
                    psAfEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr,
                    psAfEvent->uEvent.sNwkLeaveIndicationEvent.u8Rejoin);
            if ( (psAfEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr == 0UL) &&
                 (psAfEvent->uEvent.sNwkLeaveIndicationEvent.u8Rejoin == 0) )
            {
                DBG_vPrintf(TRUE, "APP-ZDO: LEAVE IND -> For Us No Rejoin\n");
                vAppFactoryResetRecords();
                vAHI_SwReset();
            }
            break;

        case ZPS_EVENT_NWK_LEAVE_CONFIRM:
            DBG_vPrintf(TRUE, "APP-ZDO: Leave Confirm status %02x Addr %016llx\n",
                    psAfEvent->uEvent.sNwkLeaveConfirmEvent.eStatus,
                    psAfEvent->uEvent.sNwkLeaveConfirmEvent.u64ExtAddr);
            if (( psAfEvent->uEvent.sNwkLeaveConfirmEvent.eStatus == ZPS_E_SUCCESS) &&
                ( psAfEvent->uEvent.sNwkLeaveConfirmEvent.u64ExtAddr == 0UL))
            {
                DBG_vPrintf(TRUE, "APP-ZDO: Leave -> Reset Data Structures\n");
                vAppFactoryResetRecords();
                vAHI_SwReset();
            }
            break;

        case ZPS_EVENT_NWK_STATUS_INDICATION:
            DBG_vPrintf(TRUE, "APP-ZDO: Network status Indication %02x addr %04x\n",
                    psAfEvent->uEvent.sNwkStatusIndicationEvent.u8Status,
                    psAfEvent->uEvent.sNwkStatusIndicationEvent.u16NwkAddr);
            break;

        case ZPS_EVENT_NWK_POLL_CONFIRM:
            DBG_vPrintf(TRUE, "APP-ZDO:  ZPS_EVENT_NWK_POLL_CONFIRM\n");
            break;

        case ZPS_EVENT_NWK_ED_SCAN:
            DBG_vPrintf(TRUE, "APP-ZDO: Energy Detect Scan %02x\n", psAfEvent->uEvent.sNwkEdScanConfirmEvent.u8Status);
            break;
        case ZPS_EVENT_ZDO_BIND:
            DBG_vPrintf(TRUE, "APP-ZDO: Zdo Bind event\n");
            break;

        case ZPS_EVENT_ZDO_UNBIND:
            DBG_vPrintf(TRUE, "APP-ZDO: Zdo Unbiind Event\n");
            break;

        case ZPS_EVENT_ZDO_LINK_KEY:
            DBG_vPrintf(TRUE, "APP-ZDO: Zdo Link Key Event Type %d Addr %016llx\n",
                        psAfEvent->uEvent.sZdoLinkKeyEvent.u8KeyType,
                        psAfEvent->uEvent.sZdoLinkKeyEvent.u64IeeeLinkAddr);
            break;

        case ZPS_EVENT_BIND_REQUEST_SERVER:
            DBG_vPrintf(TRUE, "APP-ZDO: Bind Request Server Event\n");
            break;

        case ZPS_EVENT_ERROR:
            break;

        case ZPS_EVENT_TC_STATUS:
            break;

        default:
            DBG_vPrintf(TRUE, "APP-ZDO: Unhandled Event %d\n", psAfEvent->eType);
            break;
    }
}

PRIVATE void vAppFactoryResetRecords(void)
{
    /* clear out the stack */
    ZPS_vDefaultStack();
    ZPS_eAplAibSetApsUseExtendedPanId(0);
    ZPS_vSetKeys();
    eNodeState = E_STARTUP;
    PDM_eSaveRecordData(PDM_ID_APP_END_DEVICE, &eNodeState, sizeof(teNodeState));
    ZPS_vSaveAllZpsRecords();
}
