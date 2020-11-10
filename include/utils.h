#ifndef UTILS_H
#define UTILS_H

char verifyIP(char *s);
char verifyFloat(char *s);
int readFile(char *name, char *dest);
char *updateCounter(char *txt, int to, char zeros);
int verifyMove(char *mov, int online);

#endif