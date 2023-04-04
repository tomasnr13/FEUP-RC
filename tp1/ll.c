#include "ll.h"

struct termios oldtio;
int s, r;

/*opens port
  manages SU frames for both transmitter and receiver's processes
  returns the data connection identifier
  or -1 in case of error*/
int llopen(char *port, int status)
{
  int fd;
  struct termios newtio;
  //char port_path[20];
  //sprintf(port_path, "/dev/ttyS%d", port);

  fd = open(port, O_RDWR | O_NOCTTY );
  if (fd <0) {perror(port); exit(-1); }

  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  s = 0; //initializing Ns
  r = 0; //initializing packet receiver's expecting

  setAlarm();
   
  switch(status) {
    case RECEIVER:
      receiveControlFrame(&fd, RECV_ANS, UA_C, TRANS_SEND, SET_C);
      break;
    case TRANSMITTER:
      transmitControlFrame(&fd, TRANS_SEND, SET_C, RECV_ANS, UA_C);
      break;
    default:
      break;
  }
  
  return fd;
}

/* sends data frame with informationof buffer
  retrieves number of bytes written 
  or -1 in case of error*/
int llwrite(int fd, char *buffer, int lenght)
{
  int res;
  res = transmitDataFrame(fd, lenght, buffer, s);
  if(res > -1) {
    s = (s + 1) % 2;
    return res;
  }
  else { 
    printf("Error sending I frame.\n");
    return -1;
  }
}

/*receives data frame and puts information on buffer
  retrieves number of bytes read
  or -1 in case of error */
int llread(int fd, char *buffer)
{
  int res;
  res = receiveDataFrame(fd, buffer, r);
  if(res > -1) {
    r = (r + 1) % 2;
    return res;
  }
  printf("Error receiving I frame.\n");
  return -1;
}

/*closes port
  manages SU frames for both transmitter and receiver's processes
  returns 0 if everything went right
  or -1 in case of error*/
int llclose(int fd, int status)
{
  switch(status) {
    case RECEIVER:
      receiveControlFrame(&fd, RECV_ANS, DISC_C, TRANS_SEND, DISC_C);
      break;
    case TRANSMITTER:
      transmitControlFrame(&fd, TRANS_SEND, DISC_C, RECV_ANS, DISC_C);
      break;
    default:
      break;
  }

  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tc*setattr");
    exit(-1);
  }

  if (close(fd) > -1) return 0;
  else return -1;
}
