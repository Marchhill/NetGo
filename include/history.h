#ifndef HISTORY_H
#define HISTORY_H

#include <openssl/md5.h>

//change of piece on the board
struct change {
    int x, y;
    unsigned char before, after;
    struct change* next;
};

//all changes in a turn
struct turn {
    struct change* first;
    struct turn* next;
    unsigned char hash[MD5_DIGEST_LENGTH];
};

void initHistory(unsigned char);
void undo(void);
void destroyTurnsFrom(struct turn* root);
int searchHistory(char* hash);
struct turn* newTurn(void);
struct change* newChange(unsigned char* c, unsigned char to);

struct turn* turnHistory;

#endif