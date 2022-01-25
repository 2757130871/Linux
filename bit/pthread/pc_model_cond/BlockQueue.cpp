#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <queue>
#include <cmath>

class BlockQueue{

private:
    std::queue<int> _que;
    const size_t _capacity = 5;
    pthread_mutex_t lock;
    pthread_cond_t cond_p;
    pthread_cond_t cond_c;
public:
    
    void Put()
    {
        while(IsFull())
        {
            WakeUpConsumer();
            ProducterWait();
        }
        
        int data = 1;
        _que.push(data);        
        std::cout << "product data" << std::endl;
    }

    int Get()
    {
        while(IsEmpty())
        {
            WakeupProducter();
            ConsumerWait(); 
        }
        
        int ret = _que.front();
        _que.pop();
        std::cout << "consume data" << std::endl;
        return ret;
    }

    bool IsEmpty()
    {
        return _que.empty();
    }

    bool IsFull()
    {
        return _que.size() >= _capacity;
    }
    
    void ConsumerWait()
    {
        pthread_cond_wait(&cond_c, &lock);
    }
    void ProducterWait()
    {
        pthread_cond_wait(&cond_p, &lock);
    }
    
    void WakeUpConsumer()
    {
        pthread_cond_signal(&cond_c);
    }
    
    void WakeupProducter()
    {
        pthread_cond_signal(&cond_p);
    }
};
