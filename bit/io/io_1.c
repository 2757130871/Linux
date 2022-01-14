#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

//关闭标准输出流，再打开一个文件，此时默认为1的标准输出流不是指向屏幕了，而是指向这个打开的文件

int main()
{
    close(1);
    
   int fd = open("txt", O_CREAT | O_RDWR, 0644); 

   
    printf("helloworld\n");

}
