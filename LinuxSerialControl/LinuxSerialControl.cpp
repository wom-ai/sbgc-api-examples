#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
 
#include <SBGC.h>

#define BUF_MAX 512

#define DEVICE_NAME "/dev/ttyACM0"
 
int main( void)
{
    int fd,i=0;
 
    char buf[BUF_MAX];
    char tmp, read_byte=0;
 
    struct termios newtio;
 
    printf("trying to open " DEVICE_NAME "...\n");
//    fd = open( "/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK );
    fd = open( DEVICE_NAME, O_RDWR | O_NOCTTY );
        if(fd<0) { fprintf(stderr,"ERR\n"); exit(-1); }
 
    printf("open "DEVICE_NAME "\n");
    memset( &newtio, 0, sizeof(newtio) );
 
 
    newtio.c_cflag = B115200;
    //newtio.c_cflag = B230400;
    newtio.c_cflag |= CS8;
    newtio.c_cflag |= CLOCAL;
    newtio.c_cflag |= CREAD;
    newtio.c_iflag = IGNPAR;
 //   newtio.c_iflag = ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
 
    tcflush (fd, TCIFLUSH );
    tcsetattr(fd, TCSANOW, &newtio );
 
    sleep(3);
    
    printf("listening...\n");
 
    while(1) {
 
        write( fd, "R", 1);
        i = read(fd,buf,BUF_MAX);
        buf[i] = '\0';
        printf("%s",buf);
        sleep(1);
    }

    close( fd);
    return 0;
}
