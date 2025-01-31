// server.c
#include "server.h"
#include "client_handler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


pthread_t tid;
socklen_t sl;
int sfd;
struct sockaddr_in saddr;

int main() {
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(1234);

    // Create the socket
    sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // Bind the socket to the address
    bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));

    // Listen for incoming connections
    listen(sfd, 10);

    while (1) {
        // Accept a new client connection
        struct cln* c = malloc(sizeof(struct cln));
        sl = sizeof(c->caddr);
        c->cfd = accept(sfd, (struct sockaddr*)&c->caddr, &sl);

        // Create a new thread to handle the client connection
        pthread_create(&tid, NULL, cthread, c);
        pthread_detach(tid);
    }

    return 0;
}
