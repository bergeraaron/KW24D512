#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>


/*
port=portnum,
baudrate=460800,
bytesize=serial.EIGHTBITS,
parity=serial.PARITY_NONE,
stopbits=serial.STOPBITS_ONE,
timeout=None,  # seconds
writeTimeout=None,
rtscts=True
*/

/* baudrate settings are defined in <asm/termbits.h>, which is
included by <termios.h> */
#define BAUDRATE B38400//115200
/* change this definition for the correct port */
#define MODEMDEVICE "/dev/ttyACM0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define CRTSCTS  020000000000 /*should be defined but isn't with the C99*/
#define FALSE 0
#define TRUE 1

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

volatile int STOP=FALSE; 

int main()
{
  int fd,c, res;
  struct termios oldtio,newtio;
  unsigned char buf[255];memset(buf,0x00,255);
  unsigned char pkt[255];memset(pkt,0x00,255);
  unsigned char zbpkt[255];memset(zbpkt,0x00,255);
  int pkt_ctr = 0;
  int zbpkt_ctr = 0;
  int pld_ctr = 0;
  unsigned char tmp[2];

/* 
  Open modem device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
*/
 fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY ); 
 if (fd <0) {perror(MODEMDEVICE); exit(-1); }

 tcgetattr(fd,&oldtio); /* save current serial port settings */
 bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

/* 
  BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
  CRTSCTS : output hardware flow control (only used if the cable has
			all necessary lines. See sect. 7 of Serial-HOWTO)
  CS8     : 8n1 (8bit,no parity,1 stopbit)
  CLOCAL  : local connection, no modem contol
  CREAD   : enable receiving characters
*/
 newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
 
/*
  IGNPAR  : ignore bytes with parity errors
  ICRNL   : map CR to NL (otherwise a CR input on the other computer
			will not terminate input)
  otherwise make device raw (no other input processing)
*/
 newtio.c_iflag = IGNPAR;// | ICRNL;
 
/*
 Raw output.
*/
 newtio.c_oflag = 0;
 
/*
  ICANON  : enable canonical input
  disable all echo functionality, and don't send signals to calling program
*/
// newtio.c_lflag = ICANON;
 
/* 
  initialize all control characters 
  default values can be found in /usr/include/termios.h, and are given
  in the comments, but we don't need them here
*/
/*
 newtio.c_cc[VINTR]    = 0;     // Ctrl-c
 newtio.c_cc[VQUIT]    = 0;     // Ctrl-\//
 newtio.c_cc[VERASE]   = 0;     // del
 newtio.c_cc[VKILL]    = 0;     // @ 
 newtio.c_cc[VEOF]     = 4;     // Ctrl-d //
 newtio.c_cc[VTIME]    = 0;     // inter-character timer unused //
 newtio.c_cc[VMIN]     = 1;     // blocking read until 1 character arrives //
 newtio.c_cc[VSWTC]    = 0;     // '\0' //
 newtio.c_cc[VSTART]   = 0;     // Ctrl-q // 
 newtio.c_cc[VSTOP]    = 0;     // Ctrl-s //
 newtio.c_cc[VSUSP]    = 0;     // Ctrl-z //
 newtio.c_cc[VEOL]     = 0;     // '\0' //
 newtio.c_cc[VREPRINT] = 0;     // Ctrl-r //
 newtio.c_cc[VDISCARD] = 0;     // Ctrl-u //
 newtio.c_cc[VWERASE]  = 0;     // Ctrl-w //
 newtio.c_cc[VLNEXT]   = 0;     // Ctrl-v //
 newtio.c_cc[VEOL2]    = 0;     // '\0' //
*/
/* 
  now clean the modem line and activate the settings for the port
*/
 tcflush(fd, TCIFLUSH);
 tcsetattr(fd,TCSANOW,&newtio);

 unsigned char cmd[10][256];memset(cmd,0x00,256);

