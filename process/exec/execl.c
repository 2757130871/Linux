#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
int main()
{
    printf("helloworld\n");
    execl("/usr/bin/ls", "ls", "-a", "-l", NULL);
    printf("helloworld\n");
}
