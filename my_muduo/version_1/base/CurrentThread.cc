#include "CurrentThread.h"

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{

    void cacheTid()
    {
        if (t_cacheTid == 0)
        {
            //通过syscall获取当前线程tid
            t_cacheTid = static_cast<pid_t>(syscall(SYS_getpid));
        }
    }

}
