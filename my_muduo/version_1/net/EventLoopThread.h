#include "../base/noncopyable.hpp"
#include "Thread.h"

#include <mutex>
#include <functional>
#include <condition_variable>

#include <string>

class EventLoop;

class EventLoopThread : noncopyable
{
public:
    //  线程初始化的回调操作
    using ThreadInitCallBack = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallBack &cb = ThreadInitCallBack(),
                    const std::string &name = std::string());

    ~EventLoopThread();

    EventLoop *StartLoop();

private:
    void ThreadFunc(); //线程函数 此函数中创建loop

private:
    EventLoop *loop_;
    bool exiting_;

    Thread thread_;

    std::mutex mutex_;
    std::condition_variable cond_;

    ThreadInitCallBack callback_; //调用此回调创建EventLoop
};
