#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main()
{
    
    printf("helloworld\n");
    //execl("/usr/bin/ls","ls", "-l",NULL);
    execlp("ls","ls", "-l",NULL);
    printf("helloworld\n");
}
