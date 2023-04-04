#ifndef structs_h
#define structs_h
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SIZE 512

typedef struct {
    char user[MAX_SIZE];
    char pass[MAX_SIZE];
    char hostname[MAX_SIZE];
    char filepath[MAX_SIZE];
    bool error_parsing;
} UrlInfo;

#endif