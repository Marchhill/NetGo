#include "board.h"
#include "history.h"
#include <stdio.h>
#define NO_DIR 4
#define OPPOSITE(c) ((c == WHITE) ? BLACK : WHITE)
#define INVERT_DIR(d) ((d == NO_DIR) ? NO_DIR : (d + 2) % 4)

unsigned char board [MAX_BOARD_SIZE][MAX_BOARD_SIZE];
int blackPoints = 0;
int whitePoints = 0;
int turnCounter = 0;
unsigned char boardSize;

static char _passedLast = 0;
static unsigned char *_visited[MAX_BOARD_SIZE * MAX_BOARD_SIZE];
static unsigned char **_vp = _visited;

static void getNeighbours(unsigned char*, unsigned char**);
static int hasLiberty(unsigned char*, char);
static struct change *capture(unsigned char*, struct change*);
static int unavailableSquares(void);
static int makeMove(unsigned char* c, struct turn* turn);

int initBoard(struct settings s) {
    if (s.boardSize > MAX_BOARD_SIZE || s.boardSize < MIN_BOARD_SIZE) {
        fprintf(stderr, "ERROR: Invalid board size %d, must be in range %d - %d.", s.boardSize, MIN_BOARD_SIZE, MAX_BOARD_SIZE);
        return 0;
    }

    initHistory(s.ko);
    boardSize = s.boardSize;
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            board[i][j] = EMPTY;
        }
    }

    return 1;
}

int playMove(unsigned char* mov) {
    //undo
    if (mov[0] == UNDO) {
        if (turnCounter > 0) {
            undo();
            turnCounter--;
            //remove all unavailable squares
            for (int i = 0; i < boardSize; i++) {
                for (int j = 0; j < boardSize; j++) {
                    if (board[i][j] == UNAVAILABLE)
                        board[i][j] = EMPTY;
                }
            }
            //recalculate unavailable
            if (!unavailableSquares())
                return 0;
        }
        return 1;
    }

    struct turn* turn = newTurn();
    if (!turn) {
        fprintf(stderr, "ERROR: Insufficient memory.\n");
        return 0;
    }

    //pass (end game if 2 consecutive passes)
    if (IS_PASS(mov[0])) {
        if (_passedLast) {
            return 0;
        }
         _passedLast = 1;
        goto end_turn;
    }
    else {
        _passedLast = 0;
    }

    //make move and add to turn record
    if (!makeMove(&board[mov[1]][mov[0]], turn))
        return 0;

    //change turn
    end_turn:
    //remove all unavailable squares
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (board[i][j] == UNAVAILABLE)
                board[i][j] = EMPTY;
        }
    }


    //calculate hash for turn
    MD5(board[0], sizeof(board), turn->hash);

    turnCounter++;

    //add unavailable symbols
    if (!unavailableSquares())
        return 0;

    return 1;
}

static void getNeighbours(unsigned char* c, unsigned char** n) {
    int x, y;
    x = GET_X(c);
    y = GET_Y(c);

    if (y < boardSize - 1)
        n[0] = &board[y+1][x];
    else
        n[0] = NULL;
    
    if (x < boardSize - 1)
        n[1] = &board[y][x+1];
    else
        n[1] = NULL;
    
    if (y > 0)
        n[2] = &board[y-1][x];
    else
        n[2] = NULL;
    
    if (x > 0)
        n[3] = &board[y][x-1];
    else
        n[3] = NULL;
}

static int hasLiberty(unsigned char* c, char dir) {
    *(_vp++) = c;
    unsigned char *n[4];
    unsigned char **hp;
    char skip = INVERT_DIR(dir);
    char hasVisited;
    getNeighbours(c, n);
    for (int i = 0; i < 4; i++) {
        if (n[i] && !(i == skip)) { 
            //has liberty so do not capture
            if (IS_EMPTY(*n[i])) {
                return 1;
            }
            else if (*n[i] == *c) {
                //skip if already visited
                hasVisited = 0;
                for(hp = _visited; hp < _vp; hp++) {
                    if (*hp == n[i]) {
                        //printf("has been visited!\n");
                        hasVisited = 1;
                        break;
                    }
                }
                //connected piece has liberty so return has liberty
                if (!hasVisited && hasLiberty(n[i], i))
                    return 1;
            }
        }
        
    }
    return 0;
}

