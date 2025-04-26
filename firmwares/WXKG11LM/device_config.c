#include <jendefs.h>
#include "zps_gen.h"

#include "device_config.h"
#include "buttons_definitions.h"
#include "leds_definitions.h"

PRIVATE const tsEndpointButtonConfig sButtonsConfigs[] = {
    {
        .u32DioMask = BTN_MASK,
        .bHasLed = FALSE,
        .u16Endpoint = WXKG11LM_BUTTON_ENDPOINT,
    },
};

const tsDeviceConfig sDeviceConfig = {
    .u8BasicEndpoint = WXKG11LM_BASIC_ENDPOINT,
    .u8ZdoEndpoint = WXKG11LM_ZDO_ENDPOINT,
    .sDioConfig = {
        .u32DioMask = BTN_CTRL_MASK,
        .u32LedMask = LEDS_CTRL_MASK,
    },
    .sResetButtonConfig = {
        .u32DioMask = BTN_RESET_MASK,
        .u32LedMask = LEDS_CTRL_MASK,
        .bHasLed = TRUE,
    },
    .psEndpointButtons = sButtonsConfigs,
};
