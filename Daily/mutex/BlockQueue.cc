#include <pthread.h>
#include <queue>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cmath>

using namespace std;


class BlockQueue
{
public:

    BlockQueue(int cap) : capacity(cap) {
        init();
    }

    ~BlockQueue() {
        pthread_mutex_destroy(&_lock);
    }

    size_t Size()
    {
        return que.size();
    }

    bool empty()
    {
        return que.size() >= capacity;
    }
    void put(int data)
    {
        while (!que.empty()) {
            Lock();
        }
        que.push(data);
        Unlock();
    }

    int get()
    {
        while (que.empty()) {
            Lock();
        }
        int ret = que.front();
        que.pop();
        Unlock();

        return ret;
    }

    void init()
    {
        pthread_mutex_init(&_lock,NULL);
    }

    void Lock() {
        pthread_mutex_lock(&_lock);
    }

    void Unlock()
    {
        pthread_mutex_unlock(&_lock);
    }

private:
    const int capacity = 0;
    pthread_mutex_t _lock;
    queue<int> que;
};


void* Consumer(void* arg)
{
    BlockQueue* b1 = (BlockQueue*)arg;

    while (1)
    {
        cout << b1->get() << " current size:" << b1->Size() << endl;
    }
}

void* producter(void* arg)
{
    BlockQueue* b1 = (BlockQueue*)arg;
    while (1)
    {
        b1->put(rand() % 100);
        cout << "procudt one..." <<endl;
        sleep(1);
    }
}

int main()
{

    BlockQueue b1(5);
 
    pthread_t t1, t2;
    pthread_create(&t1, NULL, producter, &b1);
    pthread_create(&t2, NULL, Consumer, &b1);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}
