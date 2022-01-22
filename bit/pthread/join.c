
//线程推出也是需要回收资源的，有点类似僵尸进程一样，join函数
//第一个参数为tid，第二个二级指针，存放指定线程的返回值。等待时会阻塞。

#include <stdio.h>
#include <unistd.h>

#include <pthread.h>
#include <signal.h>
void* fun1(void* arg)
{
    int count = 4;
    while(count--)
    {
        printf("i am sleeping\n");
        sleep(1);
    }
    printf("%ld\n",(long)arg);
    return (void*)10;
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, NULL, fun1, (void*)5);
    void* ret;
    pthread_join(tid, &ret); 
    printf("fun1 ret code: %ld\n", (long)ret);
    
}
