#include <jendefs.h>

#include "dbg.h"
#include "dbg_uart.h"
#include "string.h"

#define BOARD_LED_BIT           (17)
#define BOARD_LED_PIN           (1UL << BOARD_LED_BIT)
#define BOARD_LED_CTRL_MASK     (BOARD_LED_PIN)

void vAppRegisterPWRMCallbacks(void)
{
    /* nothing to register as device does not sleep */
}

PUBLIC void vAppMain(void)
{
    int i;
    int iteration = 0;
    // Initialize UART
    while (bAHI_GetClkSource() == TRUE);
    bAHI_SetClockRate(3); /* Move CPU to 32 MHz  vAHI_OptimiseWaitStates automatically called */

    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);

    // Initialize hardware
    vAHI_DioSetDirection(0, BOARD_LED_CTRL_MASK);

    while (1)
    {
        DBG_vPrintf(TRUE, "Blink iteration %d\n", iteration++);

        vAHI_DioSetOutput(0, BOARD_LED_PIN);

        for (i = 0; i < 1000000; i++)
            vAHI_DioSetOutput(0, BOARD_LED_PIN);

        vAHI_DioSetOutput(BOARD_LED_PIN, 0);

        for (i = 0; i < 1000000; i++)
            vAHI_DioSetOutput(BOARD_LED_PIN, 0);
    }
}
