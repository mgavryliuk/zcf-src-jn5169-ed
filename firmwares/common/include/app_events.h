#ifndef APP_GENERIC_EVENTS_H_
#define APP_GENERIC_EVENTS_H_

#include <jendefs.h>
#include "app_button_actions.h"

#ifdef DEBUG_EVENTS
#define TRACE_EVENTS TRUE
#else
#define TRACE_EVENTS FALSE
#endif

typedef enum
{
    APP_NO_EVENT,
    APP_WAKE_UP_EVENT,
    APP_RESET_DEVICE_EVENT,
    APP_BUTTON_EVENT,
} APP_teEventType;

typedef struct
{
    uint8 u8Endpoint;
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

PUBLIC void APP_vSendWakeUpEvent(void);
PUBLIC void APP_vSendResetDeviceEvent(void);
PUBLIC void APP_vSendButtonEvent(uint8 u8Endpoint, teButtonAction eButtonAction);
PUBLIC void APP_vProcessEvents(void);

#endif /*APP_GENERIC_EVENTS_H_*/
