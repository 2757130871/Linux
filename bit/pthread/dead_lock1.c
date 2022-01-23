
//死锁场景演示：
//两个线程都获得了不同的锁，此时都在申请对方手里的锁，此时两个线程都在阻塞

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int s1 = 1,s2 = 1;
pthread_mutex_t lock1, lock2;

void* func1(void* arg)
{
    while(1)
    {
        pthread_mutex_lock(&lock1);       
        printf("线程1抢到lock1\n");
        pthread_mutex_lock(&lock2);       
        printf("线程1抢到lock2\n");
       

        pthread_mutex_unlock(&lock1);       
        pthread_mutex_unlock(&lock2);       


    }
}

void* func2(void* arg)
{

    while(1)
    {
        
        pthread_mutex_lock(&lock2);       
        printf("线程2抢到lock1\n");
        pthread_mutex_lock(&lock1);       
        printf("线程2抢到lock2\n"); 

        pthread_mutex_unlock(&lock2);       
        pthread_mutex_unlock(&lock1);       
    }
}


int main()
{
    pthread_t t1, t2;
    pthread_create(&t1, NULL, func1, NULL);
    pthread_create(&t2, NULL, func2, NULL);
    pthread_mutex_init(&lock1, NULL);
    pthread_mutex_init(&lock2, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
}
