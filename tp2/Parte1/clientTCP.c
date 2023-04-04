/**      (C)2000-2021 FEUP
 *       tidy up some includes and parameters
 * */

#include "structs.h"
#include "getip.h"
#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"

UrlInfo parseUrl(char *url)
{
    UrlInfo url_info;
    url_info.error_parsing = false;

    char * url_copy = malloc(strlen(url));
    strcpy(url_copy, url);

    //parsing the url copy
    url_copy = strtok(url_copy, "//");
    if(strcmp(url_copy, "ftp:") != 0) {
        printf("Invalid url\n");
        url_info.error_parsing = true;
        return url_info;
    }
    
    url_copy = strtok(NULL, "/");
    if(url_copy == NULL) {
        printf("Invalid url\n");
        url_info.error_parsing = true;
        return url_info;
    }
    char * user_and_host = malloc(strlen(url_copy));
    strcpy(user_and_host, url_copy);

    url_copy =  strchr(url + 7, '/') + 1;
    char * files_path = malloc(strlen(url_copy));
    strcpy(files_path, url_copy);
    strcpy(url_info.filepath, files_path);

    char * user =  malloc(strlen(url_copy));
    user = strtok(user_and_host, "@");

    char * host =  malloc(strlen(url_copy));
    host = strtok(NULL, "@");
    if(host == NULL) { //no user
        host = user;
        strcpy(url_info.hostname, host);
        strcpy(url_info.user, "annonymous");
        strcpy(url_info.pass, "qualquer");
    }
    else {
        char * username =  malloc(strlen(user));
        username = strtok(user, ":");
        strcpy(url_info.user, username);

        char * pass =  malloc(strlen(url_copy));
        pass = strtok(NULL, ":");
        strcpy(url_info.pass, pass);
    }
   
    printf("user : %s, pass: %s, hostname : %s, filepath : %s\n", url_info.user, url_info.pass, url_info.hostname, url_info.filepath);

    return url_info;
}

int main(int argc, char **argv) 
{
    int sockfd;
    struct sockaddr_in server_addr;
    size_t bytes;

    if (argc != 2) {
        printf("The program needs the URL path (ftp://[<user>:<password>@]<host>/<url-path>) as argument.\n");
        return -1;
    }

    /*Argument handling to get the UrlInfo struct (username and password, the hostname, and the path of the file to transfer)*/
    UrlInfo parsed_args;
    parsed_args = parseUrl(argv[1]);
    if(parsed_args.error_parsing) return 1;
    char * address = getIp(parsed_args.hostname);
    printf("address = %s", address);
    unsigned a, b, c, d;
    sscanf("%d.%d.%d.%d",a, b, c, d);
    unsigned port = d + c*256;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(21);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }

    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }

    char response[512];
    int socketFile = fdopen(sockfd, "r");
    recv(sockfd, response, 512, 0);
    printf("response %s\n", response);
    /*send a string to the server*/
    char buf[512];
    sprintf(buf, "user %s", parsed_args.user);
    int len, bytes_sent, bytes_received;

    printf("length %s\n", buf);
    len = strlen(buf);
    printf("length %d\n", len);
    bytes_sent = send(sockfd, buf, len, 0);

    if (bytes_sent > 0)
        printf("Bytes escritos %ld\n", bytes);
    else {
        perror("write()");
        exit(-1);
    }

    
    recv(sockfd, response, 512, 0);
    printf("%s\n", response);

    if (close(sockfd)<0) {
        perror("close()");
        exit(-1);
    }
    return 0;
}

