
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
#define BATTERY_MEASURE_DELAY_CYCLES 50

#include <jendefs.h>

PUBLIC void APP_vMeasureRemainingBattery(void);

#endif /* APP_BATTERY_H */