//seq 1
/**/
 cmd[0][0] = 0x02;
 cmd[0][1] = 0x52;
 cmd[0][2] = 0x00;
 cmd[0][3] = 0x00;
 cmd[0][4] = 0x52;
 //seq 2
 cmd[1][0] = 0x02;
 cmd[1][1] = 0x95;
 cmd[1][2] = 0x20;
 cmd[1][3] = 0x00;
 cmd[1][4] = 0xB5;

 //seq 3
 cmd[2][0] = 0x02;
 cmd[2][1] = 0x52;
 cmd[2][2] = 0x00;
 cmd[2][3] = 0x00;
 cmd[2][4] = 0x00;
 cmd[2][5] = 0x52;

 //seq 4
 cmd[3][0] = 0x02;
 cmd[3][1] = 0xA3;
 cmd[3][2] = 0x08;
 cmd[3][3] = 0x00;
 cmd[3][4] = 0xAB;

 //seq 5
 cmd[4][0] = 0x02;
 cmd[4][1] = 0x85;
 cmd[4][2] = 0x09;
 cmd[4][3] = 0x08;
 cmd[4][4] = 0x52;
 cmd[4][5] = 0x00;
 cmd[4][6] = 0x00;
 cmd[4][7] = 0x00;
 cmd[4][8] = 0x00;
 cmd[4][9] = 0x00;
 cmd[4][10] = 0x00;
 cmd[4][11] = 0x00;
 cmd[4][12] = 0xD6;

 //seq 6
 cmd[5][0] = 0x02;
 cmd[5][1] = 0x85;
 cmd[5][2] = 0x09;
 cmd[5][3] = 0x08;
 cmd[5][4] = 0x21;
 cmd[5][5] = 0x0B;//CHAN
 cmd[5][6] = 0x00;
 cmd[5][7] = 0x00;
 cmd[5][8] = 0x00;
 cmd[5][9] = 0x00;
 cmd[5][10] = 0x00;
 cmd[5][11] = 0x00;
 cmd[5][12] = 0xAE;

 //seq 7
 cmd[6][0] = 0x02;
 cmd[6][1] = 0x85;
 cmd[6][2] = 0x09;
 cmd[6][3] = 0x08;
 cmd[6][4] = 0x51;
 cmd[6][5] = 0x01;
 cmd[6][6] = 0x00;
 cmd[6][7] = 0x00;
 cmd[6][8] = 0x00;
 cmd[6][9] = 0x00;
 cmd[6][10] = 0x00;
 cmd[6][11] = 0x00;
 cmd[6][12] = 0xD4;

//seq 8
 cmd[7][0] = 0x02;
 cmd[7][1] = 0x85;
 cmd[7][2] = 0x09;
 cmd[7][3] = 0x08;
 cmd[7][4] = 0x52;
 cmd[7][5] = 0x01;
 cmd[7][6] = 0x00;
 cmd[7][7] = 0x00;
 cmd[7][8] = 0x00;
 cmd[7][9] = 0x00;
 cmd[7][10] = 0x00;
 cmd[7][11] = 0x00;
 cmd[7][12] = 0xD7;

//seq 9
 cmd[8][0] = 0x02;
 cmd[8][1] = 0x85;
 cmd[8][2] = 0x09;
 cmd[8][3] = 0x08;
 cmd[8][4] = 0x52;
 cmd[8][5] = 0x00;
 cmd[8][6] = 0x00;
 cmd[8][7] = 0x00;
 cmd[8][8] = 0x00;
 cmd[8][9] = 0x00;
 cmd[8][10] = 0x00;
 cmd[8][11] = 0x00;
 cmd[8][12] = 0xD6;

/**
 //end
 cmd[9][0] = 0x02;
 cmd[9][1] = 0x4E;
 cmd[9][2] = 0x00;
 cmd[9][3] = 0x01;
 cmd[9][4] = 0x00;
 cmd[9][5] = 0x00;
 cmd[9][6] = 0x4F;
**/

 int cmd_len[10];
 cmd_len[0]=5;
 cmd_len[1]=5;
 cmd_len[2]=6;
 cmd_len[3]=5;
 cmd_len[4]=13;
 cmd_len[5]=13;
 cmd_len[6]=13;
 cmd_len[7]=13;
 cmd_len[8]=13;

 int ctr = 0;
 int cmdctr = 0;
/*
 cmdctr = 9;
   for(int xp=0;xp<cmd_len[cmdctr];xp++)
    printf("%02X ",cmd[cmdctr][xp]);
  printf("\n");
 write(fd,cmd[cmdctr],cmd_len[cmdctr]);
*/
 cmdctr=0;
/*
  for(int xp=0;xp<cmd_len[cmdctr];xp++)
    printf("%02X ",cmd[cmdctr][xp]);
  printf("\n");
 write(fd,cmd[cmdctr],cmd_len[cmdctr]);
 usleep(1000);
*/
cmdctr++;

for(int xp=0;xp<cmd_len[cmdctr];xp++)
    printf("%02X ",cmd[cmdctr][xp]);
printf(" ");
write(fd,cmd[cmdctr],cmd_len[cmdctr]);

while(1)
 {
        //printf("ctr:%d\n",ctr);
        res = read(fd,buf,255);
        if(res > 0)
        {
                printf("res:%d ctr:%d ",res,ctr);

                for(int xp = 0;xp < res;xp++)
                {
                        printf("%02X ",buf[xp]);
                }

                printf("\n");
                
                if(cmdctr < 9)
                {
                  //send next command
                  printf("send next cmd:%d len:%d ",cmdctr,cmd_len[cmdctr]);
                  for(int xp=0;xp<cmd_len[cmdctr];xp++)
                    printf("%02X ",cmd[cmdctr][xp]);
                  printf(" ");
                  write(fd,cmd[cmdctr],cmd_len[cmdctr]);
		  usleep(10);
                  cmdctr++;
                }
                ctr = 0;
        }
        usleep(1);
        //ctr++;
        //if(ctr > 100000)
        //        break;
 }
 printf("write end\n");
 write(fd,cmd[9],cmd_len[9]);

 /* restore the old port settings */
 tcsetattr(fd,TCSANOW,&oldtio);


 return 0;
}
