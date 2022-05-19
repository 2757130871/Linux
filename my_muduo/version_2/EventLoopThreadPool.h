#pragma once

#include <vector>

#include <functional>
#include <memory>
#include <string>

#include "EventLoopThread.h"
#include "EventLoop.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{

public:
    using ThreadInitCallBack = std::function<void(EventLoop *)>;

    EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);
    ~EventLoopThreadPool();

    //设置线程数 TcpServer SetThreadNum() -> EventLoopThreadPool SetThreadNum()
    void SetThreadNum(int numThreads) { numThreads_ = numThreads; }

    //创建 numThread_ 个 EventLoop 线程
    void Start(const ThreadInitCallBack &cb = ThreadInitCallBack());

    //如果工作在多线程中 baseLoop 默认以轮询的方式分配newFd给subLoop
    EventLoop *GetNextLoop();

    //获取所有的loop
    std::vector<EventLoop *> GetAllLoops();

    bool Started() const { return started_; }
    const std::string Name() const { return name_; }

private:
    EventLoop *baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_; //所有已创建的线程
    std::vector<EventLoop *> loops_;
};