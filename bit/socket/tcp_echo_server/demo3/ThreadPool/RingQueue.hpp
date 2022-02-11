#include <pthread.h>
#include <iostream>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>


using namespace std;



template<class T>
class RingQueue
{
public:
    
    RingQueue(int capacity = 5) : _capacity(capacity) ,_v(capacity) {
        init();
    } 

    void init()
    {
        sem_init(&_blank, 0, _capacity);
        sem_init(&_data, 0, 0); 
    }
    
    void P(sem_t& sem)
    {
       sem_wait(&sem);
    }
    
    void V(sem_t& sem)
    {
        sem_post(&sem);    
    }

    void put(const T& x)
    {
        P(_blank);
        _v[p_index++] = x;
        p_index %= _capacity;
        V(_data);
    }

    T get()
    {
        T ret;
        P(_data);
        ret = _v[c_index++];
        c_index %= _capacity;
        
        V(_blank);
       return ret;
    }

private:
    const int _capacity;
    vector<T> _v;
    sem_t _blank;
    sem_t _data;
    int p_index = 0;
    int c_index = 0;
};
