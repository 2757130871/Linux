#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// alarm函数在计时完成后给当前进程发送14号信号

#include <stdlib.h>
int count = 0;
void test(int no)
{
    printf("catch singal:%d \n", count);;
    exit(0);
}    

    
int main()
{
    signal(14, test);

    alarm(1);
    while(1)
    {
        count++;
    }
    
}
