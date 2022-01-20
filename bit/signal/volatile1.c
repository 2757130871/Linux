#include <stdio.h>
#include <unistd.h>
#include <signal.h>

//volatile¹Ø¼ü×Ö±£´æ±äÁ¿µÄ¿É¼ûÐÔ£¬ÓÀÔ¶¶ÁÈ¡µÄÊÇÄÚ´æÖÐ×îÐÂÖµ¡£
volatile int flag = 0;

void handler(int sig)
{
    printf("set flag = 1 \n");
    printf("receive sig = %d \n",sig);
    flag = 1;
}



int main()
{
    signal(2, handler);
    while(!flag)
    {
    }
    
}
