/*****************************************************************************
 * Custom configuration cluster for Endpoint with OnOff and MultistateInput clusters
 ****************************************************************************/

#ifndef ENDPOINT_CONFIGURATION_CLUSTER_H
#define ENDPOINT_CONFIGURATION_CLUSTER_H

#include <jendefs.h>
#include "zcl.h"
#include "zcl_options.h"

#define GENERAL_CLUSTER_ID_BUTTON_MODE 0xfc00

typedef enum
{
    E_CLD_BUTTON_MODE_ATTR_ID_MODE_TYPE = 0x0000,
} teCLD_ButtonMode_Attr;

typedef enum
{
    E_CLD_BUTTON_MODE_TOGGLE = 0,
    E_CLD_BUTTON_MODE_MOMENTARY,
    E_CLD_BUTTON_MODE_MULTISTATE_INPUT,
} teCLD_ButtonMode;

typedef enum
{
    E_CLD_BUTTON_MODE_TOGGLE_ACTION,
    E_CLD_BUTTON_MODE_MOMENTRAY_PRESSED_ACTION,
    E_CLD_BUTTON_MODE_MOMENTARY_RELEASED_ACTION,
    E_CLD_BUTTON_MODE_SINGLE_CLICK_ACTION,
    E_CLD_BUTTON_MODE_DOUBLE_CLICK_ACTION,
    E_CLD_BUTTON_MODE_TRIPPLE_CLICK_ACTION,
    E_CLD_BUTTON_MODE_LONG_CLICK_ACTION,
} teCLD_ButtonMode_Action;

typedef struct
{
    zenum8 eMode;
} tsCLD_ButtonMode;

PUBLIC teZCL_Status eCLD_ButtonModeCreateButtonMode(
    tsZCL_ClusterInstance *psClusterInstance,
    bool_t bIsServer,
    tsZCL_ClusterDefinition *psClusterDefinition,
    void *pvEndPointSharedStructPtr,
    uint8 *pu8AttributeControlBits);

extern tsZCL_ClusterDefinition sCLD_ButtonMode;
extern uint8 au8ButtonModeAttributeControlBits[];
extern const tsZCL_AttributeDefinition asCLD_ButtonModeClusterAttributeDefinitions[];

#endif /* ENDPOINT_CONFIGURATION_CLUSTER_H */
