#include "pc_model.hpp"

void* product(void* arg)
{
    BlockQueue* bq = (BlockQueue*)arg;
    while(1)
    {
        bq->Put();
    }

    return NULL;
}

void* consume(void* arg)
{
    BlockQueue* bq = (BlockQueue*)arg;
    while(1)
    {
        std::cout <<  bq->Get() << std::endl;
        usleep(60000);
    }   
    return NULL;
}

int main()
{
    BlockQueue* bq = new BlockQueue();
    pthread_t t1, t2;
    pthread_create(&t1, NULL, product, (void*)bq);
    pthread_create(&t2, NULL, consume, (void*)bq);

    pthread_exit(nullptr);
}
