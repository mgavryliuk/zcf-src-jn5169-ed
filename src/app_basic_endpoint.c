#include <jendefs.h>
#include <string.h>
#include "dbg.h"
#include "zcl.h"
#include "zcl_options.h"
#include "Basic.h"
#include "Identify.h"
#include "PDM.h"
#include "PowerConfiguration.h"

#include "app_common.h"
#include "app_basic_endpoint.h"
#include "app_battery.h"
#include "app_button.h"

tsZHA_BasicEndpoint tsBasicEndpoint;

PRIVATE void endpointCallback(tsZCL_CallBackEvent *psEvent);
PRIVATE void handleConfigurationClusterAttrsRange(tsZCL_CallBackEvent *psEvent);
PRIVATE void handleConfigurationClusterAttrsWrite(tsZCL_CallBackEvent *psEvent);
PRIVATE void loadConfigurationCluster(void);
PRIVATE void saveConfigurationCluster(void);

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

    eZCL_Status = eCLD_ConfigurationCreateConfiguration(
        &tsBasicEndpoint.sClusterInstance.sConfigurationServer,
        TRUE,
        &sCLD_Configuration,
        &tsBasicEndpoint.sConfigurationCluster,
        &au8ConfigurationAttributeControlBits[0]);
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Configuring Custom Button Mode cluster status: %d\n", eZCL_Status);

    tsBasicEndpoint.sEndPoint.u8EndPointNumber = BASIC_ENDPOINT;
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

    memcpy(tsBasicEndpoint.sBasicServerCluster.au8ManufacturerName, CLD_BAS_MANUF_NAME_STR, CLD_BAS_MANUF_NAME_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8ModelIdentifier, CLD_BAS_MODEL_ID_STR, CLD_BAS_MODEL_ID_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8DateCode, CLD_BAS_DATE_STR, CLD_BAS_DATE_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8SWBuildID, CLD_BAS_SW_BUILD_STR, CLD_BAS_SW_BUILD_SIZE);

    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Configuring voltage attribute to be reportable\n");
    eZCL_Status = eZCL_SetReportableFlag(BASIC_ENDPOINT, GENERAL_CLUSTER_ID_POWER_CONFIGURATION, TRUE, FALSE, E_CLD_PWRCFG_ATTR_ID_BATTERY_VOLTAGE);
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eZCL_SetReportableFlag status: %d\n", eZCL_Status);

    loadConfigurationCluster();
    syncBatteryStatus();
}

PUBLIC void syncBatteryStatus(void)
{
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Sync Battery data\n");
    tsBatteryData sBatteryData = APP_u8GetBatteryPercentageRemaining();
    tsBasicEndpoint.sPowerConfigurationCluster.u8BatteryPercentageRemaining = sBatteryData.u8BatteryPercentageRemaining * 2;
    tsBasicEndpoint.sPowerConfigurationCluster.u8BatteryVoltage = sBatteryData.u8Voltage;
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Battery data synced: %dV (%d%%)\n", sBatteryData.u8Voltage * 100, sBatteryData.u8BatteryPercentageRemaining);
}

PUBLIC void resetConfigurationCluster(void)
{
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Reseting configuration cluster\n");
    tsBasicEndpoint.sConfigurationCluster.eButtonMode = E_CLD_BUTTON_MODE_TOGGLE;
    tsBasicEndpoint.sConfigurationCluster.bPreventReset = FALSE;
    saveConfigurationCluster();
}

PUBLIC bool_t bGetPreventReset(void)
{
    return tsBasicEndpoint.sConfigurationCluster.bPreventReset;
}

PRIVATE void loadConfigurationCluster(void)
{
    uint16 u16ByteRead;
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Loading ConfigurationCluster attributes from PDM\n");
    PDM_eReadDataFromRecord(PDM_ID_BUTTON_MODE, &tsBasicEndpoint.sConfigurationCluster, sizeof(tsCLD_Configuration), &u16ByteRead);
    setButtonMode(tsBasicEndpoint.sConfigurationCluster.eButtonMode);
    DBG_vPrintf(TRACE_BASIC_EP,
                "BASIC EP: Button Mode: %d. Prevent Reset: %d\n",
                tsBasicEndpoint.sConfigurationCluster.eButtonMode, tsBasicEndpoint.sConfigurationCluster.bPreventReset);
}

PRIVATE void saveConfigurationCluster(void)
{
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Saving ConfigurationCluster attributes to PDM\n");
    PDM_eSaveRecordData(PDM_ID_BUTTON_MODE, &tsBasicEndpoint.sConfigurationCluster, sizeof(tsCLD_Configuration));
}

PRIVATE void endpointCallback(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterId = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;
    switch (psEvent->eEventType)
    {
    case E_ZCL_CBET_CHECK_ATTRIBUTE_RANGE:
        if (clusterId == GENERAL_CLUSTER_ID_CONFIGURATION)
        {
            handleConfigurationClusterAttrsRange(psEvent);
        }
        break;

    case E_ZCL_CBET_WRITE_INDIVIDUAL_ATTRIBUTE:
        if (clusterId == GENERAL_CLUSTER_ID_CONFIGURATION)
        {
            handleConfigurationClusterAttrsWrite(psEvent);
        }
        break;

    default:
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP CB: Event - (%d)\n", psEvent->eEventType);
        break;
    }
}

PRIVATE void handleConfigurationClusterAttrsRange(tsZCL_CallBackEvent *psEvent)
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

PRIVATE void handleConfigurationClusterAttrsWrite(tsZCL_CallBackEvent *psEvent)
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
                tsBasicEndpoint.sConfigurationCluster.eButtonMode = mode;
                saveConfigurationCluster();
                setButtonMode(mode);
            }
        }
        else if (attrId == E_CLD_PREVENT_RESET_ATTR_ID_MODE_TYPE)
        {
            bool_t bPreventReset = *((bool_t *)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData);
            DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP CB: Writing attrId %d with value %d\n", attrId, bPreventReset);
            tsBasicEndpoint.sConfigurationCluster.bPreventReset = bPreventReset;
            saveConfigurationCluster();
        }

        break;

    default:
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP CB: Writing attrId %d failed with status %d\n", attrId, psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus);
        break;
    }
}
