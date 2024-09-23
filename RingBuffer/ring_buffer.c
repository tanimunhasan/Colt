#include<stdio.h>

 /* Write API Algorithm
 Is Buffer Full
    New element cannot be inserted.
else
    Buffer[Head] = data
    Increment Head pointer
    If(Head == Tail)
        IsFull_Flag = True
*/

#include <stdio.h>

#define BUFFER_SIZE 6

// Circular buffer and flags
int CircularBuffer[BUFFER_SIZE];  // Define the buffer with the correct size
int Head = 0;                     // Head index
int Tail = 0;                     // Tail index
int IsFull_Flag = 0;              // Flag to indicate if the buffer is full

// Function to check if the buffer is full
int Is_BufferFull() {
    return (Head == Tail && IsFull_Flag == 1); // Buffer is full if head equals tail and the full flag is set
}

// Function to check if the buffer is empty
int Is_BufferEmpty() {
    return (Head == Tail && IsFull_Flag == 0); // Buffer is empty if head equals tail and the full flag is not set
}

// Function to write to the buffer
void API_WriteToBuffer(int data_element) {
    if (Is_BufferFull()) {
        printf("\nBuffer is Full");
    } else {
        CircularBuffer[Head] = data_element;  // Write data to the buffer
        printf("\nAPI_WriteToBuffer: %d", CircularBuffer[Head]);
        Head = (Head + 1) % BUFFER_SIZE;      // Update head position in a circular manner

        // Check if the buffer became full after writing
        if (Head == Tail) {
            IsFull_Flag = 1;
        }
    }
}

// Main function to test the buffer
/*
int main() {
    // Test writing to the buffer with sample data
    API_WriteToBuffer(10);
    printf("      Head = %d Tail = %d IsFull_Flag = %d\n", Head, Tail, IsFull_Flag);

    API_WriteToBuffer(20);
    printf("      Head = %d Tail = %d IsFull_Flag = %d\n", Head, Tail, IsFull_Flag);

    API_WriteToBuffer(30);
    printf("      Head = %d Tail = %d IsFull_Flag = %d\n", Head, Tail, IsFull_Flag);

    API_WriteToBuffer(40);
    printf("      Head = %d Tail = %d IsFull_Flag = %d\n", Head, Tail, IsFull_Flag);

    API_WriteToBuffer(50);
    printf("      Head = %d Tail = %d IsFull_Flag = %d\n", Head, Tail, IsFull_Flag);

    API_WriteToBuffer(60);
    printf("      Head = %d Tail = %d IsFull_Flag = %d\n", Head, Tail, IsFull_Flag);

    API_WriteToBuffer(70);
    printf("      Head = %d Tail = %d IsFull_Flag = %d\n", Head, Tail, IsFull_Flag);

    return 0;
}

*/

/* Is Buffer Empty
    No element available to read
else
    data = Buffer[Tail]
    increment Tail Pointer
    IsFull_Flag = False
*/


 void API_ReadFromBuffer(int *data_element)
{
    if(Is_BufferEmpty())
    {
        printf("\nBuffer is empty");
    }
    else
    {
        *data_element = CircularBuffer[Tail];
        printf("\nAPI_ReadFromBuffer: %d", CircularBuffer[Tail]);
        Tail = (Tail +1) % BUFFER_SIZE;
        IsFull_Flag = 0;
    }
}


int main(){

int cnt;
API_ReadFromBuffer(&cnt);
printf("            Head  = %d   IsFull_Flag = %d", Head, Tail, IsFull_Flag);

API_WriteToBuffer(10);
printf("            Head  = %d   IsFull_Flag = %d", Head, Tail, IsFull_Flag);

API_WriteToBuffer(20);
printf("            Head  = %d   IsFull_Flag = %d", Head, Tail, IsFull_Flag);

API_ReadFromBuffer(&cnt);
printf("           Head  = %d    IsFull_Flag = %d", Head, Tail, IsFull_Flag);

return 0;
}


