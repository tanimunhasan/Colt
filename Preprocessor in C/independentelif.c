#include<stdio.h>
#include<stdint.h>

// Configuration : define communication method(s)

#define USE_UART
#define USE_USB  // <-- You can comment/uncomment these to test different scenarios

// Dummy UART send implementation

void uart_send(const uint8_t* data, uint16_t length)
{
    printf("Sending over UART : ");
    for(uint16_t i = 0; i<length; i++)
    {
        printf("%c",data[i]);

    }
    printf("\n");

}

void usb_send(const uint8_t* data, uint16_t length)
{
    printf("Sending over USB : ");
    for(uint16_t i = 0; i<length; i++)
    {
        printf("%c",data[i]);

    }

    printf("\n");


}

/*
    These #ifndef checks will be active only if both USE_UART and USE_USB are not defined.
    If even one of them (USE_UART or USE_USB) is already defined, the condition inside #ifndef fails immediately, and the block inside is skipped.

    Is USE_UART defined?
    → If YES, skip the first #ifndef USE_UART block completely.

    If not, then check:
    Is USE_USB defined?
        → If YES, skip.
        → If NO, trigger #error "No communication method defined!"


*/
void send_data(const uint8_t* data, uint16_t length)
{

#ifdef USE_UART
    uart_send(data, length);
#endif // USE_UART

#ifdef USE_USB
    usb_send(data, length);
#endif // USE_USB

#ifndef USE_UART
#ifndef USE_USB
    #error "No communication method defined !"
#endif // USE_USB
#endif // USE_UART
}


int main(void)
{
    const uint8_t message [] = "Hello World!";
    send_data(message,sizeof(message) -1); // -1 to exclude null terminator

}
