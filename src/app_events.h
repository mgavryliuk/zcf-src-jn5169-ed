#ifndef APP_GENERIC_EVENTS_H_
#define APP_GENERIC_EVENTS_H_

#include <jendefs.h>
#include "app_button.h"

#ifdef DEBUG_EVENTS
#define TRACE_EVENTS TRUE
#else
#define TRACE_EVENTS FALSE
#endif

#define SYNC_BATTERY_VALUES_WAKE_UP_CYCLES 50

typedef enum
{
    APP_NO_EVENT,
    APP_WAKE_UP_EVENT,
    APP_RESET_DEVICE_EVENT,
    APP_BUTTON_EVENT,
} APP_teEventType;

typedef struct
{
    uint16 u16Endpoint;
    teButtonAction eButtonAction;
} APP_tsEventButton;

typedef struct
{
    APP_teEventType eType;
    union
    {
        APP_tsEventButton sButton;
    } uEvent;
} APP_tsEvent;

PUBLIC void sendWakeUpEvent(void);
PUBLIC void sendResetDeviceEvent(void);
PUBLIC void sendButtonEvent(uint16 u16Endpoint, teButtonAction eButtonAction);
PUBLIC void APP_processEvents(void);

#endif /*APP_GENERIC_EVENTS_H_*/
