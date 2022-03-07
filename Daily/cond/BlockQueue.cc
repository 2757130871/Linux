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
		pthread_cond_destroy(&cond_p);
		pthread_cond_destroy(&cond_c);
	}

	size_t Size()
	{
		return que.size();
	}
	bool empty()
	{
		return que.size() == 0;
	}

    bool full()
    {
        return que.size() >= capacity;
    }

	void put(int data)
	{
		while (full()) {
			pthread_cond_signal(&cond_c);
			pthread_cond_wait(&cond_p, &_lock);
		}
		que.push(data);
		//Unlock();
	}

	int get()
	{
		while (empty()) {
			pthread_cond_signal(&cond_p);
			pthread_cond_wait(&cond_c, &_lock);
		}
		int ret = que.front();
		que.pop();
		//Unlock();

		return ret;
	}


	void init()
	{
		pthread_mutex_init(&_lock, NULL);
		pthread_cond_init(&cond_p, NULL);
		pthread_cond_init(&cond_c, NULL);
	}

	void Lock() {
		pthread_mutex_lock(&_lock);
	}

	void Unlock()
	{
		pthread_mutex_unlock(&_lock);
	}

private:
	const int capacity;
	pthread_mutex_t _lock;
	queue<int> que;
	pthread_cond_t cond_p;
	pthread_cond_t cond_c;
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
		cout << "procudt one..." << endl;
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
