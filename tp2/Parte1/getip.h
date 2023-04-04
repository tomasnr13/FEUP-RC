#ifndef getip_h
#define getip_h
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include<arpa/inet.h>

char *  getIp(char *hostname);
#endif