#include <jendefs.h>


#include "dbg.h"
#include "dbg_uart.h"
#include "string.h"
#include "MicroSpecific.h"


#define BOARD_LED_BIT           (17)
#define BOARD_LED_PIN           (1UL << BOARD_LED_BIT)
#define BOARD_LED_CTRL_MASK     (BOARD_LED_PIN)
#define NVIC_INT_PRIO_LEVEL_SYSCTRL (1)
#define NVIC_INT_PRIO_LEVEL_BBC     (7)

extern void *_stack_low_water_mark;

void vAppRegisterPWRMCallbacks(void)
{
}

PUBLIC void vISR_SystemController(uint32 u32Device, uint32 u32ItemBitmap)
{
    DBG_vPrintf(TRUE, "In vISR_SystemController\n");
}

PUBLIC void vAppMain(void)
{
    // TARGET_INITIALISE();
    // SET_IPL(0);
    // portENABLE_INTERRUPTS();

    
    int i;
    int iteration = 0;

    /* Wait until FALSE i.e. on XTAL  - otherwise uart data will be at wrong speed */
    while (bAHI_GetClkSource() == TRUE);
    bAHI_SetClockRate(3); /* Move CPU to 32 MHz  vAHI_OptimiseWaitStates automatically called */
    
    /*
     * Initialize the debug diagnostics module to use UART0 at 115K Baud;
     * Do not use UART 1 if LEDs are used, as it shares DIO with the LEDS
     * */
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);

    /*
     * Initialise the stack overflow exception to trigger if the end of the
     * stack is reached. See the linker command file to adjust the allocated
     * stack size.
     */

    vAHI_SysCtrlRegisterCallback ( vISR_SystemController );
    u32AHI_Init();
    vAHI_InterruptSetPriority ( MICRO_ISR_MASK_BBC,        NVIC_INT_PRIO_LEVEL_BBC );
    vAHI_InterruptSetPriority ( MICRO_ISR_MASK_SYSCTRL, NVIC_INT_PRIO_LEVEL_SYSCTRL );
    
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
