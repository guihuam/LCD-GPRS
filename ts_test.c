#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "lcd.h"
#include "ascii.lib"
#include "tslib.h"
#include "fbutils.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>                                                 
#include <sys/stat.h>                                                      
#include <errno.h>    

#define BUTTON_ACTIVE 0x00000001
#define NR_COLORS (sizeof (palette) / sizeof (palette [0]))
#define NR_BUTTONS 13
#define COM "/dev/ttyS0"
#define CMD_TPYE	(0)
#define PHONE_NUMBER	(1)
#define MAX_CMD_DEPTH  	(2)
#define ENDMINITERM 27 
#define FALSE		0
#define TRUE  		1
volatile int STOP=FALSE;
int GET_GPRS_OK=FALSE;
volatile int baud=B9600;
extern volatile int STOP ;
static int fd;
static struct termios oldtio,newtio;
char * cmd[20]={
		"AT",
		"ATE1",
		"AT+CHFA=1", 	//设置通话通道为1,AT+CHFA 命令切换主副音频通道
		"AT+CLVL=100",	//设置受话器音量最大, AT+CLVL 命令可以调节输出音频信号增益
		"AT+CMIC=1,10"	//设置通道1的话筒增益
};

int tty_init();
int tty_end();
int tty_read(char *buf,int nbytes);
int tty_write(char *buf,int nbytes);
int tty_writecmd(char *buf,int nbytes);
void tty_flush();
void gprs_init();
void gprs_msg(char *number, int num);
void gprs_call(char *number, int num);
void gprs_hold();
void gprs_ans();
void gprs_call(char *number, int num);
void gprs_baud(char *baud,int num);

int tty_end()
{
  	tcsetattr(fd,TCSANOW,&oldtio);	
  	close(fd);
}

int tty_read(char *buf,int nbytes)
{
	return read(fd,buf,nbytes);
}

void tty_flush()
{
	tcflush(fd, TCIFLUSH);
}

int tty_write(char *buf,int nbytes)
{
	int i;
	for(i=0; i<nbytes; i++) {
		write(fd,&buf[i],1);
		usleep(100);
	}
	return tcdrain(fd);
}

int tty_writecmd(char *buf,int nbytes)
{
	int i;
	for(i=0; i<nbytes; i++) {
		write(fd,&buf[i],1);
		usleep(100);
	}
	write(fd,"\r",1);
	usleep(300000);
	return tcdrain(fd);
}

int tty_init()
{
	fd = open(COM, O_RDWR ); //| O_NONBLOCK);//
	if (fd <0) {
	    	perror(COM);
	    	exit(-1);
  	}
  	tcgetattr(fd,&oldtio); /* save current modem settings */
	bzero(&newtio, sizeof(newtio)); 
	newtio.c_cflag = baud | /*CRTSCTS |*/ CS8 /*| CLOCAL | CREAD */;
	newtio.c_iflag = IGNPAR | ICRNL; 
	newtio.c_oflag = 0; 
	newtio.c_lflag = ICANON;
	 newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */  
	 newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */ 
	 newtio.c_cc[VERASE]   = 0;     /* del */ 
	 newtio.c_cc[VKILL]    = 0;     /* @ */ 
	 newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */ 
	 newtio.c_cc[VTIME]    = 0;     /* 不使用分割字元组的计时器 */ 
	 newtio.c_cc[VMIN]     = 1;     /* 在读取到 1 个字元前先停止 */ 
	 newtio.c_cc[VSWTC]    = 0;     /* '\0' */ 
	 newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */  
	 newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */ 
	 newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */ 
	 newtio.c_cc[VEOL]     = 0;     /* '\0' */ 
	 newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */ 
	 newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */ 
	 newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */ 
	 newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */ 
	 newtio.c_cc[VEOL2]    = 0;     /* '\0' */ 
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
	return 0;
}

void gprs_init()
{ 
	int i;
	for(i=0; i<5; i++) {
		tty_writecmd(cmd[i], strlen(cmd[i])); 
	}
}

