#ifndef ZCL_OPTIONS_H
#define ZCL_OPTIONS_H

#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/*                      ZCL Specific initialization                         */
/****************************************************************************/
/* This is the NXP manufacturer code.If creating new a manufacturer         */
/* specific command apply to the Zigbee alliance for an Id for your company */
/* Also update the manufacturer code in .zpscfg: Node Descriptor->misc      */

#define ZCL_MANUFACTURER_CODE 0x1037

/* Sets the number of endpoints that will be created by the ZCL library */
#define ZCL_NUMBER_OF_ENDPOINTS 3

/* Set this Tue to disable non error default responses from clusters */
#define ZCL_DISABLE_DEFAULT_RESPONSES (TRUE)
#define ZCL_DISABLE_APS_ACK (TRUE)

#define ZCL_ATTRIBUTE_READ_SERVER_SUPPORTED
#define ZCL_ATTRIBUTE_READ_CLIENT_SUPPORTED
#define ZCL_ATTRIBUTE_WRITE_SERVER_SUPPORTED
#define ZCL_ATTRIBUTE_REPORTING_SERVER_SUPPORTED
#define ZCL_NUMBER_OF_REPORTS 2
#define ZCL_SYSTEM_MIN_REPORT_INTERVAL 1
#define ZCL_SYSTEM_MAX_REPORT_INTERVAL 600

/* Enable wild card profile */
#define ZCL_ALLOW_WILD_CARD_PROFILE
/****************************************************************************/
/*                             Enable Cluster                               */
/*                                                                          */
/* Add the following #define's to your zcl_options.h file to enable         */
/* cluster and their client or server instances                             */
/****************************************************************************/
#define CLD_BASIC
#define BASIC_SERVER

#define CLD_IDENTIFY
#define IDENTIFY_SERVER

#define CLD_GROUPS
#define GROUPS_CLIENT

#define CLD_ONOFF
#define ONOFF_CLIENT

#define CLD_MULTISTATE_INPUT_BASIC
#define MULTISTATE_INPUT_BASIC_SERVER
#define CLD_MULTISTATE_INPUT_BASIC_ATTR_NUMBER_OF_STATES 9

#define CLD_POWER_CONFIGURATION
#define POWER_CONFIGURATION_SERVER
#define CLD_PWRCFG_ATTR_BATTERY_VOLTAGE
#define CLD_PWRCFG_ATTR_BATTERY_PERCENTAGE_REMAINING
/****************************************************************************/
/*             Basic Cluster - Optional Attributes                          */
/*                                                                          */
/* Add the following #define's to your zcl_options.h file to add optional   */
/* attributes to the basic cluster.                                         */
/****************************************************************************/
#define CLD_BAS_ATTR_APPLICATION_VERSION
#define CLD_BAS_ATTR_STACK_VERSION
#define CLD_BAS_ATTR_HARDWARE_VERSION
#define CLD_BAS_ATTR_MANUFACTURER_NAME
#define CLD_BAS_ATTR_MODEL_IDENTIFIER
#define CLD_BAS_ATTR_DATE_CODE
#define CLD_BAS_ATTR_SW_BUILD_ID

#define CLD_BAS_APP_VERSION (1)
#define CLD_BAS_STACK_VERSION (1)
#define CLD_BAS_HARDWARE_VERSION (1)
#define CLD_BAS_MANUF_NAME_SIZE (3)
#define CLD_BAS_MANUF_NAME_STR "DIY"
#define CLD_BAS_DATE_SIZE (8)
#define CLD_BAS_SW_BUILD_SIZE (6)
#define CLD_BAS_POWER_SOURCE E_CLD_BAS_PS_BATTERY

#define CLD_BAS_DATE_STR BUILD_DATE
#define CLD_BAS_SW_BUILD_STR BUILD_NUMBER

#ifdef TARGET_WXKG07LM
    #define CLD_BAS_MODEL_ID_STR "lumi.remote.b286acn02.alt"
    #define CLD_BAS_MODEL_ID_SIZE (25)
#elif defined(TARGET_WXKG06LM)
    #define CLD_BAS_MODEL_ID_STR "lumi.remote.b186acn02.alt"
    #define CLD_BAS_MODEL_ID_SIZE (25)
#elif defined(TARGET_WXKG11LM)
    #define CLD_BAS_MODEL_ID_STR "lumi.remote.b1acn01.alt"
    #define CLD_BAS_MODEL_ID_SIZE (23)
#else
    #error "Unknown target device"
#endif

#endif /* ZCL_OPTIONS_H */
