#pragma once
#include <pthread.h>

class my_mutex
{
public:
    my_mutex()
    {
        pthread_mutex_init(&lock1, nullptr);
    }

    ~my_mutex()
    {
        pthread_mutex_destroy(&lock1);
    }

    void lock()
    {
        pthread_mutex_lock(&lock1);
    }
    void unlock()
    {
        pthread_mutex_unlock(&lock1);
    }

private:
    pthread_mutex_t lock1;
};
