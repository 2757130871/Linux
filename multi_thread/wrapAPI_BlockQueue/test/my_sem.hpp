#pragma once

#include <vector>

#include <pthread.h>
#include <semaphore.h>
#include <cassert>

namespace cxr
{

    class sem
    {
    public:
        sem(int sum)
        {
            assert(sem_init(&_sem1, 0, sum) == 0);
        }

        ~sem()
        {
            assert(sem_destroy(&_sem1) == 0);
        }

        bool P()
        {
            return sem_wait(&_sem1) == 0;
        }

        bool V()
        {
            return sem_post(&_sem1) == 0;
        }

    private:
        sem_t _sem1;
    };
} // namespace  cxr
