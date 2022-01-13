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
    execvp("ls", arg);
    printf("helloworld\n");
}
