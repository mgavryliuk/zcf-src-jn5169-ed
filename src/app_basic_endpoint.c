#include <jendefs.h>
#include <string.h>
#include "dbg.h"
#include "zcl.h"
#include "zcl_options.h"
#include "zps_gen.h"
#include "Basic.h"
#include "Identify.h"
#include "PowerConfiguration.h"

#include "app_basic_endpoint.h"
#include "app_battery.h"
#include "app_button.h"

PRIVATE void endpointCallback(tsZCL_CallBackEvent *psEvent);
PRIVATE void handleButtonModeClusterAttrsRange(tsZCL_CallBackEvent *psEvent);
PRIVATE void handleButtonModeClusterAttrsWrite(tsZCL_CallBackEvent *psEvent);

tsZHA_BasicEndpoint tsBasicEndpoint;

PRIVATE void syncBatteryPercentageRemaining(void)
{
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Sync Battery data\n");
    tsBatteryData sBatteryData = APP_u8GetBatteryPercentageRemaining();
    tsBasicEndpoint.sPowerConfigurationCluster.u8BatteryPercentageRemaining = sBatteryData.u8BatteryPercentageRemaining * 2;
    tsBasicEndpoint.sPowerConfigurationCluster.u8BatteryVoltage = sBatteryData.u8Voltage;
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Battery data synced: %dV (%d%%)\n", sBatteryData.u8Voltage * 100, sBatteryData.u8BatteryPercentageRemaining);
}

PUBLIC void APP_vRegisterBasicEndPoint(void)
{
    teZCL_Status eZCL_Status = eCLD_BasicCreateBasic(
        &tsBasicEndpoint.sClusterInstance.sBasicServer,
        TRUE,
        &sCLD_Basic,
        &tsBasicEndpoint.sBasicServerCluster,
        &au8BasicClusterAttributeControlBits[0]);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eCLD_BasicCreateBasic failed with status: %d\n", eZCL_Status);
    }

    eZCL_Status = eCLD_IdentifyCreateIdentify(
        &tsBasicEndpoint.sClusterInstance.sIdentifyServer,
        TRUE,
        &sCLD_Identify,
        &tsBasicEndpoint.sIdentifyServerCluster,
        &au8IdentifyAttributeControlBits[0],
        &tsBasicEndpoint.sIdentifyServerCustomDataStructure);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eCLD_IdentifyCreateIdentify failed with status: %d\n", eZCL_Status);
    }

    eZCL_Status = eCLD_PowerConfigurationCreatePowerConfiguration(
        &tsBasicEndpoint.sClusterInstance.sPowerConfigurationServer,
        TRUE,
        &sCLD_PowerConfiguration,
        &tsBasicEndpoint.sPowerConfigurationCluster,
        &au8PowerConfigurationAttributeControlBits[0]);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eCLD_PowerConfigurationCreatePowerConfiguration failed with status: %d\n", eZCL_Status);
    }

    eZCL_Status = eCLD_ButtonModeCreateButtonMode(
        &tsBasicEndpoint.sClusterInstance.sButtonModeServer,
        TRUE,
        &sCLD_ButtonMode,
        &tsBasicEndpoint.sButtonModeCluster,
        &au8ButtonModeAttributeControlBits[0]);
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Configuring Custom Button Mode cluster status: %d\n", eZCL_Status);

    tsBasicEndpoint.sEndPoint.u8EndPointNumber = WXKG07LM_ALT_BASIC_ENDPOINT;
    tsBasicEndpoint.sEndPoint.u16ManufacturerCode = ZCL_MANUFACTURER_CODE;
    tsBasicEndpoint.sEndPoint.u16ProfileEnum = HA_PROFILE_ID;
    tsBasicEndpoint.sEndPoint.bIsManufacturerSpecificProfile = FALSE;
    tsBasicEndpoint.sEndPoint.u16NumberOfClusters = sizeof(tsZHA_BasicEndpointClusterInstances) / sizeof(tsZCL_ClusterInstance);
    tsBasicEndpoint.sEndPoint.psClusterInstance = (tsZCL_ClusterInstance *)&tsBasicEndpoint.sClusterInstance;
    tsBasicEndpoint.sEndPoint.bDisableDefaultResponse = ZCL_DISABLE_DEFAULT_RESPONSES;
    tsBasicEndpoint.sEndPoint.pCallBackFunctions = &endpointCallback;

    eZCL_Status = eZCL_Register(&tsBasicEndpoint.sEndPoint);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eZCL_Register failed with status: %d\n", eZCL_Status);
    }

    memcpy(tsBasicEndpoint.sBasicServerCluster.au8ManufacturerName, "HomeMade", CLD_BAS_MANUF_NAME_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8ModelIdentifier, "lumi.remote.b286acn02.alt", CLD_BAS_MODEL_ID_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8DateCode, "20251105", CLD_BAS_DATE_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8SWBuildID, "1000-0001", CLD_BAS_SW_BUILD_SIZE);

    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Configuring voltage attribute to be reportable\n");
    eZCL_Status = eZCL_SetReportableFlag(WXKG07LM_ALT_BASIC_ENDPOINT, GENERAL_CLUSTER_ID_POWER_CONFIGURATION, TRUE, FALSE, E_CLD_PWRCFG_ATTR_ID_BATTERY_VOLTAGE);
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eZCL_SetReportableFlag status: %d\n", eZCL_Status);

    syncBatteryPercentageRemaining();
    tsBasicEndpoint.sButtonModeCluster.eMode = getButtonMode();
}

