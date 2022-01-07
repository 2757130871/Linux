#include<stdio.h>
#include <signal.h>
#include <unistd.h>

int main()
{
    int pid = fork();

    if(0 == pid)
    {
        while(1)
        {
            printf("child on work..\n");
            sleep(1);
        }
        return 0;
    }

    printf("准备杀死子进程\n");
    sleep(4);

    kill(pid, 15);


}
