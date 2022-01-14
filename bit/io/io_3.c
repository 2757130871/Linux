#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>


int main()
{
    close(1);
    
    int fd = open("txt", O_CREAT | O_RDWR, 0644); 

    char buf[] = "helloworld:write\n";
    write(fd, buf, sizeof(buf));
    printf("helloworld\n");

    fprintf(stdout, "helloworld: fprintf\n");

    fork();
    fflush(stdout);
}
