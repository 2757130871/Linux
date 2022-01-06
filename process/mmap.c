#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>



int main()
{
    int fd = open("txt",O_RDWR); 
    if(-1 == fd){
      printf("open error\n");
      return -1;
    }
    
    char buf[256] = "chenxinrong";


    void* addr =mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  
    if(MAP_FAILED == addr){
      printf("mmap error\n");
      return -1;
    }
    close(fd);
    
    memmove(addr, buf, strlen(buf) + 1);
    
    munmap(addr, 1024);

}
