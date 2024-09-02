#include<stdio.h>
#include<string.h>

struct student{
    int id;
    char name[30];
    float percentage;
}student1,student2,student3;

int main()
{

strcpy(student1.name,"Angelina");
student1.id = 1;
student1.percentage = 90.5;
printf("Id is: %d \n",student1.id);
printf("Name is: %s \n",student1.name);
printf("Percentage is: %f \n",student1.percentage);

return 0;
}

