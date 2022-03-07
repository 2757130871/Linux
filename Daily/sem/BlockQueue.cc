#include <pthread.h>
#include <queue>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cmath>
#include <semaphore.h>

using namespace std;


class BlockQueue
{
public:

	BlockQueue(int cap) : capacity(cap) {
		init();
	}

	~BlockQueue() {
		sem_destroy(&data);
		sem_destroy(&blank);
	}
	void init()
	{
		sem_init(&data, 0, 0);
		sem_init(&blank, 0, capacity);
	} 

	void P(sem_t* sem)
	{
		sem_wait(sem);

	}
	void V(sem_t* sem)
	{
		sem_post(sem);
	}

	int get()
	{
		P(&data);
		int ret  = que.front();
		que.pop();
		V(&blank);

		return ret;
	}

	void put(int e)
	{
		P(&blank);
		que.push(e);
		V(&data);
	}

private:

	const size_t capacity = 0;
	//pthread_mutex_t _lock;
	queue<int> que;

	//pthread_cond_t cond;

	int index_p = 0;
	int index_c = 0;
	
	sem_t blank;
	sem_t data;
};


void* Consumer(void* arg)
{
	BlockQueue* b1 = (BlockQueue*)arg;

	while (1)
	{
		cout << "Consumer: " << b1->get() << endl;
        sleep(1);
	}
}

void* producter(void* arg)
{
	BlockQueue* b1 = (BlockQueue*)arg;
	while (1)
	{
		b1->put(rand() % 100);
		cout << "procudt one..." << endl;
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
