#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main()
{
    
    int fd[2];
    pipe(fd);
    
    int pid = fork();
    if(pid == 0){
        //child write
        close(fd[0]);
        const char* msg = "i am child\n";
        int time = 0;
        while(1)
        {
            if(time == 4)
                exit(1);
            ssize_t count = write(fd[1], msg, strlen(msg));
            sleep(1);
            time++;
        }
        

        exit(0);
    }
    else{
        //parent read
        close(fd[1]);

        char buf[64];
        while(1)
        {
            ssize_t count = read(fd[0], buf, 63);
            if(count == 0)
                printf("read 0\n");
            buf[count] = 0;
            printf("%s\n", buf);
            sleep(1);
        }


    }
    
 

}
