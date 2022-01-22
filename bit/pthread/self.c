#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* fun1(void* arg)
{
    while(1)
    {
        printf("pthread1 pthread_slef = %ld\n",pthread_self());
        sleep(1);
    }
}

void* fun2(void* arg)
{
    while(1)
    {

        printf("pthread3 pthread_slef = %ld\n",pthread_self());
        sleep(1);
    }

}


int main()
{
    pthread_t pArr[2];
    int i;
        pthread_create(pArr, NULL, fun1, NULL);
        pthread_create(pArr + 1, NULL, fun2, NULL);
   
    sleep(1000);
}
