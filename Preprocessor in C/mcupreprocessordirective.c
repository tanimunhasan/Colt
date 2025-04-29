#include<stdio.h>
#include<stdint.h>


#define MCU_Type 2

void setup_hardware(void)
{
#if MCU_Type == 1
    printf("Initializing hardware for STM32\n");
#elif MCU_Type == 2
    printf(" Initializing hardware for ESP32\n");
#elif MCU_Type == 3
    printf("Initializing hardware for MSP430");
#else
    printf("Unknown MCU type\n");
#endif // MCU_Type
}

int main(void)
{
    #if(MCU_Type)
        setup_hardware();
    #endif

}
