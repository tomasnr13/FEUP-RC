#include "protocol_level.h"

int tries;
int trysend;
int idx;

/* function that attends alarm call*/
void alrmrot()            
{
  tries++;
  trysend=TRUE;
}

/*state machine used for SU frames*/
void framesSUstate(char byte, char flag_a, char flag_c, State *state, char *msg){
  char rej = 0x00;
  if(flag_c == 0x05) rej = 0x81;
  else if(flag_c == 0x85) rej = 0x01;
  switch(*state){
    case(START):
      if(byte == FLAG){
          *state = FLAG_RCV;
          msg[0] = byte;
        }
        break;
    case FLAG_RCV:
      if(byte == FLAG){
        *state  = FLAG_RCV;
        msg[0] = byte;
      } else if(byte == flag_a) {
        *state = A_RCV;
        msg[1] = byte;
      } else {
        *state = START;
      }
      break;
    case A_RCV:
      if(byte==flag_c){
        *state = C_RCV;
        msg[2] = byte;
      }
      else if((rej != 0x00) && (byte==rej)) {
        *state = RETRANS;
        msg[2] = byte;
      } else if(byte == FLAG){
        msg[0] = byte;
        *state = FLAG_RCV;
      } else {
        *state = START;
      }
      break;
    case C_RCV:
      if(byte==(msg[1]^msg[2])){
        msg[3] = byte;
        *state = BCC_OK;
      } else if(byte == FLAG){
        msg[0] = byte;
        *state = FLAG_RCV;
      } else {
        *state = START;
      }
      break;
    case BCC_OK:
      if(byte == FLAG){
        msg[4] = byte;
        *state = STOPS;
      } else {
        *state = START;
      }
      break;
    case STOPS:
      break;
  }
}

/*state machine used for I frames*/
void frameIstate(unsigned char byte, StateI *state, unsigned char *msg){
  printf("%d MSG\n", byte);
  switch(*state){
    case(ISTART):
      if(byte == FLAG){
          *state = IFLAG;
          msg[0] = byte;
        }
        break;
    case IFLAG:
      if(byte == FLAG){
        *state  = IFLAG;
        msg[0] = byte;
      } else {
        *state = HEADER_AND_DATA;
        msg[1] = byte;
        idx = 2;
      }
      break;
    case HEADER_AND_DATA:
      if(byte == FLAG && idx > 3){
        msg[idx] = byte;
        idx++;
        *state = ISTOP;
      }
      else if(byte == FLAG && idx < 3){
        msg[0] = byte;
        *state = IFLAG;
      }
      else {
        msg[idx] = byte;
        printf("%d MSG\n", msg[idx]);
        idx++;
      }
      break;
    case ISTOP:
      break;
  }
  printf("%d state\n", (*state));
}

void setAlarm()
{
  (void) signal(SIGALRM, alrmrot);
}

/*sends control frame and receives message, according to the function's parameters*/
void transmitControlFrame(int *fd, int a_send, int c_send, int a_recv, int c_recv)
{
  State state;
  int res;
  char msg[5];
  char response[5];

  //setup supervision frame
  msg[0]=FLAG;
  msg[1]=a_send;
  msg[2]=c_send;
  msg[3]=a_send ^ c_send;
  msg[4]=FLAG;

  int responseNotRead = TRUE;
  state=START;
  trysend=TRUE;
  tries=0;
  char byte[255];

  
  while(responseNotRead){
    if(tries>3){
      printf("Response message not received!\n");
      break;
    }

    if (trysend){
      res = write(*fd,msg,sizeof(msg));
      printf("%d bytes written\n", res);
      trysend=FALSE;
      alarm(3);
    }

    responseNotRead = TRUE;
    while(!trysend && responseNotRead){
      res = read(*fd,byte,1);
      framesSUstate(*byte, a_recv, c_recv, &state, response);
      if(state==STOPS){
        printf("Response message received!\n");
        responseNotRead=FALSE;
      }
    }
  }
}

