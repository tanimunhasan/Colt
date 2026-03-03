#include <stdio.h>
#include <msp430.h>
#include <string.h>
#include <stdbool.h>
#include "hal_uart.h"
#include "studiolib.h"

volatile bool g_reed_tap = false;

void clockConfigure(void)
{
    CSCTL0_H = CSKEY_H;               // Unlock CS registers
    CSCTL1 = DCOFSEL_0;               // DCO at 1 MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;  // No dividers
    CSCTL0_H = 0;                      // Lock CS registers
}

void uart_init(void)
{
    // Configure UART pins
    // Baud 9600 @ 1MHz

    //-----eUSCI_A1 for USB Terminal (P2.0 TX, P2.1 RX) ---------------
    P2SEL1 |= BIT0 | BIT1;      // TX: P2.0, RX: P2.1
    P2SEL0 &= ~(BIT0 | BIT1);

    UCA1BRW = 6;
    UCA1MCTLW = (8 << 4) | UCOS16;
    // Baud 9600 @ 1MHz
     UCA3BRW = 6;
     UCA3MCTLW = (8 << 4) | UCOS16;
     UCA3CTLW0 = UCSWRST;
     UCA3CTLW0 |= UCSSEL__SMCLK;
// -----------------------------------------------------------------------------

    // ---------- eUSCI_A2 for N2O Sensor (P5.0 TX, P5.1 RX) ----------
    P5SEL1 |= BIT0 | BIT1;             // Set P5.0 -> TXD, P5.1 -> RXD (Dynament Sensor)
    P5SEL0 &= ~(BIT0 | BIT1);          // Secondary function

    UCA2CTLW0 = UCSWRST;               // Put eUSCI_A2 in reset mode
    UCA2CTLW0 |= UCSSEL__SMCLK;        // Use SMCLK

    UCA2BRW = 6; // 🔹 Fix: Set Baud Rate for UCA2
    UCA2MCTLW = (8 << 4) | UCOS16;  // Modulation settings

    // Release from reset
    UCA3CTLW0 &= ~UCSWRST;
    UCA2CTLW0 &= ~UCSWRST; // Correctly release from reset


}

void delay_ms(unsigned int ms)
{
    while (ms--)
    {
        __delay_cycles(1000);          // 1 ms delay at 1 MHz
    }
}


void initGLED(void)
{
    P6DIR |= BIT0;  // Set P6.0 as output
    P6OUT &= ~BIT0; // Turn off LED initially
}


//void toggleGLED(void)
//{
//    P6OUT ^= BIT0;  // Toggle the state of P6.0
//
//
//}

void module_Write(uint8_t txData)
{

}
//-------------------------------------------------------------------------------------------------------
// MBUS or SERIAL Module - write buffer, L bytes
//-------------------------------------------------------------------------------------------------------
void module_Write_Str(uint8_t *str, uint16_t len)
{
    while (len--)
        module_Write(*str++);
}

/*----------------------------------------------------------------*/

void reed_init(void)
{
    P3DIR &= ~BIT3;      // Input
    P3REN &= ~BIT3;      // Disable internal resistor (you already have external)
    P3IES |= BIT3;       // High-to-low edge
    P3IFG &= ~BIT3;      // Clear flag
    P3IE  |= BIT3;       // Enable interrupt
}


bool reed_is_pressed(void)
{
    return ((P3IN & BIT3) == 0);
}

void reed_init_irq(void)
{
    // P3.3 input
    P3DIR &= ~BIT3;

    // You already have external pull-up (R2), so disable internal
    P3REN &= ~BIT3;

    // Interrupt on HIGH->LOW (magnet applied closes reed to GND)
    P3IES |= BIT3;

    P3IFG &= ~BIT3;   // clear pending
    P3IE  |= BIT3;    // enable
}

/*----------------------------------------------------------------*/
//Interrupt PORT3
#pragma vector=PORT3_VECTOR   // confirm this matches your device header
__interrupt void PORT3_ISR(void)
{
    if (P3IFG & BIT3)
    {
        P3IFG &= ~BIT3;

        // Debounce ~20ms @ 1MHz
        __delay_cycles(20000);

        // Read stable level: 0 = magnet present (reed closed), 1 = open
        if ((P3IN & BIT3) == 0)
        {
            g_reed_tap = true;   // flag event for main loop
        }

        // Keep detecting the same event type (HIGH->LOW)
        P3IES |= BIT3;

        // Clear again in case bounce re-set it during delay
        P3IFG &= ~BIT3;
    }
}
