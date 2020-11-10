#ifndef NET_H
#define NET_H

#include "settings.h"

struct serverInfo {
    char *address;
    struct settings *settings;
    __sig_atomic_t complete;
};

void *startServer(struct serverInfo*);
void *connectToServer(struct serverInfo*);
int sendMove(char *move);
int receiveMove(char *move);
int disconnect(void);

#endif