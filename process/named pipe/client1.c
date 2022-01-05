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
    int fd1 = open("fifo1",O_WRONLY);
    int fd2 = open("fifo2",O_RDONLY);

    char buf[64];
    
    while(1)
    {
        memset(buf, 0, 64);
        
        read(fd2, buf, 64);
        buf[strlen(buf)- 1] = 0;


        printf("%s\n", buf);

        memset(buf, 0, 64);
        int rd_count = read(STDIN_FILENO, buf, 64);
        
                          
        while(getchar() != '\n')
              ;                       
        write(fd1, buf, rd_count);
        
    }

}
