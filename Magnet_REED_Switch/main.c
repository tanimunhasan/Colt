
#include <stdio.h>
#include <msp430.h>
#include <string.h>
#include <stdbool.h>
#include "hal_uart.h"
#include "hal_gpio.h"
#include "main.h"
#include "studiolib.h"
#include "hal_system.h"


void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;         // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;
    clockConfigure();
    uart_init();
    initGLED();
    DEBUG_STRING("Hello SFCommunication\n");
    reed_init();
    __enable_interrupt();

    while(1)
        {
            if (g_reed_tap)
            {
                g_reed_tap = false;
                DEBUG_STRING("\r\nMagnet Tapped");
            }

        }
}
