/* malloc(number_of_bytes)  */
#include<stdio.h>
#include<stdlib.h>

void memory_crash(int size);
int main(void)
{
    /*
    int *a = malloc(sizeof(int)*10);

    for(int i = 0; i<10; i++) a[i] = 10 - i;
    for (int i = 0; i<10; i++)
        printf("a[%d] = %d\n",i,a[i]);
    printf("\n");

    printf("a: %p\n",a);
    free(a);
    */
    while(1)
    {
        memory_crash(128800);
    }
    return 0;
}

void memory_crash(int size)
{
    int *a = malloc(size);


}
