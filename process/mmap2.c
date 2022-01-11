#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    
    char buf[256] = "AKLDJSADSS";
    void* addr =mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);  
    if(MAP_FAILED == addr){
      printf("mmap error\n");
      return -1;
    }
    


    pid_t ret = fork();
    if(0 == ret){
        memmove(addr, buf, strlen(buf) + 1);
        return 0;
    }
    int stat = -1;
    wait(&stat);
    
    memset(buf, 0, 256);
    memmove(buf, addr, strlen(addr) + 1);
    printf("%s\n",buf);

    munmap(addr, 1024);

}
