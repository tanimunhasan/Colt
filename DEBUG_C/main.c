#include<stdio.h>

//#define JUST_CHECKING
#define LIMIT 4

int main(void)
{
    int i = 0;
    int total = 0;

    for(i=1; i<= LIMIT; i++)
    {
        total += 2*i*i+1;

        #ifdef JUST_CHECKING
            printf("i=%d, running total = %d\n",i,total);
        #endif // JUST_CHECKING
    }
    printf("Grand total = %d\n",total);
}
