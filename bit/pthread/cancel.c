
//线程的取消并不是实时的，必须在用户态转内核态的时候才触发，如调用一些系统调用函数。在执行原子性操作的线程不可被cancel

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* func(void* arg)
{
    while(1)
    {
        sleep(1);
        printf("child thread working...\n");
    }
}

int main()
{
    pthread_t tid1;
    pthread_create(&tid1, NULL, func, NULL);

    printf("三秒后取消线程\n");
    sleep(3);
    pthread_cancel(tid1);
    
}
