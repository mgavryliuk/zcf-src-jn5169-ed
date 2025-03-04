#ifndef APP_BUTTON_H
#define APP_BUTTON_H

#define APP_BTN_LEFT_DIO (1)
#define APP_BTN_LEFT_MASK (1 << APP_BTN_LEFT_DIO)
#define APP_BTN_RIGHT_DIO (2)
#define APP_BTN_RIGHT_MASK (1 << APP_BTN_RIGHT_DIO)
#define APP_BTN_CTRL_MASK (APP_BTN_LEFT_MASK | APP_BTN_RIGHT_MASK)

PUBLIC void APP_vConfigureButtons(void);
PUBLIC void APP_vHandleBtnInterrupts(void);
PUBLIC void APP_cbTimerButtonScan(void *pvParam);

#endif  /* APP_BUTTON_H */
