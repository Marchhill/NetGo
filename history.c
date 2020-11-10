#include "history.h"
#include "board.h"
#include <stdlib.h>
#include <string.h>

struct turn* turnHistory = NULL;
static unsigned char _ko;

static void destroyChangesFrom(struct change*);

void initHistory(unsigned char ko) {
    _ko = ko;
} 

void undo(void) {
    if (turnCounter == 0)
        return;
    
    struct turn* tp = turnHistory;
    struct turn* last = NULL;
    //move forward to most recent turn
    int i;
    for (i = 1; i < turnCounter; i++) {
        last = tp;
        tp = tp->next;
    }
    if (last)
        last->next = NULL;
    else
        turnHistory = NULL;
    
    struct change* cp;
    //revert board
    for (cp = tp->first; cp; cp = cp->next) {
        board[cp->y][cp->x] = cp->before;
        //remove points if piece captured
        if (!IS_EMPTY(cp->before) && IS_EMPTY(cp->after)) {
            whitePoints -= (cp->before == BLACK);
            blackPoints -= (cp->before == WHITE);
        }
    }
    destroyTurnsFrom(tp);
}

void destroyTurnsFrom(struct turn* root) {
    struct turn* tp = root; //turn pointer
    struct turn* tnext = root; //next turn pointer
    struct change* cp; //change pointer
    struct change* cnext; //next change pointer
    for (; tp; tp = tnext) {
        tnext = tp->next;
        //free changes
        for (cp = tp->first; cp; cp = cnext) {
            cnext = cp->next;
            free(cp);
        }
        free(tp);
    }
}

static void destroyChangesFrom(struct change* root) {
    struct change* cp; //change pointer
    struct change* cnext; //next change pointer
    for (cp = root; cp; cp = cnext) {
        cnext = cp->next;
        free(cp);
    }
}

int searchHistory(char* hash) {
    struct turn* tp = turnHistory;

    switch (_ko) {
        case NO_SUPERKO:
            for (int t = 0; t < turnCounter - 1; t++) {
                if ((turnCounter - t) <= 2 && strncmp(hash, tp->hash, MD5_DIGEST_LENGTH) == 0)
                    return 1;
                tp = tp->next;
            }
            break;
        case POSI_SUPERKO:
            for (int t = 0; t < turnCounter - 1; t++) {
                //found collision with previous position
                if (strncmp(hash, tp->hash, MD5_DIGEST_LENGTH) == 0)
                    return 1;
                tp = tp->next;
            }
            break;
        case SITU_SUPERKO:
            for (int t = 0; t < turnCounter - 1; t++, tp = tp -> next) {
                //incorrect turn, skip
                if (GET_TURN(t) == nextTurn)
                    continue;
                
                //found collision with previous position
                if (strncmp(hash, tp->hash, MD5_DIGEST_LENGTH) == 0)
                    return 1;
            }
            break;
    }

    return 0;
}

struct turn* newTurn(void) {
    struct turn* turn = turnHistory;
    int i;

    for (i = 1; i < turnCounter; i++)
        turn = turn->next;
    
    if (!turn) {
        turn = (struct turn*) malloc(sizeof(struct turn));
        if (!turn) {
            return NULL;
        }
        turn->next = NULL;
        turnHistory = turn;
    }
    else {
        turn->next = (struct turn*) malloc(sizeof(struct turn));
        turn = turn->next;
        if (!turn) {
            return NULL;
        }
        turn->next = NULL;
    }
    turn->first = NULL;

    return turn;
}

struct change* newChange(unsigned char* c, unsigned char to) {
    struct change* change = (struct change*) malloc(sizeof(struct change));
    if (!change) {
        return NULL;
    }
    change->x = GET_X(c);
    change->y = GET_Y(c);
    change->before = *c;
    change->after = to;
    change->next = NULL;

    return change;
}