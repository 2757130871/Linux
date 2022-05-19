#include "EventLoopThread.h"

#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallBack &cb, const std::string &name)
    : loop_(nullptr),                                               /**/
      exiting_(false),                                              /**/
      thread_(std::bind(&EventLoopThread::ThreadFunc, this), name), /**/
      mutex_(),                                                     /**/
      cond_(),                                                      /**/
      callback_(cb)                                                 /**/
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->Quit(); //线程退出时 关闭事件循环
        thread_.Join();
    }
}

EventLoop *EventLoopThread::StartLoop()
{
    thread_.Start(); // Start 内部才创建新线程

    EventLoop *loop = nullptr;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop == nullptr)
            cond_.wait(lock);

        loop_ = loop;
    }

    return loop_;
}

//线程函数 此函数在创建的新线程中执行 此函数中创建loop并开启事件循环
void EventLoopThread::ThreadFunc()
{
    EventLoop loop; //!! EventLoop在此处创建

    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;

        cond_.notify_one(); //唤醒StartLoop
    }

    loop.Loop(); //开启EventLoop 开始监听就绪事件

    //此loop quit
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}