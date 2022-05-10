#pragma once

class noncopyable
{
protected:
    noncopyable() = default;
    ~noncopyable() = default;

public:
    noncopyable(const noncopyable &obj) = delete;
    void operator=(const noncopyable &obj) = delete;
};
