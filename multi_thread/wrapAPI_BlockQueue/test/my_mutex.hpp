#pragma once
#include <pthread.h>

namespace cxr
{

    class mutex
    {
    public:
        mutex()
        {
            pthread_mutex_init(&_lock, nullptr);
        }

        ~mutex()
        {
            pthread_mutex_destroy(&_lock);
        }

        void lock()
        {
            pthread_mutex_lock(&_lock);
        }
        void unlock()
        {
            pthread_mutex_unlock(&_lock);
        }

    private:
        pthread_mutex_t _lock;
    };
}
