#ifndef BOARD_H
#define BOARD_H
#define MAX_BOARD_SIZE 19
#define MIN_BOARD_SIZE 5

#define NO_SUPERKO 0
#define POSI_SUPERKO 1
#define SITU_SUPERKO 2

#define EMPTY '-'
#define WHITE 'w'
#define BLACK 'b'
#define UNAVAILABLE 'x'
#define IS_PASS(c) (c == 'x' || c == 'X')
#define UNDO '<'

#define GET_X(c) ((c - board[0]) % MAX_BOARD_SIZE)
#define GET_Y(c) ((c - board[0]) / MAX_BOARD_SIZE)
#define GET_TURN(t) ((t % 2 == 0) ? BLACK : WHITE)
#define IS_EMPTY(c) (c != WHITE && c != BLACK)

#include "settings.h"

int initBoard(struct settings);
int playMove(unsigned char* mov);

unsigned char board [MAX_BOARD_SIZE][MAX_BOARD_SIZE];
unsigned char boardSize;
int blackPoints;
int whitePoints;
int turnCounter;

#define currentTurn ((turnCounter % 2 == 0) ? BLACK : WHITE)
#define nextTurn ((turnCounter % 2 == 1) ? BLACK : WHITE)

#endif