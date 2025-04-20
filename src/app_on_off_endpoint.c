#include <jendefs.h>
#include "dbg.h"
#include "zps_gen.h"
#include "Groups.h"
#include "OnOff.h"
#include "MultistateInputBasic.h"

#include "app_on_off_endpoint.h"
#include "app_button.h"

tsZHA_OnOffEndpoint tsOnOffLeftButtonEndpoint;
tsZHA_OnOffEndpoint tsOnOffRightButtonEndpoint;

PRIVATE void APP_cbOnOffEndpointCallback(tsZCL_CallBackEvent *psEvent);

PRIVATE void APP_vRegisterOnOffEndPoint(uint8 u8EndPoint, tsZHA_OnOffEndpoint *tsEndpoint)
{
    DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: Registering endpoint: %d\n", u8EndPoint);
    tsEndpoint->sEndPoint.u8EndPointNumber = u8EndPoint;
    tsEndpoint->sEndPoint.u16ManufacturerCode = ZCL_MANUFACTURER_CODE;
    tsEndpoint->sEndPoint.u16ProfileEnum = HA_PROFILE_ID;
    tsEndpoint->sEndPoint.bIsManufacturerSpecificProfile = FALSE;
    tsEndpoint->sEndPoint.u16NumberOfClusters = sizeof(tsZHA_OnOffEndpointClusterInstances) / sizeof(tsZCL_ClusterInstance);
    tsEndpoint->sEndPoint.psClusterInstance = (tsZCL_ClusterInstance *)&tsEndpoint->sClusterInstance;
    tsEndpoint->sEndPoint.bDisableDefaultResponse = ZCL_DISABLE_DEFAULT_RESPONSES;
    tsEndpoint->sEndPoint.pCallBackFunctions = &APP_cbOnOffEndpointCallback;

    teZCL_Status eZCL_Status = eCLD_OnOffCreateOnOff(
        &tsEndpoint->sClusterInstance.sOnOffClient,
        FALSE,
        &sCLD_OnOffClient,
        &tsEndpoint->sOnOffClientCluster,
        &au8OnOffClientAttributeControlBits[0],
        NULL);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: eCLD_OnOffCreateOnOff failed with status: %d\n", eZCL_Status);
    }

    eZCL_Status = eCLD_MultistateInputBasicCreateMultistateInputBasic(
        &tsEndpoint->sClusterInstance.sMultistateInputServer,
        TRUE,
        &sCLD_MultistateInputBasic,
        &tsEndpoint->sMultistateInputServerCluster,
        &au8MultistateInputBasicAttributeControlBits[0]);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: eCLD_MultistateInputBasicCreateMultistateInputBasic failed with status: %d\n", eZCL_Status);
    }

    eZCL_Status = eCLD_GroupsCreateGroups(
        &tsEndpoint->sClusterInstance.sGroupsClient,
        FALSE,
        &sCLD_Groups,
        &tsEndpoint->sGroupsClientCluster,
        &au8GroupsAttributeControlBits[0],
        &tsEndpoint->sGroupsClientCustomDataStructure,
        &tsEndpoint->sEndPoint);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: eCLD_GroupsCreateGroups failed with status: %d\n", eZCL_Status);
    }

    eZCL_Status = eZCL_Register(&tsEndpoint->sEndPoint);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: eZCL_Register failed with status: %d\n", eZCL_Status);
    }
}

PUBLIC void APP_vRegisterOnOffEndPoints(void)
{
    APP_vRegisterOnOffEndPoint(WXKG07LM_LEFTBUTTON_ENDPOINT, &tsOnOffLeftButtonEndpoint);
#ifdef TARGET_WXKG07LM
    APP_vRegisterOnOffEndPoint(WXKG07LM_RIGHTBUTTON_ENDPOINT, &tsOnOffRightButtonEndpoint);
#endif
}

PUBLIC teZCL_Status eSendCommand(uint16 u16Endpoint, teCLD_OnOff_Command eOnOffCommand)
{
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_BOUND_NO_ACK;
    DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: Sending On/Off command status...\n");
    uint8 sequenceNo;
    teZCL_Status eStatus = eCLD_OnOffCommandSend(
        u16Endpoint,
        1,
        &addr,
        &sequenceNo,
        eOnOffCommand);
    DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: Sending On/Off command status: %02x\n", eStatus);
    return eStatus;
}

PUBLIC teZCL_Status eReportAction(uint16 u16Endpoint, teButtonAction eButtonAction)
{
    tsZHA_OnOffEndpoint *tsButtonEndpoint;

    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_SHORT_NO_ACK;

    if (u16Endpoint == WXKG07LM_LEFTBUTTON_ENDPOINT)
    {
        tsButtonEndpoint = &tsOnOffLeftButtonEndpoint;
    }
    else
    {
        tsButtonEndpoint = &tsOnOffRightButtonEndpoint;
    }
    tsButtonEndpoint->sMultistateInputServerCluster.u16PresentValue = (zuint16)eButtonAction;

    DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: Reporting multistate action EP=%d value=%d...\n", u16Endpoint, tsButtonEndpoint->sMultistateInputServerCluster.u16PresentValue);
    PDUM_thAPduInstance myPDUM_thAPduInstance = hZCL_AllocateAPduInstance();
    teZCL_Status eStatus = eZCL_ReportAttribute(
        &addr,
        GENERAL_CLUSTER_ID_MULTISTATE_INPUT_BASIC,
        E_CLD_MULTISTATE_INPUT_BASIC_ATTR_ID_PRESENT_VALUE,
        u16Endpoint,
        1,
        myPDUM_thAPduInstance);
    PDUM_eAPduFreeAPduInstance(myPDUM_thAPduInstance);
    DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: Sending report status: %02x\n", eStatus);
    return eStatus;
}

PRIVATE void APP_cbOnOffEndpointCallback(tsZCL_CallBackEvent *psEvent)
{
    switch (psEvent->eEventType)
    {
    default:
        DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: Invalid event type (%d) in APP_ZCL_cbEndpointCallback\n", psEvent->eEventType);
        break;
    }
}
