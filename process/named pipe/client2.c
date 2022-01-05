#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>


//实现两进程之间聊天功能
int main()
{
    int fd1 = open("fifo1",O_RDONLY);
    int fd2 = open("fifo2",O_WRONLY);

    char buf[64];
    
    while(1)
    {
        memset(buf, 0, 64);
        int rd_count = read(STDIN_FILENO, buf, 64);
                          
        while(getchar() != '\n')
              ;                       

        write(fd2, buf , rd_count); 
        memset(buf, 0, 64);
        read(fd1, buf, 64);
        
        printf("%s\n", buf);
    }

}
