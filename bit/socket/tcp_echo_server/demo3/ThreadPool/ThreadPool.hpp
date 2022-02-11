#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <queue>
#include <vector>

using namespace std;

class Task {

public:
    void Run()
    {
        std::cout << "Task Running...." << "pthread:" << pthread_self() << endl;
    }
    
};

class ThreadPool 
{
public:

    ThreadPool(int capacity = 5): _capacity(capacity), _pool(_capacity)
    {

    }

    ~ThreadPool()
    {
        pthread_mutex_destroy(&_lock);  
        pthread_cond_destroy(&_cond);  
    }

    void lock_queue()
    {
        pthread_mutex_lock(&_lock);
    }
    
    void unlock_queue()
    {
        pthread_mutex_unlock(&_lock);
    }

    void wait_queue()
    {
        pthread_cond_wait(&_cond, &_lock);
    }
    
    static void* handler(void* arg)
    {
        Task* t;
        ThreadPool* tp = (ThreadPool*)arg;
        
        while(1)
        {
            tp->lock_queue();
        
            while(tp->_que.empty()){
                tp->wait_queue();
            }
            t = tp->_que.front();   
            tp->_que.pop();
            tp->unlock_queue();
        
            t->Run();
        }
        return NULL;
    }
    
    void init()
    {
        pthread_mutex_init(&_lock, NULL);  
        pthread_cond_init(&_cond, NULL);  
        
        for(int i = 0;i < _capacity;i++){
            pthread_t t1;
            pthread_create(&t1, NULL, handler, this);
            pthread_detach(t1);
            _pool.push_back(t1);
        }
    }

    void Put(Task* t)
    {
        lock_queue();
        _que.push(t);
        unlock_queue();

        pthread_cond_signal(&_cond);
    }

private:
    const int _capacity; 
    queue<Task*> _que;
    vector<pthread_t> _pool;
    pthread_mutex_t _lock;
    pthread_cond_t _cond;
};
