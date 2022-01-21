#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void* func1(void* arg)
{
    while(1)
    {
        printf("i ma thread1\n");
        sleep(1);
    }
}


void* func2(void* arg)
{
    while(1)
    {
        printf("i ma thread2\n");
        sleep(1);
    }
    
}



int main()
{
    pthread_t tid1, tid2;
     pthread_create(&tid1, NULL, func1, (void*)NULL);
     pthread_create(&tid2, NULL, func2, (void*)NULL);
    
     while(1)
     {
         printf("main thread run \n");
         sleep(1);
     }
}
