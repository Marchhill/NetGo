#include <stdio.h>
#include <ctype.h>
#include "utils.h"
#include "board.h"

char verifyIP(char *s) {
    char *sp = s;
    int n, zeroLast;
    char *numStart = s;
    //check all 4 numbers are in range
    for (int i = 0; i < 4; i++) {
        n = 0;
        //get text up to '.' or end
        for (;*sp != '.' && *sp != '\0'; sp++)
            ;
        //not enough full stops, or no number between dots
        if ((*sp == '\0' && i != 3) || numStart == sp)
            return 0;
        
        //convert string to int
        zeroLast = 0;
        while (numStart != sp) {
            if (!isdigit(*numStart))
                return 0;
            if (*numStart == '0') {
                if (zeroLast)
                    return 0;
                else
                    zeroLast = 1;
            }
            n = (10 * n) + (*numStart - '0');
            numStart++;
        }

        //out of range
        if (n < 0 || n > 255)
            return 0;
        numStart++;
        sp++;
    }

    return 1;
}

char verifyFloat(char *s) {
    int dots = 0;
    for (char *sp = s; *sp; sp++) {
        if (*sp == '.') {
            if (++dots > 1) {
                return 0;
            }

        }
    }
    return 1;
}

int readFile(char *name, char *dest) {
    FILE *f = fopen(name, "r");
    if (!f) {
        fprintf(stderr, "Could not open file \"%s\".", name);
        return 0;
    }

    int c;
    while ((c = getc(f)) != EOF)
        *(dest++) = c;
    
    *dest = '\0';

    return 1;
}

char *updateCounter(char *txt, int to, char zeros) {
    txt[0] = (to / 100) % 10 + '0';
    if (!zeros && txt[0] == '0') txt[0] = ' ';
    txt[1] = (to / 10) % 10 + '0';
    if (!zeros && txt[1] == '0') txt[1] = ' ';
    txt[2] = to % 10 + '0';

    return txt;
}

int verifyMove(char *mov, int online) {
    if (IS_PASS(mov[0]) || (!online && mov[0] == UNDO)) {
        return 1;
    }

    if (islower(mov[0]))
        mov[0] -= 'a';
    else if (isupper(mov[0]))
        mov[0] -= 'A';

    if (islower(mov[1]))
        mov[1] -= 'a';
    else if (isupper(mov[1]))
        mov[1] -= 'A';

    if (mov[0] < 0 || mov[1] < 0 || mov[0] >= boardSize || mov[1] >= boardSize) {
        fprintf(stderr, "ERROR: Invalid move, out of board range.\n");
        return 0;
    }
    
    //Ensures move is on available square
    if (board[mov[1]][mov[0]] != EMPTY) {
        fprintf(stderr, "ERROR: Invalid move, space is occupied.\n");
        return 0;
    }

    return 1;
}