/*receives control frame and sends response, according to the function's parameters*/
void receiveControlFrame(int *fd, int a_send, int c_send, int a_recv, int c_recv)
{
  State state;
  int res;
  char buf[255];
  state=START;
  int notReceived = TRUE;
  tries = 0;
  char msg[5];
  trysend = TRUE;

  while(notReceived)
  {
    if(tries>3) {
      printf("Message not received!\n");
      break;
    }
    res = read(*fd,buf,1);
    framesSUstate(*buf, a_recv, c_recv, &state, msg);
    if(state==STOPS){
      printf("Transmitter's message received!\n");
      notReceived=FALSE;
    }
    trysend = FALSE;
    alarm(3);
  }    

  char resp[5];

  //setup response frame
  resp[0]=FLAG;
  resp[1]=a_send;
  resp[2]=c_send;
  resp[3]=a_send ^ c_send;
  resp[4]=FLAG;

  if (!notReceived){
    res = write(*fd,resp,sizeof(resp));
    printf("%d bytes response message sent\n", res);
  }
}

void stuffingFrame(char *frame, int framesize, char *stuffed, int stuffedsize)
{
  int iSt = 0;
  int iFr = 0; 
  while (iSt < stuffedsize && iFr < framesize) {
    if (iFr == 0 || iFr == (framesize-1)) {
      stuffed[iSt] = frame[iFr]; //Flags are not verified
      iSt++;
    }
    else if (frame[iFr] == FLAG || frame[iFr] == ESC) {
      stuffed[iSt] = ESC;
      stuffed[iSt+1] = frame[iFr] ^ 0x20;
      iSt += 2;
    } 
    else {
      stuffed[iSt] = frame[iFr];
      iSt++;
    }

    iFr++; //next frame byte
  }
}

int destuffingFrame(unsigned char *frame, int framesize, unsigned char *destuffed, int destuffedsize)
{
  int iSt = 0;
  int iDst = 0; 
  while (iSt < framesize && iDst < destuffedsize) {
    if (iSt == 0 || iSt == (framesize-1)) {
      destuffed[iDst] = frame[iSt]; //Flags are not verified
      iDst++;
      iSt++;
    }
    else if (frame[iSt] == ESC) {
      if((frame[iSt+1] ^ 0x20) == FLAG)
      {
        destuffed[iDst] = FLAG;
        iDst++;
      }
      else if((frame[iSt+1] ^ 0x20) == ESC)
      {
        destuffed[iDst] = ESC;
        iDst++;
      }
      iSt += 2;
    } 
    else {
      destuffed[iDst] = frame[iSt];
      iDst++;
      iSt++;
    }
  }
  return iDst;
}

/*sends data frame containing information in data and receives according response
  return nr of bytes written*/
int transmitDataFrame(int fd, int datasize, char *data, int S) 
{
  int myS = ((S == 0) ? 0x00 : 0x40);
  int frame_size = 6 + datasize;
  char msg[frame_size];

  printf("%d %d %d tdf\n", fd, datasize, myS);

  msg[0] = FLAG;
  msg[1] = TRANS_SEND;
  msg[2] = myS;
  msg[3] = TRANS_SEND ^ myS;

  memcpy(&msg[4], data, datasize);
  unsigned char BCC2 = msg[4];
  for (int i = 5; i < datasize+4; i++) {
    BCC2 = BCC2 ^ msg[i];
  }
  printf("%d BCC2\n", BCC2);
  msg[datasize + 4] = BCC2;
  msg[datasize + 5] = FLAG;


  int stuffedsize = frame_size;
  for (int i = 1; i < (frame_size-1); i++) {
    if (msg[i] == FLAG || msg[i] == ESC)
      stuffedsize++;
  }

  char stuffedMsg[stuffedsize];
  stuffingFrame(msg, datasize+6, stuffedMsg, stuffedsize);

  trysend=TRUE;
  tries=0;
  int responseNotRead = TRUE;
  int res,red;
  char response[5];
  State state;
  state = START;
  int retransm = 3;

  int expected_r;
  if (S == 0) expected_r = 1;
  else expected_r = 0;

  while(responseNotRead){
    if(tries>5){
      printf("Response message ACK/NACK not received!\n");
      break;
    }

    if (trysend){
      res = write(fd,stuffedMsg,sizeof(stuffedMsg));
      printf("%d bytes written\n", res);
      trysend=FALSE;
      alarm(3);
    }

    responseNotRead = TRUE;
    while(!trysend && responseNotRead){
      red = read(fd,response,1);
      framesSUstate(*response, RECV_ANS, RR_C(expected_r), &state, response);
      if(state == RETRANS && retransm>0)
      {
        tries = 0;
        retransm--;
        responseNotRead = TRUE;
        trysend = TRUE;
        printf("REG MSG RECEIVED\n");
      }
      else if (state == RETRANS) {
        printf("Max of retransmissions reached\n");
        break;
      }
      if(state==STOPS){
        printf("Response message received!\n");
        responseNotRead=FALSE;
      }
    }
  }
  if (responseNotRead == TRUE) return -1;
  return res;
}

