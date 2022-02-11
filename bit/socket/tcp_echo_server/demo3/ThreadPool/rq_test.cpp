#include "RingQueue.hpp"



void* Comsumer(void* arg)
{

    RingQueue<int>* rq = (RingQueue<int>*)arg; 
    
    while(1)
    {
        cout << "消费一个：" << rq->get() << endl;
        sleep(1);
    }

    return NULL;
}   

void* Producter(void* arg)
{
    
    RingQueue<int>* rq = (RingQueue<int>*)arg; 

    int n = 1;
    while(1)
    {
        if(n == 6)
            n = 1;
        rq->put(n);
        printf("生产一个 %d\n", n);
        n++;
        usleep(10000);
    }

    return NULL;
}

int main()
{
    RingQueue<int>* rq = new RingQueue<int>();
    
    pthread_t t1, t2;
    pthread_create(&t1, NULL, Comsumer,rq);
    pthread_create(&t2, NULL, Producter, rq);
        
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}
