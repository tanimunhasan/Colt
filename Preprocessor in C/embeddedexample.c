/* The preprocessor will ONLY compile the first matching block:
    ✅ #ifdef USE_UART is true ➔ only uart_send() will be compiled and called.
    ❌ #elif defined(USE_USB) will be skipped, even though USE_USB is also defined.
    This is because #ifdef/#elif/#else behaves like a first-match "if-else if-else" ladder — only one branch is taken.

*/

/*
✅ If you want both UART and USB active together?
*******You must not use #elif, but check both independently:*****

*/

#include<stdio.h>
#include<stdint.h>

// In config.h

#define USE_UART

#define USE_USB
void uart_send(uint8_t* data, uint16_t length)
{
    printf("Sending over UART: ");
    for(uint16_t i = 0; i<length; i++)
    {

        printf("%c",data[i]);
    }
    printf("\n");

}

void usb_send(uint8_t* data, uint16_t length)
{
    printf("SENDING over USB:  ");
    for (uint16_t i = 0; i<length; i++)
    {
        printf("%c", data[i]);

    }
    printf("\n");


}

void send_data(const uint8_t* data, uint16_t length)
{

#ifdef USE_UART
    uart_send(data, length);
#elif defined(USE_USB)
    usb_send(data,length);
#else
    #error "No communicatoin method define!"

#endif // USE_UART
}


int main(void)
{

    const uint8_t message[] =  "Hello World";
    send_data(message, sizeof(message)-1); // -1 to exclude null terminator

    return 0;

}
