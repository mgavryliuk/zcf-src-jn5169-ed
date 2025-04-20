
#ifndef APP_BATTERY_H
#define APP_BATTERY_H

#ifdef DEBUG_BATTERY
#define TRACE_BATTERY TRUE
#else
#define TRACE_BATTERY FALSE
#endif

#define BATTERY_MIN_MV (2000)
#define BATTERY_MAX_MV (3000)
#define BATTERY_DELTA_MV (BATTERY_MAX_MV - BATTERY_MIN_MV)

typedef struct
{
    uint8 u8Voltage;
    uint8 u8BatteryPercentageRemaining;
} tsBatteryData;

PUBLIC tsBatteryData APP_u8GetBatteryPercentageRemaining(void);

#endif /* APP_BATTERY_H */
