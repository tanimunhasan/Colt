#include<stdio.h>

#define US 0
#define UK 1
#define France 2
#define Germany 3
#define Country Germany

int main()
{
    #if Country == US || Country == UK
        #define Greeting "Hello."
    #elif Country == France
        #define Greeting "Bonjure."
    #elif Country == Germany
        #define Greeting "Guten Tag."
    #endif // Country
    printf("Greeting is: %s",Greeting);
}
