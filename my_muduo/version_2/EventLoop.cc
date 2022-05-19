#include <sys/eventfd.h>
#include <unistd.h>

#include "Util.h"
#include "EventLoop.h"

#include "./Channel.h"
#include "./Poller.h"

//__thread 只有在GNU编译有效，不支持跨平台
//此关键字修饰的变量每个线程都拥有一份，
__thread EventLoop *t_loopInThisThread = nullptr;

//定义默认的Poller超时时间
const int kPollTimeMs = 10000;

// craete eventfd notify sub reactor handler
int CreateEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    LOG(INFO) << "evtfd: " << evtfd << std::endl;

    if (evtfd < 0)
    {
        LOG(FATAL) << "eventfd error!" << std::endl;
        exit(1);
    }

    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),                             /* EventLoop是否结束 */
      quit_(false),                                /* EventLoop是否退出 */
      callingPendingFuctors_(false),               /* 是否有存在的cb未执行 (是否正在执行cb操作)  */
      threadId_(CurrentThread::tid()),             /**/
      poller_(Poller::NewDefaultPoller(this)),     /**/
      wakeupFd_(CreateEventfd()),                  /**/
      wakeupChannel_(new Channel(this, wakeupFd_)) /**/
                                                   /*  currentActiveChannel_(nullptr)  */
{

    LOG(INFO) << "EventLoop craeted in this thread: " << threadId_ << std::endl;

    //如果此线程已经创建了EventLoop,
    if (t_loopInThisThread)
    {
        LOG(FATAL) << "Another EventLoop existed !!" << std::endl;
        exit(0);
    }
    else
    {
        t_loopInThisThread = this;
    }

    //设置wakeupfd的事件类型 and 发生事件之后的回调操作
    wakeupChannel_->SetReadCallBack(std::bind(&EventLoop::HandleRead, this));
    //每一个sub EventLoop都监听wakeupchannel的EPOLLIN事件
    wakeupChannel_->EnableReading();
}

EventLoop::~EventLoop()
{
    //取消所有的感兴趣事件
    wakeupChannel_->DisableAll();
    wakeupChannel_->Remove();

    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::HandleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG(FATAL) << " EventLoop::HandleRead() reads " << n << " bytes instead of 8 bits" << std::endl;
        exit(-1);
    }
}

void EventLoop::Loop()
{
    looping_ = true;
    quit_ = false;

    LOG(INFO) << "EventLoop::Loop this: " << this << std::endl;

    while (!quit_)
    {
        activeChannels_.clear();
        //监听客户端的fd 和 wakeup的fd
        pollReturnTime_ = poller_->Poll(kPollTimeMs, &activeChannels_);

        for (Channel *channel : activeChannels_)
        {
            channel->HandleEvent(pollReturnTime_);
        }

        // 执行当前EventLoop事件循环需要处理的回调操作
        //
        // IO线程 mainloop accept fd 打包成Channel 传给 subloop
        // mainloop事先注册一个回调cb （需要subloop执行）
        // wakeup subloop后，执行以下方法，执行之前mainloop注册的cb操作
        DoPendingFunctors();
    }

    LOG(INFO) << "EventLoop stop looping" << std::endl;
    looping_ = false;
}

// one loop per thread
// 两种情况
// 1 thread在自己的loop中自己调用quit
// 2 thread调用了其他loop的quit
void EventLoop::Quit()
{
    quit_ = true;
    if (!IsInLoopThread())
    {
        //如果是其他线程调用quit
        Wakeup();
    }
}

//在当前loop中执行CallBack
void EventLoop::RunInLoop(Functor cb)
{
    if (IsInLoopThread()) //在当前loop线程中执行cb
        cb();
    else //不在当前loop线程中执行cb 需要唤醒loop所在线程执行cb
        QueueInLoop(cb);
}

//把cb放入queue中，唤醒loop所在线程执行cb
void EventLoop::QueueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    //不在自己所属的loop || 此loop正在处理cb,又收到了新的回调
    if (!IsInLoopThread() || callingPendingFuctors_)
    {
        Wakeup(); //唤醒loop所在的线程
    }
}

// MainReacotr wakeup SubReactor
//唤醒loop所在的thread
void EventLoop::Wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG(FATAL) << " EventLoop::HandleRead() writes " << n << " bytes instead of 8 bits" << std::endl;
        exit(-1);
    }
}

//执行回调
void EventLoop::DoPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFuctors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor &func : functors)
        func(); //执行当前loop需要执行的回调操作

    callingPendingFuctors_ = false;
}

// Channel通过EventLoop操作Poller
void EventLoop::UpdateChannel(Channel *channel)
{
    poller_->UpdateChannel(channel);
}
void EventLoop::RemoveChannel(Channel *channel)
{
    poller_->RemoveChannel(channel);
}
bool EventLoop::HasChannel(Channel *channel)
{
    return poller_->HasChannel(channel);
}
