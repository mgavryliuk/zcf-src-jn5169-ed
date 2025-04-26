#include <jendefs.h>

#include "dbg.h"
#include "zcl.h"
#include "zcl_customcommand.h"
#include "AppHardwareApi.h"
#include "PowerConfiguration.h"

#include "device_config.h"
#include "app_battery.h"

PRIVATE uint8 u8MeasureCycles = BATTERY_MEASURE_DELAY_CYCLES;

PRIVATE void vMeasureBattery(void);

PUBLIC void APP_vMeasureRemainingBattery(void)
{
    if (u8MeasureCycles == BATTERY_MEASURE_DELAY_CYCLES) {
        u8MeasureCycles = 0;
        vMeasureBattery();
        return;
    }
    u8MeasureCycles++;
}

PRIVATE void vMeasureBattery(void) {
    uint8 u8BatteryPercentageRemaining;

    vAHI_AdcEnable(E_AHI_ADC_SINGLE_SHOT, E_AHI_AP_INPUT_RANGE_2, E_AHI_ADC_SRC_VOLT);
    DBG_vPrintf(TRACE_BATTERY, "BATTERY: Getting VDD voltage\n");
    vAHI_AdcStartSample();
    while (bAHI_AdcPoll())
        ;
    vAHI_AdcDisable();
    uint16 u16AdcValue = u16AHI_AdcRead();
    uint32 u32Temp = ((uint32)u16AdcValue * 7410);
    uint16 u16BattLevelmV = (u32Temp >> 11);
    if (u16BattLevelmV < BATTERY_MIN_MV)
    {
        u8BatteryPercentageRemaining = 0;
        u16BattLevelmV = 0;
    }
    else
    {
        u8BatteryPercentageRemaining = (uint8)((u16BattLevelmV - BATTERY_MIN_MV) * 100 / BATTERY_DELTA_MV);
        if (u8BatteryPercentageRemaining > 100)
        {
            u8BatteryPercentageRemaining = 100;
        }
    }
    DBG_vPrintf(TRACE_BATTERY, "BATTERY: Voltage: %d mV. MIN: %d, DELTA: %d. Percantage remaining: %d\n",
                u16BattLevelmV, BATTERY_MIN_MV, BATTERY_DELTA_MV, u8BatteryPercentageRemaining);

    tsZCL_ClusterInstance *psZCL_ClusterInstance;
    teZCL_Status eStatus = eZCL_SearchForClusterEntry(sDeviceConfig.u8BasicEndpoint, GENERAL_CLUSTER_ID_POWER_CONFIGURATION, TRUE, &psZCL_ClusterInstance);
    DBG_vPrintf(TRACE_BATTERY, "BATTERY: Search for cluster entry %d in endpoint %d status: %d\n", GENERAL_CLUSTER_ID_POWER_CONFIGURATION, sDeviceConfig.u8BasicEndpoint, eStatus);

    ((tsCLD_PowerConfiguration *)psZCL_ClusterInstance->pvEndPointSharedStructPtr)->u8BatteryVoltage = (uint8)(u16BattLevelmV / 100);
    ((tsCLD_PowerConfiguration *)psZCL_ClusterInstance->pvEndPointSharedStructPtr)->u8BatteryPercentageRemaining = u8BatteryPercentageRemaining;
}
