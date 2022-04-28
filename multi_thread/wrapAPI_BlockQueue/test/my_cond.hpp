#pragma once
#include <pthread.h>

class cond
{
public:
    cond()
    {
        pthread_cond_init(&_cond, nullptr);
    }

    ~cond()
    {
        pthread_cond_destroy(&_cond);
    }

    void wait(pthread_mutex_t *lock)
    {
        pthread_cond_wait(&_cond, lock);
    }

    void signal(pthread_mutex_t *lock)
    {
        pthread_mutex_unlock(lock);
    }

private:
    pthread_cond_t _cond;
};