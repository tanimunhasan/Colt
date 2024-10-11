/*   Here, we create a C program for intializing character array with a specific character('Z') using memset(). */

#include<stdio.h>
#include<string.h>


int main()
{
    char str[20];
    memset(str,'Z', 10);
    printf("String after memset(): %s\n", str);
    return 0;

}

/* Output:  String after memset(): ZZZZZZZZZZ       */
