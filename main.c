/************************************************
 *  GPRS demo, use ppp to connect internet
 *  use ttyS1 to ctrol GPRS
 *  by Zou jian guo <ah_zou@163.com>   
 *  2004-11-02
 *  modify by sprife for UP_CUP6410II GPRS 
*************************************************/


#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

//#include <sys/signal.h>
#include <pthread.h>

#include <sys/types.h>                      //tty.c                                               
#include <sys/stat.h>                                                      
//#include <fcntl.h>                                                       
//#include <termios.h>                                                    
//#include <stdio.h>    
//#include <stdlib.h>
#include <string.h>

#include <errno.h>                 /* //keyshell.c*/

#define COM "/dev/ttyS0"
//#define COM "/dev/ttySAC1"


//#include <stdio.h>                     //gprs.c
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/signal.h>
//#include "tty.h"
//#include"gprs.h"


//#include <stdio.h>                //keyshell.c
//#include <stdlib.h>
//#include <string.h>
//#include <sys/types.h>
//#include <fcntl.h>
//#include <unistd.h>

//#include <pthread.h>
//#include <time.h>

char shell_s[]="\nkeyshell>$: ";

/*int argc, char *argv[]*/
#define CMD_TPYE			(0)
#define PHONE_NUMBER		(1)
#define MAX_CMD_DEPTH  	(2)
extern volatile int STOP ;

static int fd;
static struct termios oldtio,newtio;


					//#include "tty.h"
#ifndef  __TTY_H__
#define __TTY_H__

int tty_init();
int tty_end();
int tty_read(char *buf,int nbytes);
int tty_write(char *buf,int nbytes);
int tty_writecmd(char *buf,int nbytes);
void tty_flush();
#endif

					//#include "gprs.h"
#ifndef __GPRS_H__
#define __GPRS_H__

extern char * cmd[];

void gprs_init();
void gprs_msg(char *number, int num);
void gprs_call(char *number, int num);
void gprs_hold();
void gprs_ans();
void gprs_call(char *number, int num);
void gprs_baud(char *baud,int num);
#endif


/*--------------------------------------------------------*/
#define ENDMINITERM 27 /* ESC to quit miniterm */
#define FALSE		0
#define TRUE  		1


/*--------------------------------------------------------*/
volatile int STOP=FALSE;
int GET_GPRS_OK=FALSE;
volatile int baud=B9600;

//extern int baud;             // defined in tty.c 

//======keyshell.c
int get_line(char *cmd)
{
 	int i=0;
 	char temp;
	while (1) { 
		temp = getchar();

		if (temp == '\r' || temp == '\n') 
		{
			return 0 ;
		}
	
		cmd[i]=temp;
 
             	if(cmd[i]==13)
		{
                        cmd[i]=0;
                        break;
                }
		  fflush(stdout);
		  i++;
	}
}

void * keyshell()
{

	char cmd[256]={0,};
	
	gprs_init();		//GPRS 初始化	
	
	printf("\n<gprs control shell>");
	printf("\n [1]  give a call");		
	printf("\n [2]  respond a call");		
	printf("\n [3]  hold a call");		
	printf("\n [4]  send a msg");
	printf("\n [5]  change baudrate");
	printf("\n [6]  exit");
	printf("\n [**] help menu");

	while(1){

		printf(shell_s);
		fflush(stdout);

		get_line(cmd);
		printf("\r\n");				//显示必要的输出

		if(strncmp("1",cmd,1)==0){
			
			printf("\nyou select to gvie a call, please input number:");	
			fflush(stdout);
			get_line(cmd);
			gprs_call(cmd, strlen(cmd));
			printf("\ncalling......");		
		} else if(strncmp("2",cmd,1)==0){
			
			gprs_ans();
			printf("\nanswering.......");	
		} else if(strncmp("3",cmd,1)==0){

			gprs_hold();
			printf("\nhold a call");
		}else if (strncmp("4",cmd,1)==0){
		
			printf("\nyou select to send a message, please input number:");	
			fflush(stdout);
			get_line(cmd);
			gprs_msg(cmd, strlen(cmd));
			printf("\nsending......");	
		} else if(strncmp("6",cmd,1)==0){
			printf("\nexit this program!\n");
			STOP=1;
			break;
		} else if(strncmp("5",cmd,1)==0){
			printf("\nyou select to change baudrate, please input baudrate:");
			fflush(stdout);
			get_line(cmd);
			gprs_baud(cmd, strlen(cmd));
			printf("please exit and run as another baudrate.");
		}else if (strncmp("**",cmd,2)==0){
			printf("\n<gprs control shell>");
			printf("\n [1]  give a call");
			printf("\n [2]  respond a call");		
			printf("\n [3]  hold a call");		
			printf("\n [4]  send a msg");
			printf("\n [5]  change baudrate");
			printf("\n [6]  exit");
			printf("\n [**] help menu");
		}else if(cmd[0] != 0){
		
			system(cmd);
		}		
	}		
}                      //==keyshell.c

