#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

void handler(int sig)
{
    printf("receive sig :%d\n", sig);
    exit(0);
}


//子进程退出的时候会对父进程发送17号信号， SIGCHLD 。waitpid就是信号处理函数调用的。
int main()
{
    pid_t ret = fork();
    if(ret == 0)
    {
            printf("子进程5秒后退出\n");
            sleep(5);
        return 0;
    }
    
    signal(17, handler);
    
    while(1)
    {
        printf("等待子进程退出\n");
        sleep(1);
    }

}
