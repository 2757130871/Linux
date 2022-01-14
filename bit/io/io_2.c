#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>


int main()
{
    close(1);
    
   int fd = open("txt", O_CREAT | O_RDWR, 0644); 

   
	//这连个C标准库函数默认拥有用户级的缓冲区，如果
	//向屏幕输出就是行缓冲，遇到'\n'才刷新，如果向文件
	//写入就是全缓冲，缓冲区满了才写入，
    printf("helloworld\n");
    fprintf(stdout, "helloworld: fprintf\n");
}
