/*****************************************************************************
 * Custom configuration cluster for Endpoint with OnOff and MultistateInput clusters
 ****************************************************************************/

#ifndef ENDPOINT_CONFIGURATION_CLUSTER_H
#define ENDPOINT_CONFIGURATION_CLUSTER_H

#include <jendefs.h>
#include "zcl.h"
#include "zcl_options.h"

#define GENERAL_CLUSTER_ID_CONFIGURATION 0xfc00

typedef enum
{
    E_CLD_BUTTON_MODE_ATTR_ID_MODE_TYPE = 0x0000,
    E_CLD_PREVENT_RESET_ATTR_ID_MODE_TYPE = 0x0001,
} teCLD_Configuration_Attr;

typedef enum
{
    E_CLD_BUTTON_MODE_TOGGLE,
    E_CLD_BUTTON_MODE_MOMENTARY_ON_OFF,
    E_CLD_BUTTON_MODE_MULTISTATE_INPUT,
} teCLD_ButtonMode;

typedef struct
{
    zenum8 eButtonMode;
    zbool bPreventReset;
} tsCLD_Configuration;

PUBLIC teZCL_Status eCLD_ConfigurationCreateConfiguration(
    tsZCL_ClusterInstance *psClusterInstance,
    bool_t bIsServer,
    tsZCL_ClusterDefinition *psClusterDefinition,
    void *pvEndPointSharedStructPtr,
    uint8 *pu8AttributeControlBits);

extern tsZCL_ClusterDefinition sCLD_Configuration;
extern uint8 au8ConfigurationAttributeControlBits[];
extern const tsZCL_AttributeDefinition asCLD_ConfigurationClusterAttributeDefinitions[];

#endif /* ENDPOINT_CONFIGURATION_CLUSTER_H */
