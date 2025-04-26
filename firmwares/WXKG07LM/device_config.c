#include <jendefs.h>
#include "zps_gen.h"

#include "device_config.h"
#include "buttons_definitions.h"
#include "leds_definitions.h"

PRIVATE const tsEndpointButtonConfig sButtonsConfigs[] = {
    {
        .u32DioMask = BTN_LEFT_MASK,
        .u32LedMask = LED_LEFT_MASK,
        .bHasLed = TRUE,
        .u16Endpoint = WXKG07LM_LEFTBUTTON_ENDPOINT,
    },
    {
        .u32DioMask = BTN_RIGHT_MASK,
        .u32LedMask = LED_RIGHT_MASK,
        .bHasLed = TRUE,
        .u16Endpoint = WXKG07LM_RIGHTBUTTON_ENDPOINT,
    },
};

const tsDeviceConfig sDeviceConfig = {
    .u8BasicEndpoint = WXKG07LM_BASIC_ENDPOINT,
    .u8ZdoEndpoint = WXKG07LM_ZDO_ENDPOINT,
    .sDioConfig = {
        .u32DioMask = BTN_CTRL_MASK,
        .u32LedMask = LEDS_CTRL_MASK,
    },
    .sResetButtonConfig = {
        .u32DioMask = BTN_CTRL_MASK,
        .u32LedMask = LEDS_CTRL_MASK,
        .bHasLed = TRUE,
    },
    .psEndpointButtons = sButtonsConfigs,
};
