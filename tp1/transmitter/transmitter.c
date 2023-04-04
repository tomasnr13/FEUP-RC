/*Non-Canonical Input Processing*/

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
#include "../ll.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define PACKET_DATA 1
#define PACKET_CTRL_START 2
#define PACKET_CTRL_END 3
#define FILES_SIZE 0
#define FILES_NAME 1

#include <time.h>

/*opens file
  sends control start packet
  sends fragmentated file in data packets of 100 bytes
  send control end packet
  to the receiver process
  return -1 in case of error*/
int transmitFile(int *fd, char *file_path)
{
  int fdfile;
  struct stat st;
  int byteswrote = 0;

  if (stat(file_path, &st) < 0) {
    perror("error in stat");
    return -1;
  }
  if ((fdfile = open(file_path, O_RDONLY)) < 0) {
    perror("error opening file");
    return -1;
  }

  // get the files name
  char *filename = file_path;
  for(int i=0; file_path[i++]; i++){
    if(file_path[i] == '/'){
      filename = file_path + i + 1;
    }
  }

  //create and send control start packet

  int filesize = st.st_size;
  printf("opened %s of %d\n", filename, filesize);
  int L1 = sizeof(filesize);

  int L2 = strlen(filename);

  char pack[5+L1+L2];

  pack[0] = PACKET_CTRL_START;
  pack[1] = FILES_SIZE;
  pack[2] = L1;
  memcpy(&pack[3], &filesize, L1);
  pack[L1+3] = FILES_NAME;
  pack[L1+4] = L2;
  memcpy(&pack[5+L1], filename, L2);

  byteswrote = llwrite((*fd), pack, 5+L1+L2);
  if (byteswrote == -1) return -1;

  //create and send data packets

  printf("%d %d %s\n", fdfile, filesize, filename);
  unsigned char buf[256];
  unsigned res = 0;
  int seqnumber = 0;
  int bytessent = 0;
  char datapack[100]; //each pack will have size 256
  while((res = read(fdfile, buf, 96)) > 0)  //read 256 - 4 (header bytes of the pack) data bytes
  {
    printf("%d res\n", res);
    datapack[0] = PACKET_DATA;
    datapack[1] = seqnumber % 255;
    datapack[2] = res / 256;
    datapack[3] = res % 256;
    memcpy(&datapack[4], buf, res);
    bytessent += res;
    byteswrote = llwrite((*fd), datapack, res+4);
    if (byteswrote == -1) return -1;

    printf("%d packet number\n", seqnumber);
    seqnumber++;
  }

  //create and send control end packet

  char pack2[5+L1+L2];

  pack2[0] = PACKET_CTRL_END;
  pack2[1] = FILES_SIZE;
  pack2[2] = L1;
  memcpy(&pack2[3], &filesize, L1);
  pack2[3+L1] = FILES_NAME;
  pack2[4+L1] = L2;
  memcpy(&pack2[5+L1], filename, L2);

  byteswrote = llwrite((*fd), pack2, 5+L1+L2);
  if (byteswrote == -1) return -1;

  if (close(fdfile) > -1) return 0;
  else return -1;
}

/*transmitter's  main*/
int main(int argc, char** argv)
{
  clock_t start, end;
  double cpu_time_used;

  int fd;
  char buf[255];

  char file_path[256];
  file_path[0] = '\0';
    
  if ( argc < 3 )  {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS5\n");
    exit(1);
  }
  strcpy(file_path, argv[2]);

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  start = clock();
  fd = llopen(argv[1], 1);

  if (transmitFile(&fd, file_path) == -1) return -1;

  llclose(fd, 1);

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("transmitter time: %f",cpu_time_used);
  return 0;
}
