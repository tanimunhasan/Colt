#include<stdio.h>

typedef struct {
    unsigned powerOn            : 1;
    unsigned errorDetected      : 1;
    unsigned dataReady          : 1;
    unsigned reserver           : 5;    // Reserved to make a full byte (optional)
} StatusFlags_t;

int main(void)
{

    StatusFlags_t status = {0}; // Initialize all flags to 0

    // Set some flags
    status.powerOn = 1;
    status.dataReady = 1;

    // Check flags

    if(status.powerOn)
    {
        printf("System is powered on. \n");

    }
    if(status.dataReady)
    {
        printf("Data is ready. \n");
    }

    /* ========================== */

    // Check whether flag sets 0 or 1,
    // if 0 that means set clear, if 1 set by flag

    //Clear flags
    status.dataReady = 0;
    status.powerOn = 0;
    if(!status.dataReady)
    {
        printf("Data is no longer ready. \n");
    }

    if(!status.powerOn)
    {
        printf("System is not powered. \n");
    }


    return 0;
}
