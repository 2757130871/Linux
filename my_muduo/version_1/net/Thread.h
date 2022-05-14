#pragma once

#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <string>

#include <unistd.h>

#include "../base/noncopyable.hpp"

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func,
                    const std::string &name = std::string());
    ~Thread();

    void Start();
    void Join();

    bool Started() const { return started_; }
    pid_t Tid() const { return tid_; }
    const std::string &Name() { return name_; }

    //返回已创建的线程数
    static int NumCreated() { return numCreated_; }

private:
    void SetDefaultName();

private:
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_; // c++11的thread对象一创建就执行 使用智能指针控制启动时间
    pid_t tid_;
    ThreadFunc func_;

    std::string name_;
    static std::atomic_int32_t numCreated_; //已创建的线程个数
};
