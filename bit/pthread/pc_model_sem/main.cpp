#include "circular_queue.hpp"
#include <unistd.h>
using namespace std;

void* Producter(void* arg)
{

    CircularQueue* cq = (CircularQueue*)arg;
    int i = 1;
    while(1)
    {
        cq->Put(i++);
        if(i == 5)
            i = 1;
    }
    
    return NULL;
}

void* Consumer(void* arg)
{
    CircularQueue* cq = (CircularQueue*)arg;
    while(1)
    {
        cout << cq->Get() << endl;
        sleep(1);
    }
    
    return NULL;
}
int main()
{
    pthread_t t1, t2;
    CircularQueue* cq = new CircularQueue(5);
    pthread_create(&t1, NULL, Producter, cq);
    pthread_create(&t2, NULL, Consumer, cq);
    
    pthread_exit(NULL);
}
