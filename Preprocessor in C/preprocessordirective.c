#include <stdio.h>
#include <stdint.h>

#define USE_BUFFERED_RX_MOD     0
#define RX_BUFFER_SIZE      8

volatile uint8_t    rxBuffer[RX_BUFFER_SIZE];
volatile uint8_t    moduleRxHead = 0;
volatile uint8_t    moduleRxTail = 0;
volatile uint8_t    moduleRxCount = 0;


void USART_RX_ISR(uint8_t receivedByte)
{
#if(USE_BUFFERED_RX_MOD)

    if(moduleRxCount<RX_BUFFER_SIZE)
    {
        rxBuffer[moduleRxHead++] = receivedByte;
        moduleRxHead %= RX_BUFFER_SIZE;
        moduleRxCount++;
        printf("Now USE_BUFFERED_RX_MOD == 1\n");


    }
    else{
        printf("Buffer overflow!\n");
    }
#else
    printf("ISR Processing: Got byte = 0x%02X\n", receivedByte);

#endif
}

// Called periodically in main loop to process buffered data
void process_buffered_data(void)
{
#if (USE_BUFFERED_RX_MOD)
    while (moduleRxCount > 0) {
        uint8_t byte = rxBuffer[moduleRxTail++];
        moduleRxTail %= RX_BUFFER_SIZE;
        moduleRxCount--;

        printf("Main Loop Processing: Got byte = 0x%02X\n", byte);
    }
#endif
}

int main(void)
{
    // Simulate some received bytes
    USART_RX_ISR(0x41);  // 'A'
    USART_RX_ISR(0x42);  // 'B'
    USART_RX_ISR(0x43);  // 'C'
    printf("Now USE_BUFFERED_RX_MOD == 0\n");

#if (USE_BUFFERED_RX_MOD)
    // In buffered mode, processing happens outside ISR
    process_buffered_data();
#endif

    return 0;
}
