#ifndef APP_LED_H
#define APP_LED_H

#define APP_LED1_DIO (16)
#define APP_LED1_PIN (1UL << APP_LED1_DIO)

#define APP_LED2_DIO (17)
#define APP_LED2_PIN (1UL << APP_LED2_DIO)
#define APP_LED_CTRL_MASK (APP_LED1_PIN | APP_LED2_PIN)

PUBLIC void APP_vSetupLeds();
PUBLIC void APP_vBlinkLed();
PUBLIC void APP_cbBlinkLed(void *pvParam);

#endif /* APP_LED_H */
