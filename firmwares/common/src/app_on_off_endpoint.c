#include <jendefs.h>

#include "dbg.h"
#include "zps_gen.h"
#include "Groups.h"
#include "OnOff.h"
#include "MultistateInputBasic.h"

#include "device_config.h"
#include "app_on_off_endpoint.h"

PRIVATE tsZHA_OnOffEndpoint tsOnOffButtonEndpoints[BUTTON_COUNT];

PRIVATE void APP_cbOnOffEndpointCallback(tsZCL_CallBackEvent *psEvent);
PRIVATE void APP_vRegisterOnOffEndPoint(uint8 u8EndPoint, tsZHA_OnOffEndpoint *tsEndpoint);

PUBLIC void APP_vRegisterOnOffEndPoints(void)
{
    int i = 0;
    for (i = 0; i < BUTTON_COUNT; i++)
    {
        APP_vRegisterOnOffEndPoint(sDeviceConfig.psEndpointButtons[i].u16Endpoint, &tsOnOffButtonEndpoints[i]);
    }
}

PRIVATE void APP_vRegisterOnOffEndPoint(uint8 u8Endpoint, tsZHA_OnOffEndpoint *tsEndpoint)
{
    DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: Registering endpoint: %d\n", u8Endpoint);
    tsEndpoint->sEndPoint.u8EndPointNumber = u8Endpoint;
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

PRIVATE void APP_cbOnOffEndpointCallback(tsZCL_CallBackEvent *psEvent)
{
    switch (psEvent->eEventType)
    {
    default:
        DBG_vPrintf(TRACE_ON_OFF_EP, "ON_OFF EP: Invalid event type (%d) in APP_ZCL_cbEndpointCallback\n", psEvent->eEventType);
        break;
    }
}
