#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int tickets = 100;

void* func1(void* arg)
{
    while(1)
    {
        if(tickets > 0)
        {
            usleep(10000);
            printf("抢到第%d张票了,\n", tickets);
            tickets--;
        }
        else{
            break;
        }
    }

    return NULL;
}




int main()
{
    
    pthread_t tid1, tid2, tid3, tid4;
    pthread_create(&tid1, NULL, func1, NULL);
    pthread_create(&tid2, NULL, func1, NULL);
    pthread_create(&tid3, NULL, func1, NULL);
    pthread_create(&tid4, NULL, func1, NULL);

    
    pthread_exit(NULL);

}
