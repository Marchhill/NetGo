#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "net.h"

#define PORT 19759
#define REQ_QUEUE 10

static int _fd = -1, _conn = -1, _isServer = 0;

static int sendSettings(struct settings* s);
static int receiveSettings(struct settings* s);

void *startServer(struct serverInfo *s) {
    _isServer = 1;

    //server address
    struct sockaddr_in sa;

    //create socket IPv4 TCP
    if ((_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("Could not create TCP socket.\n");
        s->complete = -1;
        return NULL;
    }

    //init bytes to zero
    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = htonl(INADDR_ANY); //accept any address

    if (bind(_fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        perror("Could not bind to port.\n");
        close(_fd);
        s->complete = -1;
        return NULL;
    }

    if (listen(_fd, REQ_QUEUE) == -1) {
        perror("Could not listen on port.\n");
        close(_fd);
        s->complete = -1;
        return NULL;
    }

    printf("Waiting for player to join...\n");
    if ((_conn = accept(_fd, NULL, NULL)) < 0) {
        printf("Could not accept new connection.\n");
        close(_fd);
        s->complete = -1;
        return NULL;
    }

    sendSettings(s->settings);
    s->complete = 1;
    return NULL;
}

void *connectToServer(struct serverInfo *s) {
    _isServer = 0;

    //server address
    struct sockaddr_in sa;

    //create socket IPv4 TCP
    if ((_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("Could not create TCP socket.");
        s->complete = -1;
        return NULL;
    }

    //init bytes to zero
    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    if (!inet_pton(AF_INET, s->address, &sa.sin_addr)) { //set server adress
        perror("Could not set server address.");
        close(_fd);
        s->complete = -1;
        return NULL;
    }

    if (connect(_fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        perror("Could not connect to server");
        close(_fd);
        s->complete = -1;
        return NULL;
    }

    receiveSettings(s->settings);
    s->complete = 1;
    return NULL;
}

int disconnect(void) {
    int success = 1;
    if (_isServer && _conn >= 0) {
        success &= (shutdown(_conn, SHUT_RDWR) != -1);
        close(_conn);
    }
    if (_fd >= 0) {
        success &= (shutdown(_fd, SHUT_RDWR) != -1);
        close(_fd);
    }

    if (!success)
        perror("Could not close connection.");

    return success;
}

int sendMove(char *move) {
    return send(_isServer ? _conn : _fd, move, 2, 0) == 2;
}

static int sendSettings(struct settings* s) {
    return send(_isServer ? _conn : _fd, s, sizeof(struct settings), 0) == sizeof(struct settings);
}

int receiveMove(char *move) {
    return recv(_isServer ? _conn : _fd, (void *) move, 2, 0) == 2;
}

static int receiveSettings(struct settings* s) {
    return recv(_isServer ? _conn : _fd, s, sizeof(struct settings), 0) == sizeof(struct settings);
}