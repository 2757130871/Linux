
#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg)
    : baseLoop_(baseLoop), /**/
      name_(nameArg),      /**/
      started_(false),     /**/
      numThreads_(0),      /**/
      next_(0)             /**/
{
}

/* EventLoop是在stack上创建的 无需手动delete */
EventLoopThreadPool::~EventLoopThreadPool() {}

//创建numThread_个EventLoop线程
void EventLoopThreadPool::Start(const ThreadInitCallBack &cb)
{
    started_ = true;
    for (int i = 0; i < numThreads_; i++)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);

        EventLoopThread *t = new EventLoopThread(cb, buf);

        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        //底层创建线程 绑定一个EventLoop 返回EventLoop地址
        loops_.push_back(t->StartLoop());
    }

    //用户没有调用SetThreadNum设置线程数
    //所以只有一个baseLoop处理新连接和已有连接上的I/O
    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

//如果工作在多线程中 baseLoop 默认以轮询的方式分配newFd给subLoop
EventLoop *EventLoopThreadPool::GetNextLoop()
{
    EventLoop *loop = baseLoop_;

    // Round Robin
    if (!loops_.empty())
    {
        loop = loops_[next_++];
        if (static_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }

    return loop;
}

//获取所有的loop
std::vector<EventLoop *> EventLoopThreadPool::GetAllLoops()
{
    if (loops_.empty())
    {
        return std::vector<EventLoop *>(1, baseLoop_);
    }

    return loops_;
}
