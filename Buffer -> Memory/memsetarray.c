/*
    Below the program initializes an integer array with zeroes using memset().


*/

#include <stdio.h>
#include <string.h>


int main()
{
    int arr[10];

    memset(arr,0,sizeof(arr));
    //printf("%d\n",sizeof(arr));
/*

    Since arr is an array of 10 integers, the total size of the array is:
    10 elements * 4 bytes per element = 40 bytes
    Thus, sizeof(arr) returns 40 because the size of the entire array is 40 bytes.

*/

    printf("Array after memset(): ");

    for(int i =0; i<10; i++)
        {
        printf("%d",arr[i]);

    }
    printf("\n");

    return 0;



}
/*    Output: Array after memset(): 0000000000  */
// if we use int arr[40] and i<40 then it will show 40->0 [0000000000 0000000000 0000000000 0000000000]
