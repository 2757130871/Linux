#pragma once
#include <queue>
#include <pthread.h>
#include <iostream>
#include "Task.hpp"

#include "Util.hpp"

#define NUM 4

class ThreadPool
{
private:
    int num = NUM;
    bool stop = false;
    std::queue<Task *> task_queue;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    static ThreadPool *thread_pool;

    ThreadPool(int _num = NUM) : num(_num) //单例模式 不允许实例化
    {
        pthread_mutex_init(&lock, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ThreadPool(const ThreadPool &tp) {}

    bool InitThreadPool()
    {
        for (int i = 0; i < num; i++)
        {
            pthread_t t1;
            if (pthread_create(&t1, NULL, ThreadRoutine, this) != 0)
            {
                LOG(FATAL) << "CREATE thread error!!!!\n";
                exit(-1);
            }
            pthread_detach(t1); //分离
        }
        return true;
    }

public:
    static ThreadPool *GetInstance()
    {
        if (thread_pool == nullptr)
        {
            pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_lock(&lock);
            if (thread_pool == nullptr)
            {

                thread_pool = new ThreadPool();
                thread_pool->InitThreadPool();
                std::cout << "---------- create thread" << std::endl; // DEBUG
            }
            pthread_mutex_unlock(&lock);
        }

        return thread_pool;
    }

    ~ThreadPool()
    {
        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);
    }

    bool IsStop()
    {
        return stop;
    }
    bool IsEmpty()
    {
        return task_queue.empty();
    }
    bool IsFull()
    {
        return false; //???????????
    }

    //
    static void *ThreadRoutine(void *arg)
    {
        ThreadPool *tp = (ThreadPool *)arg;

        while (true)
        {
            Task *task = NULL;

            tp->Lock(); //加锁
            while (tp->IsEmpty())
            {
                tp->ThreadWait();
            }

            tp->PopTask(&task);
            tp->UnLock(); //解锁

            task->ProcessOn(); //开始处理请求

            delete task;
        }

        return nullptr;
    }

    void PushTask(Task *task)
    {
        Lock();
        task_queue.push(task);
        UnLock();

        ThreadWakeUp(); //唤醒一个线程取任务
    }

    void PopTask(Task **task)
    {
        *task = task_queue.front();
        task_queue.pop();
    }

    void ThreadWait()
    {
        pthread_cond_wait(&cond, &lock);
    }

    void ThreadWakeUp()
    {
        pthread_cond_signal(&cond);
    }

    void Lock()
    {
        pthread_mutex_lock(&lock);
    }
    void UnLock()
    {
        pthread_mutex_unlock(&lock);
    }
};

ThreadPool *ThreadPool::thread_pool = nullptr;
