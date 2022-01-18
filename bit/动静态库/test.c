#include <stdio.h>
#include "add.h"
#include "sub.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        printf("input errro\n");
        exit(0);
    }


    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    
    printf("%d - %d = %d", a, b, sub(a, b));

}