/*receives data frame from transmitter and sends acoording response
  returns size in bytes of the destuffed data frame
  OR -1*/
int receiveDataFrame(int fd, char *finalbuf, int r)
{
  unsigned char buf[256];
  unsigned char stuffedMsg[500];
  unsigned char msg[270];
  int notReceived = TRUE;
  int res;
  int tries = 0;
  StateI state;
  state = ISTART;
  char resp[5];
  idx = 0;

  while(notReceived)
  {
    if(tries>5) {
      printf("Message I not received!\n");
      break;
    }
    //read bytes of stuffed msg and update it's state
    res = read(fd,buf,1); //OK
    if (res == 1) {
      frameIstate(*buf, &state, stuffedMsg); //OK 
      if(state==ISTOP){
        printf("Transmitter's I message received!\n");
        notReceived=FALSE;
      }
      trysend = FALSE;
      alarm(3);
    }
  }

  unsigned char newStuffed[idx];
  memcpy(newStuffed,stuffedMsg,idx);

  printf("%d Bytes read from frame\n", idx);

  if(!notReceived) {
    int sizeOfDestufed = destuffingFrame(newStuffed, idx, msg, idx+7); 
    printf("%d Bytes OF DESTUFFED\n", sizeOfDestufed);
    //sleep(1);
    if( (msg[1]^msg[2]) != msg[3])
    {
      printf("Frame with header error\n");
      return -1;
    }
    else {
      unsigned char bcc2 = msg[4];
      for(int i=5; i<sizeOfDestufed-2; i++)
      {
        bcc2 ^= msg[i];
      }
      unsigned char bcc2rec = msg[sizeOfDestufed-2];
      printf("%d bcc2 %d\n", bcc2, bcc2rec);

      if(bcc2 == bcc2rec)
      {
        if(msg[2] == 0x00 && r==0)
        {
          printf("New I Frame received!\n");
          resp[0]=FLAG;
          resp[1]=TRANS_SEND;
          resp[2]=RR_C(1);
          resp[3]=TRANS_SEND ^ RR_C(1);
          resp[4]=FLAG;
          write(fd,resp,sizeof(resp));
          memcpy(finalbuf, &msg, sizeOfDestufed);
          return sizeOfDestufed;
        }
        else if(msg[2] == 0x40 && r==1)
        {
          printf("New I Frame received!\n");
          resp[0]=FLAG;
          resp[1]=TRANS_SEND;
          resp[2]=RR_C(0);
          resp[3]=TRANS_SEND ^ RR_C(0);
          resp[4]=FLAG;
          write(fd,resp,sizeof(resp));
          memcpy(finalbuf, &msg, sizeOfDestufed);
          return sizeOfDestufed;
        }
        else {
          printf("Duplicated I Frame received!\n");
          resp[0]=FLAG;
          resp[1]=TRANS_SEND;
          resp[2]=RR_C(r);
          resp[3]=TRANS_SEND ^ RR_C(r);
          resp[4]=FLAG;
          write(fd,resp,sizeof(resp));
          return -1;
        }
      }
      else {
        if((msg[2] == 0x40 && r==1) || (msg[2] == 0x00 && r==0)) {
          printf("Frame with error in data protection field!\n");
          resp[0]=FLAG;
          resp[1]=TRANS_SEND;
          resp[2]=REJ_C(r);
          resp[3]=TRANS_SEND ^ REJ_C(r);
          resp[4]=FLAG;
          write(fd,resp,sizeof(resp));
          return -1;
        }
        else {
          printf("Frame with error in data protection field, but it's a duplicate\n");
          resp[0]=FLAG;
          resp[1]=TRANS_SEND;
          resp[2]=RR_C(r);
          resp[3]=TRANS_SEND ^ RR_C(r);
          resp[4]=FLAG;
          write(fd,resp,sizeof(resp));
          return -1;
        }
      }
    }
  }
  printf("Failed to receive I Frame\n");
  return -1;
}
