#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


pthread_cond_t cond;
pthread_mutex_t lock;
void* func1(void* arg)
{
    while(1)
    {
        pthread_cond_wait(&cond, &lock);
        printf("消费了一个\n");
       sleep(1);
    }

}


void* func2(void* arg)
{
    while(1)
    {
        pthread_cond_signal(&cond);
        printf("生产了一个\n");
       sleep(1);
    }
}



int main()
{
    
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_t t1, t2;
    pthread_create(&t1, NULL, func1, NULL);
    pthread_create(&t2, NULL, func2, NULL);


    getchar();

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&lock);
    
}
