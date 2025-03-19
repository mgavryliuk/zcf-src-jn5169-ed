#include "dbg.h"
#include "AppHardwareApi.h"

#include "app_battery.h"

PRIVATE tsBatteryData sBatteryData;

PRIVATE uint8 u8BatteryPercentageRemaining;

PUBLIC tsBatteryData APP_u8GetBatteryPercentageRemaining(void)
{
    vAHI_AdcEnable(E_AHI_ADC_SINGLE_SHOT, E_AHI_AP_INPUT_RANGE_2, E_AHI_ADC_SRC_VOLT);
    DBG_vPrintf(TRACE_BATTERY, "BATTERY: Getting VDD voltage\n");
    vAHI_AdcStartSample();
    while (bAHI_AdcPoll())
        ;
    vAHI_AdcDisable();
    uint16 u16AdcValue = u16AHI_AdcRead();
    uint32 u32Temp = ((uint32)u16AdcValue * 7410);
    uint16 u16BattLevelmV = (u32Temp >> 11);
    DBG_vPrintf(TRACE_BATTERY, "BATTERY: Voltage: %d mV. MIN: %d, DELTA: %d\n", u16BattLevelmV, BATTERY_MIN_MV, BATTERY_DELTA_MV);
    u8BatteryPercentageRemaining = (uint8)((u16BattLevelmV - BATTERY_MIN_MV) * 100 / BATTERY_DELTA_MV);
    if (u8BatteryPercentageRemaining > 100)
    {
        u8BatteryPercentageRemaining = 100;
    }
    DBG_vPrintf(TRACE_BATTERY, "BATTERY: Percantage remaining: %d\n", u8BatteryPercentageRemaining);
    sBatteryData.u8Voltage = (uint8)(u16BattLevelmV / 100);
    sBatteryData.u8BatteryPercentageRemaining = u8BatteryPercentageRemaining;
    return sBatteryData;
}
