#include "RingQueue.hpp"
#include  "ThreadPool.hpp"




int main()
{
    ThreadPool* tp = new ThreadPool();
    tp->init();
    
    //主线程负责塞任务
    while(1)
    {
        Task* task = new Task();

        tp->Put(task);
        printf("塞了一个任务\n");
        usleep(500000);
    }
    
}
