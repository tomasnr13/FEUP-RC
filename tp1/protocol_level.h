#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#define FALSE 0
#define TRUE 1

//Campo de endereço - A
#define TRANS_SEND 0x03
#define RECV_ANS 0x03
#define RECV_SEND 0x01
#define TRANS_ANS 0x01

//Campo de controlo - C
#define UA_C 0x07
#define DISC_C 0x0b
#define FLAG 0x7e
#define SET_C 0x03
#define RR_C(r) ((r==0) ? 0x05 : 0x85)
#define REJ_C(r) ((r==0) ? 0x01 : 0x81)

#define FLAG 0x7e
#define ESC 0x7d

#define PACKET_CTRL_DATA 1
#define PACKET_CTRL_START 2
#define PACKET_CTRL_END 3

typedef enum  {START, FLAG_RCV, A_RCV, C_RCV, RETRANS, BCC_OK, STOPS} State;

typedef enum  {ISTART, IFLAG, HEADER_AND_DATA, ISTOP} StateI;

