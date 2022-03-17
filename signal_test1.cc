#include <iostream>
#include <stdio.h>

#include <signal.h>
#include <sys/types.h>
#include <cstdlib>
#include <unistd.h>

int main()
{   
    sigset_t in, out;
    sigemptyset(&in);
    sigaddset(&in, 2);
    
    while(1)
    {
        
    sigprocmask(SIG_SETMASK, &in, &out);
        sigset_t pending;
        sigpending(&pending);
            
        for(int i = 1;i <= 31;i++)
        {
            printf("%d", sigismember(&pending, i));
        }
        std::cout << std::endl;
        sleep(1);
    }
    
    
}
