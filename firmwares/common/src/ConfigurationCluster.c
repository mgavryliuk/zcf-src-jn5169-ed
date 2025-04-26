#include <jendefs.h>

#include "dbg.h"
#include "zcl.h"
#include "PDM.h"

#include "ConfigurationCluster.h"
#include "zcl_options.h"

const tsZCL_AttributeDefinition asCLD_ConfigurationClusterAttributeDefinitions[] = {
    {E_CLD_BUTTON_MODE_ATTR_ID_MODE_TYPE, (E_ZCL_AF_RD | E_ZCL_AF_WR | E_ZCL_AF_MS), E_ZCL_ENUM8, (uint32)(&((tsCLD_Configuration *)(0))->eButtonMode), 0},
    {E_CLD_PREVENT_RESET_ATTR_ID_MODE_TYPE, (E_ZCL_AF_RD | E_ZCL_AF_WR | E_ZCL_AF_MS), E_ZCL_BOOL, (uint32)(&((tsCLD_Configuration *)(0))->bPreventReset), 0},
};

tsZCL_ClusterDefinition sCLD_Configuration = {
    GENERAL_CLUSTER_ID_CONFIGURATION,
    TRUE,
    E_ZCL_SECURITY_NETWORK,
    (sizeof(asCLD_ConfigurationClusterAttributeDefinitions) / sizeof(tsZCL_AttributeDefinition)),
    (tsZCL_AttributeDefinition *)asCLD_ConfigurationClusterAttributeDefinitions,
    NULL};

uint8 au8ConfigurationAttributeControlBits[(sizeof(asCLD_ConfigurationClusterAttributeDefinitions) / sizeof(tsZCL_AttributeDefinition))];

PRIVATE void vLoadConfigurationCluster(tsCLD_Configuration *sConfigCluster);
PRIVATE void vSaveConfigurationCluster(tsCLD_Configuration *sConfigCluster);

PUBLIC teZCL_Status eCLD_ConfigurationCreateConfiguration(
    tsZCL_ClusterInstance *psClusterInstance,
    bool_t bIsServer,
    tsZCL_ClusterDefinition *psClusterDefinition,
    void *pvEndPointSharedStructPtr,
    uint8 *pu8AttributeControlBits)
{
    vZCL_InitializeClusterInstance(
        psClusterInstance,
        bIsServer,
        psClusterDefinition,
        pvEndPointSharedStructPtr,
        pu8AttributeControlBits,
        NULL,
        NULL);

    vLoadConfigurationCluster((tsCLD_Configuration *)pvEndPointSharedStructPtr);
    return E_ZCL_SUCCESS;
}

PUBLIC void vHandleConfigurationClusterAttrsRange(tsZCL_CallBackEvent *psEvent)
{
    uint16 attrId = psEvent->uMessage.sIndividualAttributeResponse.u16AttributeEnum;
    if (attrId == E_CLD_BUTTON_MODE_ATTR_ID_MODE_TYPE)
    {
        uint8 mode = *((uint8 *)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData);
        DBG_vPrintf(TRACE_CONFIGURATION_CLUSTER, "CONFIG CLUSTER: Validating attrId %d with value %d\n", attrId, mode);
        if ((mode < E_CLD_BUTTON_MODE_TOGGLE) || (mode > E_CLD_BUTTON_MODE_MULTISTATE_INPUT))
        {
            DBG_vPrintf(TRACE_CONFIGURATION_CLUSTER, "CONFIG CLUSTER: Invelid Mode value: %d\n", mode);
            psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus = E_ZCL_ERR_ATTRIBUTE_RANGE;
        }
    }
}

PUBLIC void vHandleConfigurationClusterAttrsWrite(tsZCL_CallBackEvent *psEvent, tsCLD_Configuration *sConfigCluster)
{
    uint16 attrId = psEvent->uMessage.sIndividualAttributeResponse.u16AttributeEnum;
    switch (psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus)
    {
    case E_ZCL_SUCCESS:
        if (attrId == E_CLD_BUTTON_MODE_ATTR_ID_MODE_TYPE)
        {
            uint8 mode = *((uint8 *)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData);
            DBG_vPrintf(TRACE_CONFIGURATION_CLUSTER, "CONFIG CLUSTER: Writing attrId %d with value %d\n", attrId, mode);
            if ((mode < E_CLD_BUTTON_MODE_TOGGLE) || (mode > E_CLD_BUTTON_MODE_MULTISTATE_INPUT))
            {
                DBG_vPrintf(TRACE_CONFIGURATION_CLUSTER, "CONFIG CLUSTER: Invalid Mode value: %d\n", mode);
            }
            else
            {
                sConfigCluster->eButtonMode = mode;
                vSaveConfigurationCluster(sConfigCluster);
            }
        }
        else if (attrId == E_CLD_PREVENT_RESET_ATTR_ID_MODE_TYPE)
        {
            bool_t bPreventReset = *((bool_t *)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData);
            DBG_vPrintf(TRACE_CONFIGURATION_CLUSTER, "CONFIG CLUSTER: Writing attrId %d with value %d\n", attrId, bPreventReset);
            sConfigCluster->bPreventReset = bPreventReset;
            vSaveConfigurationCluster(sConfigCluster);
        }

        break;

    default:
        DBG_vPrintf(TRACE_CONFIGURATION_CLUSTER, "CONFIG CLUSTER: Writing attrId %d failed with status %d\n", attrId, psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus);
        break;
    }
}

PUBLIC void vResetConfigurationCluster(void)
{
    DBG_vPrintf(TRACE_CONFIGURATION_CLUSTER, "CONFIG CLUSTER: Reseting configuration cluster from PDM\n");
    PDM_vDeleteDataRecord(PDM_ID_CONFIGURATION_CLUSTER);
}


PRIVATE void vLoadConfigurationCluster(tsCLD_Configuration *sConfigCluster)
{
    if (sConfigCluster != NULL)
    {
        sConfigCluster->eButtonMode = E_CLD_BUTTON_MODE_TOGGLE;
        sConfigCluster->bPreventReset = FALSE;
        uint16 u16ByteRead;
        DBG_vPrintf(TRACE_CONFIGURATION_CLUSTER, "CONFIG CLUSTER: Loading ConfigurationCluster attributes from PDM\n");
        PDM_eReadDataFromRecord(PDM_ID_CONFIGURATION_CLUSTER, sConfigCluster, sizeof(tsCLD_Configuration), &u16ByteRead);
        DBG_vPrintf(TRACE_CONFIGURATION_CLUSTER,
                    "CONFIG CLUSTER: Button Mode: %d. Prevent Reset: %d\n",
                    sConfigCluster->eButtonMode, sConfigCluster->bPreventReset);
    }
}

PRIVATE void vSaveConfigurationCluster(tsCLD_Configuration *sConfigCluster)
{
    DBG_vPrintf(TRACE_CONFIGURATION_CLUSTER, "CONFIG CLUSTER: Saving ConfigurationCluster attributes to PDM\n");
    PDM_eSaveRecordData(PDM_ID_CONFIGURATION_CLUSTER, sConfigCluster, sizeof(tsCLD_Configuration));
}
