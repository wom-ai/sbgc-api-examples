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
    newtio.c_cflag &= ~PARENB;
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;

    newtio.c_iflag = IGNPAR;
 //   newtio.c_iflag = ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag &= ~ICANON; // MUST
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

//
// reference:
//   * https://stackoverflow.com/questions/63785360/c-alignas1-does-not-affect-the-size-of-a-struct
//   * https://dojang.io/mod/page/view.php?id=432
//
#pragma pack(push, 1)
struct Version_Info {
    unsigned char board_ver;
    unsigned short firmware_ver;
    unsigned char debug_mode;
    unsigned short board_features;
    unsigned char connection_flags;
    unsigned int  frw_extra_id;
    unsigned char reserved0;
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned char reserved3;
    unsigned char reserved4;
    unsigned char reserved5;
    unsigned char reserved6;
};
#pragma pack(pop)

struct Angle_Info {
    short imu_angle;
    short rc_target_angle;
    short rc_speed;
};

int main( void)
{
    init_serial(fd, oldtio, newtio);

    init_sig();

	SBGC_Demo_setup(fd);
#if 1
    printf("version info:\n");
    {
        SerialCommand cmd;
        cmd.init(SBGC_CMD_BOARD_INFO);
        sbgc_parser.send_cmd(cmd, 0);

        sleep(1); // MUST

        int size=0;
        char buf[BUF_MAX];
        printf("[header]---------------------\n");
        size = read(fd,buf,4);
        if (size != 4) {
            fprintf(stderr, "failed to read header \n");
            exit(-1);
        }

        for (int i = 0;i < 4; i++) {
            printf("0x%x\t(%u)\n", (unsigned char)buf[i], (unsigned char)buf[i]);
        }

        printf("[body]---------------------\n");
#if 1
        struct Version_Info version = {0, };
        size = read(fd, (unsigned char*)&version, sizeof(Version_Info));

        if (size != sizeof(Version_Info))
        {
            fprintf(stderr, "size = %d, sizeof(Version_Info) = %d\n", size, sizeof(Version_Info));
            fprintf(stderr, "failed to read body \n");
            exit(-1);
        }

        unsigned char *cur = (unsigned char*)(&version);
        for (int i = 0; i < sizeof(Version_Info); i++)
        {
            printf("[%d] %u\n", i, cur[i]);
        }

        printf("board_ver: %u(0x%x)\n", version.board_ver, version.board_ver);
        printf("firmware_ver: %u(0x%x)\n", version.firmware_ver, version.firmware_ver);

        // checksum
        size = read(fd,buf,BUF_MAX);
        buf[size] = '\0';

        for (int i = 0;i < size; i++) {
            printf("0x%x\t(%d)\n", buf[i], buf[i]);
        }

#else

        size = read(fd,buf,BUF_MAX);
        buf[size] = '\0';

        for (int i = 0;i < size; i++) {
            printf("0x%x\t(%d)\n", buf[i], buf[i]);
        }
#endif
    }
#endif


    printf("angle info:\n");

    //for (int k = 0; k < 100; k++)
    {
        SerialCommand cmd;
        cmd.init(SBGC_CMD_GET_ANGLES);
        sbgc_parser.send_cmd(cmd, 0);

        sleep(1); // MUST

        int size=0;
        char buf[BUF_MAX];
        printf("[header]---------------------\n");
        size = read(fd,buf,4);
        if (size != 4) {
            fprintf(stderr, "failed to read header \n");
        }

        for (int i = 0;i < 4; i++) {
            printf("0x%x\t(%u)\n", (unsigned char)buf[i], (unsigned char)buf[i]);
        }

        printf("[body]---------------------\n");

        struct Angle_Info angles[3] = {{0, }, {0, }, {0, }};
        size = read(fd, (unsigned char*)angles, sizeof(Angle_Info)*3);
        buf[size] = '\0';

        for (int i = 0;i < 3; i++) {
            printf("[%03d] imu_angle:       %03.03f\t(0x%x)\n", i, SBGC_ANGLE_TO_DEGREE(angles[i].imu_angle), angles[i].imu_angle);
            printf("[%03d] rc_target_angle: %03.03f\t(0x%x)\n", i, SBGC_ANGLE_TO_DEGREE(angles[i].rc_target_angle), angles[i].rc_target_angle);
            printf("[%03d] rc_speed:        %d\t(0x%x)\n", i, angles[i].rc_speed, angles[i].rc_speed);
        }

        // checksum
        size = read(fd,buf,BUF_MAX);
        buf[size] = '\0';

        for (int i = 0;i < size; i++) {
            printf("0x%x\t(%d)\n", buf[i], buf[i]);
        }
    }

    printf("moving...\n");

    printf("Set RPY=(0, 0, 0)...\n");

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
