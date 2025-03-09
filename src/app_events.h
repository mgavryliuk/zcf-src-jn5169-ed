#ifndef APP_GENERIC_EVENTS_H_
#define APP_GENERIC_EVENTS_H_

typedef enum
{
    APP_E_EVENT_NONE = 0,
    APP_E_BTN_PRESSED,
    APP_E_BTN_RELEASED
} APP_teEventType;

typedef struct
{
    uint8 u8Button;
} APP_tsEventButton;

typedef struct
{
    APP_teEventType eType;
    union
    {
        APP_tsEventButton sButton;
    } uEvent;
} APP_tsEvent;

#endif /*APP_GENERIC_EVENTS_H_*/
