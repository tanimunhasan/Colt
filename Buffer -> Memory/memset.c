/*
    Following in the syntax of the C library memset() function--
    void *memset(void *str, int c, size_t n)

    Parameters
    This functions accepts the following parameters-

    -> str -> This is a pointer to the block of memory to fill
    -> c -> This is a second parameter of type integer and converts it to an unsigned char before
    using it to fill a block of memory
    -> n -> This is the number of bytes to be set to the value.


*/


#include <stdio.h>
#include <string.h>

int main()
{
    char str[50];

    strcpy(str, "Welcome to B4T Station");
    puts(str);

    memset(str,'#',7);
    printf(str);

    return 0;
}


/*
    puts() ->  puts() is a function used to display strings on screen


*/



/*
Output: 
Welcome to B4T Station
####### to B4T Station

*/