void gprs_hold()
{
	tty_writecmd("AT", strlen("AT"));
	tty_writecmd("ATH", strlen("ATH"));
}

void gprs_ans()
{ 
	tty_writecmd("AT", strlen("AT"));
	tty_writecmd("ATA", strlen("ATA"));
}

void gprs_call(char *number, int num)			
{ 

	tty_write("ATD", strlen("ATD")); 	
	tty_write(number, num);
	tty_write(";\r", strlen(";\r"));
	usleep(200000);
}

void gprs_msg(char *number, int num)		
{               
	char ctl[]={26,0};
	char text[]="Hello, I am miguhua and sending mesg by UP-TECH";                     
	tty_writecmd("AT", strlen("AT"));
	tty_writecmd("ATE0", strlen("ATE0"));
	tty_writecmd("AT+CMGF=1", strlen("AT+CMGF=1"));	
	tty_write("AT+CMGS=", strlen("AT+CMGS="));
	tty_write("\"", strlen("\""));
	tty_write(number, strlen(number));
  	tty_write("\"", strlen("\""));
  	tty_write("\r", strlen("\r"));
	usleep(10000);
        tty_write(text, strlen(text));
	tty_write(ctl, 1);  
	usleep(300000);
}

void gprs_baud(char *baud,int num)
{
	tty_write("AT+IPR=", strlen("AT+IPR="));
	tty_writecmd(baud, strlen(baud) );
	usleep(200000);
}

void* gprs_read()//void * data
{
	//int i=0;
	char c;
	//char buf[1024];
  	printf("\nread modem\n");
	
  	while (STOP==FALSE) 	{		

	    	tty_read(&c,1); 	
		printf("%c",c);	
	}
  	printf("exit from reading modem\n");
  	return NULL; 
}

int get_baudrate(char** argv)//int argc,
{	
	int v=atoi(argv[1]);
	switch(v){		
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
			return B57600;
		case 115200:	
			return B115200;
		default:
			return -1;
		} 
}

static int palette [] ={0x000000, 0xffe080, 0xffffff, 0xe0c0a0, 0x304050, 0x80b8c0};
struct ts_button {
	int x, y, w, h;
	char *text;
	int flags;
	};

static int button_palette [6] = {1, 4, 2,1, 5, 0};
static struct ts_button buttons [NR_BUTTONS];
static void sig(int sig)
{
	close_framebuffer();
	fflush(stderr);
	printf("signal %d caught\n", sig);
	fflush(stdout);
	exit(1);
}
static void button_draw (struct ts_button *button)
{
	int s = (button->flags & BUTTON_ACTIVE) ? 3 : 0;
	rect (button->x, button->y, button->x + button->w - 1,
	      button->y + button->h - 1, button_palette [s]);
	fillrect (button->x + 1, button->y + 1,
		  button->x + button->w - 2,
		  button->y + button->h - 2, button_palette [s + 1]);
	put_string_center (button->x + button->w / 2,
			   button->y + button->h / 2,
			   button->text, button_palette [s + 2]);
}
static int button_handle (struct ts_button *button, struct ts_sample *samp)
{
	int inside = (samp->x >= button->x) && (samp->y >= button->y) &&
		(samp->x < button->x + button->w) &&
		(samp->y < button->y + button->h);
	if (samp->pressure > 0) {
		if (inside) {
			if (!(button->flags & BUTTON_ACTIVE)) {
				button->flags |= BUTTON_ACTIVE;
				button_draw (button);
			}
		} else if (button->flags & BUTTON_ACTIVE) {
			button->flags &= ~BUTTON_ACTIVE;
			button_draw (button);
		}
	} else if (button->flags & BUTTON_ACTIVE) {
		button->flags &= ~BUTTON_ACTIVE;
		button_draw (button);
                return 1;
	}

        return 0;
}
static void refresh_screen0 ()
{
        int i;

        fillrect (0, 0, xres - 1, yres - 1, 0);
        for (i = 0; i < NR_BUTTONS; i++)
                button_draw (&buttons [i]);
}

