#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
 
#define PWM_SET_FREQ 1
#define PWM_STOP     0
 
 
int fd;
int main(int argc, char **argv)//dev state freq
{
    char* filename;
    long int freq;
 
    filename=argv[1];
    if (argc != 4)
        {
          printf("parm limited!\n");
          return 0;
        }
    printf("open %s \n",filename);
    fd=open(filename,O_RDWR);
    if(fd<0)
    {
        printf("can't open\n");
        return 0;
    }
    if(!strcmp("on",argv[2]))
    {
        printf("PWM ON!\n");
        freq=atol(argv[3]);
        ioctl(fd,PWM_SET_FREQ,freq);
        printf("PWM FREQ set:%d Hz \n",freq);
    }
    else if(!strcmp("off",argv[2]))
    {
        ioctl(fd,PWM_STOP);
        printf("PWM stop!\n");
    }
    else
        printf("error:param wrong!\n");
        while(1)
        {
            sleep(1000);
        }
    close(fd);
    return 0;
}