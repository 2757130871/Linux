#pragma once

namespace CurrentThread
{
    extern __thread int t_cacheTid;
    
    void cacheTid();

    inline int tid()
    {
        if (__builtin_expect(t_cacheTid == 0, 0))
        {
            //未获取当前线程id
            cacheTid();
        }
        return t_cacheTid;
    }

};