static int makeMove(unsigned char* c, struct turn* turn) {
    turn->first = newChange(c, currentTurn);
    if (!turn->first) {
        fprintf(stderr, "ERROR: Insufficient memory.\n");
        return 0;
    }

    *c = currentTurn;

    struct change* lastChange = turn->first;

    //capture pieces
    unsigned char *n[4];
    unsigned char **np = n;
    char hasCaptured = 0;
    getNeighbours(c, n);
    for (int i = 0; i < 4; i++) {
        if (*np && **np == nextTurn) {
            //reset visited
            _vp = _visited;
            if (!hasLiberty(*np, i)) {
                lastChange = capture(*np, lastChange);
                if (!lastChange)
                    return 0;
                hasCaptured = 1;
            }
        }
        np++;
    }

    return 1;
}

static struct change *capture(unsigned char* c, struct change* lastChange) {
    unsigned char **cp;
    for (cp = _visited; cp < _vp; cp++) {
        lastChange->next = newChange(*cp, EMPTY);
        if (!lastChange->next) {
            fprintf(stderr, "ERROR: Insufficient memory.\n");
            return NULL;
        }
        lastChange = lastChange->next;

        whitePoints += (**cp == BLACK);
        blackPoints += (**cp == WHITE);
        **cp = EMPTY;
    }
    return lastChange;
}

static int unavailableSquares(void) {
    char xBuf[MAX_BOARD_SIZE * MAX_BOARD_SIZE];
    int i, j;

    for (i = 0; i < boardSize; i++) {
        for (j = 0; j < boardSize; j++) {
            xBuf[i * boardSize + j] = '\0';
            if (board[i][j] == EMPTY) {
                board[i][j] = currentTurn;

                //reset visited
                _vp = _visited;

                //check if has any neighbours
                unsigned char *n[4];
                unsigned char **np = n;
                char hasNeighbour = 0;
                char hasConnectedLiberty = 0;
                getNeighbours(&board[i][j], np);
                for (int dir = 0; dir < 4; dir++) {
                    if (*np) {
                        //if this piece or a connected piece has a liberty, or enemy pieces can be captured then allow move
                        if (IS_EMPTY(**np)) {
                            //has a liberty
                            hasConnectedLiberty = 1;
                        }
                        else {
                            //has a neighbouring piece
                            hasNeighbour = 1;
                            
                            //has a connected liberty or enemy doesnt
                            _vp = _visited;
                            if (**np == currentTurn && hasLiberty(*np, dir)) {
                                hasConnectedLiberty = 1;
                                break;
                            }
                            _vp = _visited;
                            if (**np == nextTurn && !hasLiberty(*np, dir)) {
                                hasConnectedLiberty = 1;
                                break;
                            }
                        }
                    }
                    np++;
                }

                board[i][j] = EMPTY;

                //suicide not allowed
                if (!hasConnectedLiberty) {
                    xBuf[i * boardSize + j] = UNAVAILABLE;
                }
                else if (hasNeighbour) { //check ko (only neccasary if the piece has at least one neigbour)
                    //simulate turn for hash
                    struct turn* sim = newTurn();
                    if (!newTurn) {
                        fprintf(stderr, "ERROR: Insufficient memory.\n");
                        return 0;
                    }
                    makeMove(&board[i][j], sim);
                    MD5(board[0], sizeof(board), sim->hash);
                    //revert to original state
                    turnCounter++;
                    undo();
                    turnCounter--;

                    if (searchHistory(sim->hash)) {
                        xBuf[i * boardSize + j] = UNAVAILABLE;
                    }
                }
            }
        }
    }

    for (i = 0; i < boardSize; i++) {
        for (j = 0; j < boardSize; j++) {
            if (xBuf[i * boardSize + j])
                board[i][j] = xBuf[i * boardSize + j];
        }
    }
    return 1;
}