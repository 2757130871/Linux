#include <stdio.h>
#include <unistd.h>
#include <signal.h>

//volatile 保持内存的可见性，如果程序被编译器优化的很厉害，那么有一些变量就可能一直存在寄存器中，cpu不会一直去内存中访问，如果此时有另外一个
//执行流修改了这个值，这对于cpu是不可见的。所以这里需要用到volatile关键字。
volatile int flag = 0;

void handler(int sig)
{
    printf("set flag = 1 \n");
    printf("receive sig = %d \n",sig);
    flag = 1;
}



int main()
{
    signal(2, handler);
    while(!flag)
    {
    }
    
}
