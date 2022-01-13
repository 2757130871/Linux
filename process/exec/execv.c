#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
int main()
{

    char* arg[] = {
     
        "ls",
        "-a",
        "-l",
        NULL,
            
    };
    printf("helloworld\n");
    execv("/usr/bin/ls", arg);
    printf("helloworld\n");
}
