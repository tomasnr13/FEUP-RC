#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h> 
#include <fcntl.h>
#include <arpa/inet.h>
#define h_addr h_addr_list[0]	

typedef struct info{
  char user[128]; 
  char password[128];   
  char host[256];   
  char path[240];    
  char filename[128]; 
  char hostname[128];  
  char ip[128];
}info;

/*struct hostent {
    char *h_name;    // Official name of the host.
    char **h_aliases;    // A NULL-terminated array of alternate names for the host.
    int h_addrtype;    // The type of address being returned; usually AF_INET.
    int h_length;    // The length of the address in bytes.
    char **h_addr_list;    // A zero-terminated array of network addresses for the host.
    // Host addresses are in Network Byte Order.
};*/


FILE * socketfile;
char * uph;
info args;

int parseUrl(char * url, info *args){
  char* ftp = strtok(url, "/");  
  char* urlrest = strtok(NULL, "/"); 
  char* path = strtok(NULL, "");    

  if (strcmp(ftp, "ftp:") != 0){
    printf("Error: Not using ftp\n");
    return 1;
  }
  
  char* user = strtok(urlrest, ":");
  char* pass = strtok(NULL, "@");


  // no user:password given
  if (pass == NULL)
  {
    user = "anonymous";
    pass = "pass";
    strcpy(args->host, urlrest);
  } else
    strcpy(args->host, strtok(NULL, ""));
  

  strcpy(args->path, path);
  strcpy(args->user, user);
  strcpy(args->password, pass);
  
  struct hostent *h;
    if ((h = gethostbyname(args->host)) == NULL) {
    herror("gethostbyname()");
    exit(-1);
  }
  printf("Host name  : %s\n", h->h_name);
  printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *) h->h_addr)));
  strcpy(args->hostname,h->h_name);
  strcpy(args->ip, inet_ntoa( *( (struct in_addr *)h->h_addr)));

  char fullpath[256];
  strcpy(fullpath, args->path);
  char* token = strtok(fullpath, "/");
  while( token != NULL ) {
    strcpy(args->filename, token);
    token = strtok(NULL, "/");
  }
  return 0;
}

int init(char *ip, int port, int *socketfd){
  struct sockaddr_in server_addr;

  /*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */
    
	/*opens a TCP socket*/
	if ((*socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    perror("socket()");
    return 1;
  }

	/*connects to the server*/
  if(connect(*socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
    perror("connect()");
		return 1;
	}

  return 0;
}

int sendCommand(int socketfd, char * command){
  printf(" about to send command: \n> %s", command);
  int sent = send(socketfd, command, strlen(command), 0);
  if (sent == 0){
    printf("sendCommand: Connection closed");
    return 1;
  }
  if (sent == -1){
    printf("sendCommand: error");
    return 2;
  }
  printf("> command sent\n");
  return 0;
}

int readResponse(){
  char * buf;
  size_t bytesRead = 0;

  while (1){
    getline(&buf, &bytesRead, socketfile);
    printf("< %s", buf);
    if (buf[3] == ' '){
      long code = strtol(buf, &buf, 10);


      break;
    }
  }
  return 0;
}

int readResponsePassive(char (*ip)[], int *port){
    printf("rrps");
  char * buf;
	size_t bytesRead = 0;

  while (1){
    getline(&buf, &bytesRead, socketfile);
    printf("< %s", buf);
    if (buf[3] == ' '){
      break;
    }
  }

  strtok(buf, "(");       
  char* ip1 = strtok(NULL, ",");       // 193
  char* ip2 = strtok(NULL, ",");       // 137
  char* ip3 = strtok(NULL, ",");       // 29
  char* ip4 = strtok(NULL, ",");       // 15

  sprintf(*ip, "%s.%s.%s.%s", ip1, ip2, ip3, ip4);
  
  char* p1 = strtok(NULL, ",");       // 199
  char* p2 = strtok(NULL, ")");       // 78

  *port = atoi(p1)*256 + atoi(p2);
    printf("rrpe");
  return 0;
}

  
int saveFile(char* filename, int socketfd){
  printf("> filename: %s\n", filename);
  int filefd = open(filename, O_WRONLY | O_CREAT, 0777);
  if (filefd < 0){
    printf("Error: saveFile\n");
    return 1;
  }
  
  int bytes_read;
  char buf[1];
  do {
    bytes_read = read(socketfd, buf, 1);
    if (bytes_read > 0) write(filefd, buf, bytes_read);
  } while (bytes_read != 0);
  
  close(filefd);
  return 0;
}

int main(int argc, char ** argv){
    int socketfd, socketfd_r;
    char command[256];

    if(argc!=2){
        return 1;
    }

    parseUrl(argv[1], &args);

    printf("\nhost: %s\npath: %s\nuser: %s\npassword: %s\nfile name: %s\nhost name: %s\nip address: %s\n\n", 
  args.host, args.path, args.user, args.password, args.filename, args.hostname, args.ip);

    printf("here");
    init(args.ip, 21, &socketfd);

    socketfile = fdopen(socketfd, "r");
	readResponse();

    // login
    sprintf(command, "user %s\r\n", args.user);
    sendCommand(socketfd, command);
    if (readResponse() != 0) return 1;
    sprintf(command, "pass %s\r\n", args.password);
    sendCommand(socketfd, command);
    if (readResponse() != 0) return 1;

    // get ip and port
    char ip[32]; int port;
    sprintf(command, "pasv\r\n");
    sendCommand(socketfd, command);
    readResponsePassive(&ip, &port);
    printf("ip: %s\nport: %d\n", ip, port);

    if (init(ip, port, &socketfd_r) != 0){
        printf("Error: init()\n");
        return -1;
    }

    sprintf(command, "retr %s\r\n", args.path);
    sendCommand(socketfd, command);
    if (readResponse() != 0) return 1;

    saveFile(args.filename, socketfd_r);
    return 0;

}