//===========tty.c
//==============================================================
int tty_end()
{
  	tcsetattr(fd,TCSANOW,&oldtio);	 	/* restore old modem setings */
//  	tcsetattr(0,TCSANOW,&oldstdtio); 	/* restore old tty setings */
  	close(fd);
}
//==============================================================
int tty_read(char *buf,int nbytes)
{
	return read(fd,buf,nbytes);
}
//==============================================================
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

//==============================================================
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

//==============================================================
/*int tty_writebyte(char *buf)
{
	write(fd,&buf[0],1);
	usleep(10);
//	write(fd,buf,nbytes);
	return tcdrain(fd);
}*/
//==============================================================

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
	tcsetattr(fd,TCSANOW,&newtio);/*set attrib	  */

/* now clean the modem line and activate the settings for modem */
// 	tcflush(fd, TCIFLUSH);
//	tcsetattr(fd,TCSANOW,&newtio);/*set attrib	  */
//	signal(SIGTERM,do_exit);
//	signal(SIGQUIT,do_exit);
//	signal(SIGINT,do_exit);
	return 0;
} //==================tty.c end



//==============gprs.c
void gprs_init()
{ 
	int i;
	for(i=0; i<5; i++) {
		tty_writecmd(cmd[i], strlen(cmd[i])); 
		
	}
	//下面选择通道0，使用普通电话话柄
	/*
	  gprs_cmd("AT+CHFA=0\r"); //设置通话通道为0
	  usleep(1000);
	  gprs_cmd("AT+CLVL=100\r");//设置受话器音量最大
	  usleep(1000);
	  gprs_cmd("AT+CMIC=0,10\r");//设置通道0的话筒增益
	  usleep(1000);
	*/
}

void gprs_hold()
{
	tty_writecmd("AT", strlen("AT"));
	tty_writecmd("ATH", strlen("ATH"));//发送挂机命令ATH
}

void gprs_ans()
{ 
	tty_writecmd("AT", strlen("AT"));
	tty_writecmd("ATA", strlen("ATA"));//发送接听命令ATA
}

//拨叫  
void gprs_call(char *number, int num)			
{ 

	tty_write("ATD", strlen("ATD")); //发送拨打命令ATD	
	tty_write(number, num);
	tty_write(";\r", strlen(";\r"));
	usleep(200000);

}

//发送短信
void gprs_msg(char *number, int num)		
{ 
	int i = 0;                //miguihua
	char ctl[]={26,0};
	char text[100];                     //miguihua
	tty_writecmd("AT", strlen("AT"));
	//usleep(5000);

	tty_writecmd("ATE0", strlen("ATE0"));
	tty_writecmd("AT+CMGF=1", strlen("AT+CMGF=1"));		//发送修改字符集命令
	tty_write("AT+CMGS=", strlen("AT+CMGS="));	//发送发短信命令，具体格式见手册
	tty_write("\"", strlen("\""));
	tty_write(number, strlen(number));
  	tty_write("\"", strlen("\""));
  	tty_write("\r", strlen("\r"));
	usleep(10000);
	printf("Please enter the text:\n");
	
	text[0] = getchar();
	while(i < 100)
	{
		i++;
		text[i] = getchar();
		
		if(text[i] == '\n')
		{
			text[i] = ' ';
			break;
		}
	}
	
        tty_write(text, strlen(text));
	tty_write(ctl, 1);      //“CTRL+Z"的ASCII码

	usleep(300000);
}
void gprs_baud(char *baud,int num)
{
	tty_write("AT+IPR=", strlen("AT+IPR="));
	tty_writecmd(baud, strlen(baud) );
	usleep(200000);
}//============gprs.c end


char * cmd[20]={
		"AT",
		"ATE1",
		"AT+CHFA=1", 			//设置通话通道为1,AT+CHFA 命令切换主副音频通道
		"AT+CLVL=100",			//设置受话器音量最大, AT+CLVL 命令可以调节输出音频信号增益
		"AT+CMIC=1,10"			//设置通道1的话筒增益
//		"atd12345678\r",
};

//void * keyshell() ;


/* modem input handler */
void* gprs_read(void * data)
{
	int i=0;
	char c;
	char buf[1024];
  	printf("\nread modem\n");
	
  	while (STOP==FALSE) 	{		

	    	tty_read(&c,1); 	
		printf("%c",c);	
	}
  	printf("exit from reading modem\n");
  	return NULL; 
}

int get_baudrate(int argc,char** argv)
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

/*--------------------------------------------------------*/
int main(int argc,char** argv)
{
	int ok;
 	pthread_t th_a, th_b;
 	void * retval;
	if (argc >1)
		{
			baud=get_baudrate(argc, argv);
		}
	tty_init();

	pthread_create(&th_b, NULL, gprs_read, 0);
  	pthread_create(&th_a, NULL, keyshell, 0); 
	
	while(!STOP){
		usleep(100000);
	}
  	tty_end();
  	exit(0); 
}
