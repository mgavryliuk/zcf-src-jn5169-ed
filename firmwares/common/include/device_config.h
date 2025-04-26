#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include <jendefs.h>
#include "buttons_definitions.h"

#ifndef BUTTON_COUNT
#error "BUTTON_COUNT is not defined"
#endif

typedef struct
{
    const uint32_t u32DioMask;
    const uint32_t u32LedMask;
} tsDioConfig;

typedef struct
{
    const uint32_t u32DioMask;
    const uint32_t u32LedMask;
    const bool bHasLed;
} tsSystemButtonConfig;

typedef struct
{
    const uint32_t u32DioMask;
    const uint32_t u32LedMask;
    const bool bHasLed;
    const uint16_t u16Endpoint;
} tsEndpointButtonConfig;

typedef struct
{
    const char *sDeviceModelId;
    const uint8_t u8BasicEndpoint;
    const uint8_t u8ZdoEndpoint;

    const tsDioConfig sDioConfig;
    const tsSystemButtonConfig sResetButtonConfig;

    const tsEndpointButtonConfig *psEndpointButtons;
} tsDeviceConfig;

extern const tsDeviceConfig sDeviceConfig;

#endif /* DEVICE_CONFIG_H */
