#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void handler(int sig)
{
    printf("%d sig = %d\n", sig);
    
}

int main()
{
    signal(8, handler);
    while(1)
    {

        int b = 0;  
            int i  =1 / b;
    }


}


