#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>


int main()
{
    //wait函数 等待子进程结束并回收资源
    
    int pid = fork();
    if(pid == 0){
        int i = 0;
        for(i = 0;i < 10;i++)
        {
          printf("%d\n",i);
          sleep(1);
        }
        exit(10);
    }
    int val = 0;
    printf("父进程开始等待\n");
    int ret = wait(&val);
    
    printf("回收完成\n");
}

