#pragma once

#include <vector>
#include "my_sem.hpp"

template <class T>
class block_queue
{

public:
    block_queue(int capacity = 10, int limit = 100) : _cap(capacity), blank(capacity), data(0)
    {
        v.resize(capacity);
    }

    void Put(const T &x)
    {
        blank.P();
        v[index_p++] = x;
        index_p %= _cap;
        data.V();
    }

    const T &Get()
    {
        data.P();
        const T &ref = v[index_c++];
        index_c %= _cap;
        blank.V();

        return ref;
    }

private:
    std::vector<T> v;
    int _cap;
    int _limit; //最大容量上限
    int index_p = 0;
    int index_c = 0;
    cxr::sem data;
    cxr::sem blank;
};
