#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <termios.h>
#include <fcntl.h>

#include <SBGC.h>
#include <SBGC_Linux.h>

#define BUF_MAX 512

//#define DEVICE_NAME "/dev/ttyACM0"
#define DEVICE_NAME "/dev/ttyUSB0"

int fd;
struct termios oldtio, newtio;

void init_serial(int &fd, struct termios &oldtio, struct termios &newtio)
{
    printf("trying to open " DEVICE_NAME "...\n");
    fd = open( DEVICE_NAME, O_RDWR | O_NOCTTY );
    if (fd<0) {
        fprintf(stderr, "failed to open " DEVICE_NAME "\n");
        exit(-1);
    }

    if (tcgetattr(fd,&oldtio)) {
        fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }

    printf("open " DEVICE_NAME " by FD(%d)\n", fd);
    memset(&newtio, 0, sizeof(newtio) );

    newtio.c_cflag = B115200; // BAUD RATE
    //newtio.c_cflag = B230400;
    newtio.c_cflag |= CS8;
    newtio.c_cflag |= CLOCAL;
    newtio.c_cflag |= CREAD;
    newtio.c_iflag = IGNPAR;
 //   newtio.c_iflag = ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;

    if (tcflush(fd, TCIFLUSH)) {
        fprintf(stderr, "Error %i from tcflush: %s\n", errno, strerror(errno));
    }
    if (tcsetattr(fd, TCSANOW, &newtio)) {
        fprintf(stderr, "Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }

    sleep(3);
}

void deinit_serial(int &fd, struct termios &oldtio)
{
    if (tcsetattr(fd,TCSANOW,&oldtio)) {
        fprintf(stderr, "Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }
    close( fd);
}

//
// reference: https://stackoverflow.com/questions/6990093/how-to-convert-signal-name-string-to-signal-code
//
void signal_handler(int signalno)
{
    fprintf(stderr, "[SIG] %s(%d)\n", strsignal(signalno), signalno);

    deinit_serial(fd, oldtio);
    exit(1);
}

void init_sig(void)
{
    struct sigaction saio;
    saio.sa_handler = signal_handler;
    sigemptyset(&saio.sa_mask);
    saio.sa_flags = 0;
    saio.sa_restorer = NULL;
    sigaction(SIGINT, &saio,NULL);
}

int main( void)
{
    init_serial(fd, oldtio, newtio);

    init_sig();

    printf("moving...\n");

    printf("Set RPY=(0, 0, 0)...\n");
	SBGC_Demo_setup(fd);

#if 1
    static SBGC_cmd_control_ext_t cmd_control = { 
        { SBGC_CONTROL_MODE_NO, SBGC_CONTROL_MODE_ANGLE, SBGC_CONTROL_MODE_ANGLE },  // control mode for ROLL, PITCH, YAW
        { { 0, 0 }, { 0, 0 }, { 0, 0 } }  // angle and speed
    }; 
    SBGC_cmd_control_ext_send(cmd_control, sbgc_parser);
	sleep(4);

    printf("Set RPY=(0, 10, 0)...\n");
    cmd_control.data[PITCH].angle = SBGC_DEGREE_TO_ANGLE(20); 
    SBGC_cmd_control_ext_send(cmd_control, sbgc_parser);
	sleep(4);


    printf("Set RPY=(0, -10, 0)...\n");
    cmd_control.data[PITCH].angle = SBGC_DEGREE_TO_ANGLE(-20); 
    SBGC_cmd_control_ext_send(cmd_control, sbgc_parser);
	sleep(4);

    printf("Set RPY=(0, 0, 0)...\n");
    cmd_control.data[PITCH].angle = SBGC_DEGREE_TO_ANGLE(0); 
    SBGC_cmd_control_ext_send(cmd_control, sbgc_parser);
	sleep(4);

    printf("Set RPY=(0, 0, 10)...\n");
    cmd_control.data[YAW].angle = SBGC_DEGREE_TO_ANGLE(20);
    SBGC_cmd_control_ext_send(cmd_control, sbgc_parser);
	sleep(4);


    printf("Set RPY=(0, 0, -10)...\n");
    cmd_control.data[YAW].angle = SBGC_DEGREE_TO_ANGLE(-20);
    SBGC_cmd_control_ext_send(cmd_control, sbgc_parser);
	sleep(4);

    printf("Set RPY=(0, 0, 0)...\n");
    cmd_control.data[YAW].angle = SBGC_DEGREE_TO_ANGLE(0);
    SBGC_cmd_control_ext_send(cmd_control, sbgc_parser);
	sleep(4);

    printf("Turn off Control...\n");
    cmd_control = { 
        { SBGC_CONTROL_MODE_NO, SBGC_CONTROL_MODE_NO, SBGC_CONTROL_MODE_NO},  // control mode for ROLL, PITCH, YAW
        { { 0, 0 }, { 0, 0 }, { 0, 0 } }  // angle and speed
    }; 
    SBGC_cmd_control_ext_send(cmd_control, sbgc_parser);
	sleep(4);
#else
	SBGC_cmd_control_t c = { 0, 0, 0, 0, 0, 0, 0 };
	c.mode = SBGC_CONTROL_MODE_ANGLE;
	c.anglePITCH = SBGC_DEGREE_TO_ANGLE(20);
    //c.angleYAW = SBGC_DEGREE_TO_ANGLE(20);
	if (SBGC_cmd_control_send(c, sbgc_parser))
    {
        fprintf(stderr, "Something wrong\n");
        exit(-1);
    }
	sleep(4);

	c.mode = SBGC_CONTROL_MODE_ANGLE;
	c.anglePITCH = SBGC_DEGREE_TO_ANGLE(-20);
    //c.angleYAW = SBGC_DEGREE_TO_ANGLE(10);
	if (SBGC_cmd_control_send(c, sbgc_parser))
    {
        fprintf(stderr, "Something wrong\n");
        exit(-1);
    }
	sleep(4);
	c.anglePITCH = SBGC_DEGREE_TO_ANGLE(-10);
	c.angleYAW = SBGC_DEGREE_TO_ANGLE(-10);
	SBGC_cmd_control_send(c, sbgc_parser);
	sleep(4);

	c.anglePITCH = 0;
	c.angleYAW = 0;
	SBGC_cmd_control_send(c, sbgc_parser);
	sleep(4);

#endif


    deinit_serial(fd, oldtio);
    return 0;
}
