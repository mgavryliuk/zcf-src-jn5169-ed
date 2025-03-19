#ifndef APP_GENERIC_EVENTS_H_
#define APP_GENERIC_EVENTS_H_

typedef enum
{
    APP_NO_EVENT,
    APP_JOINED,
    APP_WAKED_UP
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
