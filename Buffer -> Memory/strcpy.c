/*
    The C library strcpy() function accepts two parameter which copies the string pointed to,
    by src to dest. This function is essential for maintaining and upgrading of older system.

    Following  is the syntax of C library strcpy() function-

    char *strcpy(char *dest, const char *src)

    Parameters:

    -> dest-> This is the pointer to the destination array where the content is to be copied
    -> src -> This is the string to be copied

    *** Ensure that the destination array has enough space to hold the source string,
    containing the null terminator.****


*/

#include <stdio.h>
#include <string.h>

int main()
{
    char src[40];
    char dest[100];

    memset(dest, '\0', sizeof(dest));
    strcpy(src, "This is B4T Workstation");

    strcpy(dest,src);

    printf("Final copied string: %s\n", dest);

    return 0;


}