int main()
{
	struct tsdev *ts;
	int x, y;
	unsigned int i;
	unsigned int mode = 0;
	int j=0;
	int m = 0;
	char *tsdevice=NULL;
	char cmd[256]={0,};
	signal(SIGSEGV, sig);
	signal(SIGINT, sig);
	signal(SIGTERM, sig);
	tty_init();
	tsdevice = strdup ("/dev/event1");
	ts = ts_open (tsdevice, 0);
	if (!ts) {
		perror (tsdevice);
		exit(1);
	}
	if (ts_config(ts)) {
		perror("ts_config");
		exit(1);
	}
	if (open_framebuffer()) {
		close_framebuffer();
		exit(1);
	}
	x = xres/2;
	y = yres/2;
	for (i = 0; i < NR_COLORS; i++)
		setcolor (i, palette [i]);

	//dingyi shouji tuxing
	memset (&buttons, 0, sizeof (buttons));
	buttons [0].w = xres / 2;
	buttons [0].h = 80;
	buttons [0].x = xres / 4;
	buttons [0].y = 50;
	buttons [0].text = "";
	
	buttons [1].w = xres / 6;
	buttons [1].h = buttons [0].h;
	buttons [1].x = buttons [0].x;
	buttons [1].y = buttons [0].y + buttons [0].h;
	buttons [1].text = "1";

	buttons [2].w = xres / 6;
	buttons [2].h = buttons [1].h;
	buttons [2].x = buttons [0].x + buttons [1].w ;
	buttons [2].y = buttons [0].y + buttons [0].h;
	buttons [2].text = "2";


	buttons [3].w = xres / 6;
	buttons [3].h = buttons [1].h;
	buttons [3].x = buttons [0].x + buttons [1].w + buttons [2].w;
	buttons [3].y = buttons [0].y + buttons [0].h;
	buttons [3].text = "3";


	buttons [4].w = xres / 6;
	buttons [4].h = buttons [1].h;
	buttons [4].x = buttons [0].x;
	buttons [4].y = buttons [0].y + buttons [0].h + buttons [1].h;
	buttons [4].text = "4";

	buttons [5].w = xres / 6;
	buttons [5].h = buttons [1].h;
	buttons [5].x = buttons [0].x + buttons [4].w;
	buttons [5].y = buttons [0].y + buttons [0].h + buttons [2].h;
	buttons [5].text = "5";

	buttons [6].w = xres / 6;
	buttons [6].h = buttons [1].h;
	buttons [6].x = buttons [0].x + buttons [4].w + buttons [5].w;
	buttons [6].y = buttons [0].y + buttons [0].h + buttons [3].h;
	buttons [6].text = "6";

	buttons [7].w = xres / 6;
	buttons [7].h = buttons [1].h;
	buttons [7].x = buttons [0].x;
	buttons [7].y = buttons [0].y + buttons [0].h + buttons [1].h +buttons [4].h;
	buttons [7].text = "7";

	buttons [8].w = xres / 6;
	buttons [8].h = buttons [1].h;
	buttons [8].x = buttons [0].x + buttons [7].w;
	buttons [8].y = buttons [0].y + buttons [0].h + buttons [2].h + buttons [5].h;
	buttons [8].text = "8";

	buttons [9].w = xres / 6;
	buttons [9].h = buttons [1].h;
	buttons [9].x = buttons [0].x + buttons [7].w + buttons [8].w;
	buttons [9].y = buttons [0].y + buttons [0].h + buttons [3].h + buttons [6].h;
	buttons [9].text = "9";

	buttons [10].w = xres / 6;
	buttons [10].h = buttons [1].h;
	buttons [10].x = buttons [0].x;
	buttons [10].y = buttons [0].y + buttons [0].h + buttons [1].h + buttons [4].h + buttons [7].h;
	buttons [10].text = "mes";

	buttons [11].w = xres / 6;
	buttons [11].h = buttons [1].h;
	buttons [11].x = buttons [0].x + buttons [10].w;
	buttons [11].y = buttons [0].y + buttons [0].h+ buttons [2].h + buttons [5].h + buttons [8].h;
	buttons [11].text = "0";

	buttons [12].w = xres / 6;
	buttons [12].h = buttons [1].h;
	buttons [12].x = buttons [0].x + buttons [10].w + buttons [11].w;
	buttons [12].y = buttons [0].y + buttons [0].h + buttons [3].h + buttons [6].h + buttons [9].h;
	buttons [12].text = "call";
	refresh_screen0 ();
	while (1) {
		struct ts_sample samp;
		int ret;
		/* Show the cross */
		if ((mode & 15) != 1)
			put_cross(x, y, 2 | XORMODE);

		ret = ts_read(ts, &samp, 1);
		/* Hide it */
		if ((mode & 15) != 1)
			put_cross(x, y, 2 | XORMODE);
		if (ret < 0) {
			perror("ts_read");
			close_framebuffer();
			exit(1);
		}
		if (ret != 1)
			continue;
		mode = -1;
		for (i = 0; i < NR_BUTTONS; i++)				
		{	if (button_handle (&buttons [i], &samp))
			{
				switch (i) {
				case 0:
					mode = 0;
					j = -1;
					break;
				case 1:
					mode = 1;
					cmd[m] = '1';
					put_string_center (buttons[0].x+25*j,60,"1", 1);
					break;
				case 2:
                                        mode = 2;
					cmd[m] = '2';
                                        put_string_center (buttons[0].x+25*j,60,"2", 1);
                                        break;	

				case 3:
                                        mode = 3;
					cmd[m] = '3';
                                        put_string_center (buttons[0].x+25*j,60,"3", 1);
                                        break;	
				case 4:
                                        mode = 4;
					cmd[m] = '4';	
					put_string_center (buttons[0].x+25*j,60,"4", 1);
                                        break;
				case 5:
                                        mode = 5;
					cmd[m] = '5';
                                        put_string_center (buttons[0].x+25*j,60,"5", 1);
                                        break;
				case 6:
                                        mode = 6;
					cmd[m] = '6';
                                        put_string_center (buttons[0].x+25*j,60,"6", 1);
                                        break;
				case 7:
                                        mode = 7;
					cmd[m] = '7';
                                        put_string_center (buttons[0].x+25*j,60,"7", 1);
                                        break;
				case 8:
                                        mode = 8;
					cmd[m] = '8';
                                        put_string_center (buttons[0].x+25*j,60,"8", 1);
                                        break;
				case 9:
                                        mode = 9;
					cmd[m] = '9';
                                        put_string_center (buttons[0].x+25*j,60,"9", 1);
                                        break;
				
				case 10:
                                        mode = 10;
                                        break;
				
				case 11:
                                        mode = 11;
					cmd[m] = '0';	
                                        put_string_center (buttons[0].x+25*j,60,"0", 1);
                                        break;
				case 12:
                                        mode = 12;
                                        break;
				}
				m++;
				j = (++j > 12) ? 0 : j;	
				if(j == 0)
					m  = 0;
			}
					
		}
		if (samp.pressure > 0) {
			if (mode == 0x80000001)
				line (x, y, samp.x, samp.y, 0);
			x = samp.x;
			y = samp.y;
			mode |= 0x80000000;
		} 
		else
			mode &= ~0x80000000;
		if(mode == 10 || mode == 12)
			break;
	}
	put_string_center (240,105,&cmd, 1);
	if(mode == 12)
	{
		put_string_center (440,105,"call....", 1);
		gprs_call(cmd, strlen(cmd));
	}
	else if(mode == 10)
	{
		put_string_center (440,105,"mesg....", 1);
		gprs_msg(cmd, strlen(cmd));
	}
	for (;;);
	close_framebuffer();
}
