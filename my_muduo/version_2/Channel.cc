#include "Channel.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{
}

//析构操作必须是EventLoop所属的线程负责析构此Channel
Channel::~Channel() {}

// fd通过poller通知的事件，调用相对应的回调
void Channel::HandleEvent(TimeStamp recviveTime)
{
    if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard)
        {
            HandleEventWithGuard(recviveTime);
        }
    }
    else
    {
        HandleEventWithGuard(recviveTime);
    }
}

//设置回调对象
void Channel::SetReadCallBack(ReadEventCallBack cb) { readCallBack_ = std::move(cb); }
void Channel::SetWriteCallBack(EventCallBack cb) { writeCallBack_ = std::move(cb); }
void Channel::SetCloseCallBack(EventCallBack cb) { closeCallBack_ = std::move(cb); }
void Channel::SetErrorCallBack(EventCallBack cb) { errorCallBack_ = std::move(cb); }

//防止cannel已经被手动remove还在执行回调
void Channel::Tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

int Channel::Get_Fd() const { return fd_; }
int Channel::GetEvents() const { return events_; }
void Channel::SetREvents(int revents) { revents_ = revents; }
bool Channel::IsNonEvent() const { return events_ == kNoneEvent; }

void Channel::EnableReading()
{
    events_ |= kReadEvent;
    Update();
}

void Channel::DisableReading()
{
    events_ &= ~kReadEvent;
    Update();
}
void Channel::EnableWriting()
{
    events_ |= kWriteEvent;
    Update();
}
void Channel::DisableWrirting()
{
    events_ &= ~kWriteEvent;
    Update();
}
void Channel::DisableAll()
{
    events_ = kNoneEvent;
    Update();
}

bool Channel::IsNoneEvent() { return events_ == kNoneEvent; }
bool Channel::IsReading() { return events_ & kReadEvent; }
bool Channel::IsWriting() { return events_ & kWriteEvent; }

int Channel::GetIndex() { return index_; }
void Channel::SetIndex(int idx) { index_ = idx; }

EventLoop *Channel::OwnerLoop() { return loop_; }

void Channel::Remove()
{
    loop_->RemoveChannel(this);
}
void Channel::Update()
{
    // add ....
    loop_->UpdateChannel(this);
}

void Channel::HandleEventWithGuard(TimeStamp recviveTime)
{
    //未知错误
    if ((events_ & EPOLLHUP) && !(events_ & EPOLLIN))
        if (closeCallBack_)
            closeCallBack_();

    //读取事件
    if ((events_ & (EPOLLIN | EPOLLPRI)))
        if (readCallBack_)
            readCallBack_(recviveTime);

    //可写事件
    if (events_ & EPOLLOUT)
        if (writeCallBack_)
            writeCallBack_();

    //错误事件
    if (events_ & EPOLLERR)
        if (errorCallBack_)
            errorCallBack_();
}