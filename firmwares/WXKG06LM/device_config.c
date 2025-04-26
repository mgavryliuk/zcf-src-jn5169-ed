#include <jendefs.h>
#include "zps_gen.h"

#include "device_config.h"
#include "buttons_definitions.h"
#include "leds_definitions.h"

PRIVATE const tsEndpointButtonConfig sButtonsConfigs[] = {
    {
        .u32DioMask = BTN_MASK,
        .u32LedMask = LEDS_CTRL_MASK,
        .bHasLed = TRUE,
        .u16Endpoint = WXKG06LM_BUTTON_ENDPOINT,
    },
};

const tsDeviceConfig sDeviceConfig = {
    .sDeviceModelId = "lumi.remote.b186acn02.diy",
    .u8BasicEndpoint = WXKG06LM_BASIC_ENDPOINT,
    .u8ZdoEndpoint = WXKG06LM_ZDO_ENDPOINT,
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
