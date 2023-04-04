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
#include "protocol_level.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define RECEIVER 2
#define TRANSMITTER 1

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

#define FLAG 0x7e
