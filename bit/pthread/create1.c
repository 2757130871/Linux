#include <stdio.h>
#include <unistd.h>

#include <pthread.h>

void* fun1(void* arg)
{
    int count = 4;
    while(count--)
    {
        printf("i am sleeping\n");
        sleep(1);
    }
    printf("%ld\n",(long)arg);
    return NULL;
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, NULL, fun1, (void*)5);

    getchar();
}
