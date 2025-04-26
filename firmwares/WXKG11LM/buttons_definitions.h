#ifndef BUTTONS_DEFINITIONS_H
#define BUTTONS_DEFINITIONS_H

#define BUTTON_COUNT (1)

#define BTN_DIO (16) // main button DIO
#define BTN_MASK (1 << BTN_DIO)
#define BTN_RESET_DIO (0) // reset button DIO
#define BTN_RESET_MASK (1 << BTN_RESET_DIO)
#define BTN_CTRL_MASK (BTN_MASK | BTN_RESET_MASK)

#endif /* BUTTONS_DEFINITIONS_H */
