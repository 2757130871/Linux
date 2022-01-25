#include <queue>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>

class CircularQueue {

private:
    const size_t _capacity;
    sem_t _blank;
    sem_t _data;
    size_t _index_d  = 0;
    size_t _index_b = 0;
    std::vector<int> _v; 
    
public:
    CircularQueue(size_t capacity):
        _capacity(capacity)
    {
        _v.resize(_capacity); 
        sem_init(&_blank, 0, _capacity); 
        sem_init(&_data, 0, 0);

    }
        
    void Put(int x)
    {
        sem_wait(&_blank); 
        
        _v[_index_b++] = x; 
        _index_b %= _capacity; 
        std::cout << "生产了一个" << std::endl;
        sem_post(&_data);
    }

    int Get()
    {
        
        sem_wait(&_data); 
        int ret;
        ret = _v[_index_d++];
        _index_d %= _capacity; 

        std::cout << "消费了一个" << std::endl;
        sem_post(&_blank);
      
        return ret;
    }
    
    

};