PRIVATE void endpointCallback(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterId = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;
    switch (psEvent->eEventType)
    {
    case E_ZCL_CBET_CHECK_ATTRIBUTE_RANGE:
        if (clusterId == GENERAL_CLUSTER_ID_BUTTON_MODE)
        {
            handleButtonModeClusterAttrsRange(psEvent);
        }
        break;

    case E_ZCL_CBET_WRITE_INDIVIDUAL_ATTRIBUTE:
        if (clusterId == GENERAL_CLUSTER_ID_BUTTON_MODE)
        {
            handleButtonModeClusterAttrsWrite(psEvent);
        }
        break;

    default:
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP CB: Event - (%d)\n", psEvent->eEventType);
        break;
    }
}

PRIVATE void handleButtonModeClusterAttrsRange(tsZCL_CallBackEvent *psEvent)
{
    uint16 attrId = psEvent->uMessage.sIndividualAttributeResponse.u16AttributeEnum;
    if (attrId == E_CLD_BUTTON_MODE_ATTR_ID_MODE_TYPE)
    {
        uint8 mode = *((uint8 *)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData);
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP CB: Validating attrId %d with value %d\n", attrId, mode);
        if ((mode < E_CLD_BUTTON_MODE_TOGGLE) || (mode > E_CLD_BUTTON_MODE_MULTISTATE_INPUT))
        {
            DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP CB: Invelid Mode value: %d\n", mode);
            psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus = E_ZCL_ERR_ATTRIBUTE_RANGE;
        }
    }
}

PRIVATE void handleButtonModeClusterAttrsWrite(tsZCL_CallBackEvent *psEvent)
{
    uint16 attrId = psEvent->uMessage.sIndividualAttributeResponse.u16AttributeEnum;
    switch (psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus)
    {
    case E_ZCL_SUCCESS:
        if (attrId == E_CLD_BUTTON_MODE_ATTR_ID_MODE_TYPE)
        {
            uint8 mode = *((uint8 *)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData);
            DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP CB: Writing attrId %d with value %d\n", attrId, mode);
            if ((mode < E_CLD_BUTTON_MODE_TOGGLE) || (mode > E_CLD_BUTTON_MODE_MULTISTATE_INPUT))
            {
                DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP CB: Invalid Mode value: %d\n", mode);
            }
            else
            {
                tsBasicEndpoint.sButtonModeCluster.eMode = mode;
                setButtonMode(mode);
            }
        }
        break;

    default:
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP CB: Writing attrId %d failed with status %d\n", attrId, psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus);
        break;
    }
}

PUBLIC void sendBasicEndpointReports(void)
{
    teZCL_Status eStatus;
    PDUM_thAPduInstance myPDUM_thAPduInstance;
    tsZCL_Address sDestinationAddress;
    sDestinationAddress.uAddress.u16DestinationAddress = 0x0000;
    sDestinationAddress.eAddressMode = E_ZCL_AM_SHORT;

    syncBatteryPercentageRemaining();
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Sending battery report\n");
    myPDUM_thAPduInstance = hZCL_AllocateAPduInstance();
    if (myPDUM_thAPduInstance == PDUM_INVALID_HANDLE)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: PDUM_INVALID_HANDLE\n");
    }
    eStatus = eZCL_ReportAllAttributes(
        &sDestinationAddress,
        GENERAL_CLUSTER_ID_POWER_CONFIGURATION,
        WXKG07LM_ALT_BASIC_ENDPOINT,
        1,
        myPDUM_thAPduInstance);
    if (E_ZCL_SUCCESS != eStatus)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Sending battery report failed with status: %d\n", eStatus);
    }
    else
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Battery report sent successfully\n");
    }
    PDUM_eAPduFreeAPduInstance(myPDUM_thAPduInstance);
}
