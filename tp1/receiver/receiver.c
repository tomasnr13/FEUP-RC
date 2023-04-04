/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "../ll.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define PACKET_DATA 1
#define PACKET_CTRL_START 2
#define PACKET_CTRL_END 3
#define FILES_SIZE 0
#define FILES_NAME 1

int seqnr;

/*receiver's main
calls llopen
reads data packets from llread
*/
int main(int argc, char** argv)
{
  //clock_t start, end;
  //double cpu_time_used;

  int fd;
  char bytee[255];
    
  if ( (argc < 2) /*|| 
      ((strcmp("/dev/ttyS0", argv[1])!=0) && 
      (strcmp("/dev/ttyS5", argv[1])!=0) )*/) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS5\n");
    exit(1);
  }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  //start = clock();
  fd = llopen(argv[1], 2);

  int fdfile;
  int res;
  struct stat st;
  char fileToOpen[256];
  int filesize = 0;
  unsigned char buf[262];
  int seqnr = 0;

  int isEndPack = 0;
  while(!isEndPack)
  {
    res = llread(fd, buf);
    if (res == -1) continue;
    
    unsigned int packsize = res-6;
    unsigned char pack[packsize];
    memcpy(pack, &buf[4], packsize);

    if(pack[0] == PACKET_CTRL_START) 
    {
      printf("PACK IS START\n");
      for (int i = 1; i < packsize; i++)
      {
        if(pack[i] == FILES_SIZE) //then pack[i+1] is the size of the file's size
        {
          printf("PACK IS SIZE\n");
          int bshift = 0;
          for(int ii = i+2; ii < (i+2+pack[i+1]); ii++)
          {
            filesize = (pack[ii] << (8*bshift)) | filesize;
            bshift++;
          }  
        }
      }

      for (int i = 1; i < packsize; i++)
      {
        if(pack[i] == FILES_NAME) //then pack[i+1] is the size of the file's name
        {
          printf("PACK IS NAME\n");
          int bshift = 0;
          char filename[pack[i+1]];
          for(int ii = i+2; ii < (i+2+pack[i+1]); ii++)
          {
            filename[bshift] = pack[ii];
            bshift++;
          }  
          filename[bshift] = '\0';
          strcat(fileToOpen, filename);
          printf("File %s of %d bytes\n", filename, filesize);
        }
      }
      fdfile = open(fileToOpen, O_WRONLY | O_CREAT, 0777);
      if (fdfile < 0) {
        perror("Error opening file.");
        break;
      }
      
    }

    else if(pack[0] == PACKET_DATA) 
    {
      printf("PACK IS DATA\n");
      if(pack[1] != seqnr) {
        printf("Error seq number\n");
        continue;
      }
      seqnr = (seqnr + 1) % 255;
      int datasize = 256 * pack[2] + pack[3];
      write(fdfile, &pack[4], datasize);
    }

    else if(pack[0] == PACKET_CTRL_END)
    {
      printf("PACK IS END\n");
      close(fdfile);
      isEndPack = 1;
    }
  }

  llclose(fd, 2);
  //end = clock();
  //cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  //printf("receiver end: %f",cpu_time_used);

  return 0;
}
