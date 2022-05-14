#include "Thread.h"
#include "../base/CurrentThread.h"

#include <semaphore.h>

std::atomic_int32_t Thread::numCreated_ = {0};

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false),        /**/
      joined_(false),         /**/
      tid_(0),                /**/
      func_(std::move(func)), /**/
      name_(name)
{
    SetDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        //让系统自动回收线程资源
        thread_->detach();
    }
}

void Thread::Start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, 0, 0);

    thread_ = std::shared_ptr<std::thread>(new std::thread([&]()
                                                           {
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        //开启新线程 执行此函数
        func_(); }));

    sem_wait(&sem);
}

void Thread::Join()
{
    joined_ = true;
    thread_->join();
}

void Thread::SetDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}
