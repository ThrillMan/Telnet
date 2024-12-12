// server.h
#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


struct cln {
    int cfd;
    struct sockaddr_in caddr;
};

void* cthread(void* arg);


#endif // SERVER_H
