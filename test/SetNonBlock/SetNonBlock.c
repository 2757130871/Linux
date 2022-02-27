#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

void SetNonBlock(int fd)
{
    int fl = fcntl(fd, F_GETFL);
    if(fl < 0) 
    {
        printf("fcntl error\n");
        exit(-1);
    }
    
    fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

int main()
{
    SetNonBlock(0);
    while(1)
    {
        sleep(1);
        char c;
        int rd = read(0, &c, 1);
        
        if(rd  > 0)
        {
            printf("%c\n", c);
        }
        else if(errno == EAGAIN)
        {
            printf("%s\n",strerror(errno));
        }
        else
        {
            printf("%d\n", rd);
        }
        printf("-----------------\n");
    }
}
