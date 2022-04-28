#include <unistd.h>

#include <iostream>

#include "block_queue.hpp"

using namespace std;

void *routine1(void *arg)
{

    block_queue<int> *bq = (block_queue<int> *)arg;

    int i = 1;
    while (1)
    {

        cout << "producter: " << i << endl;
        bq->Put(i);
        usleep(1000);
        i++;
    }

    return nullptr;
}

void *routine2(void *arg)
{
    block_queue<int> *bq = (block_queue<int> *)arg;

    while (1)
    {
        int ret = bq->Get();
        cout << "Comsumer: " << ret << endl;
        cout << "\n";
    }

    return nullptr;
}

int main()
{
    block_queue<int> bq;

    pthread_t t1, t2;
    pthread_create(&t1, NULL, routine1, &bq);
    pthread_create(&t2, NULL, routine2, &bq);

    pthread_join(t1, NULL);
    pthread_join(t1, NULL);
}