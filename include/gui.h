#ifndef GUI_H
#define GUI_H

int initGui(int textShader, int boxShader, int boardShader);
void setState(char);
void processMouseClick(float x, float y);
void textInput(GLFWwindow* w, unsigned int uc);
void drawState(void);
void waitForMove(void);
void destroyWidgets(void);

char state;
#define STATE_SPASH 0
#define STATE_MAIN_MENU 1
#define STATE_GAME_OPS 2
#define STATE_GAME_OPS_ONLINE 3
#define STATE_WAIT 4
#define STATE_JOIN 5
#define STATE_INGAME 6
#define STATE_THEIR_TURN 7
#define TOTAL_STATES 8

char isOnline;

//special keys
#define BACKSPACE '\b'
#define ENTER '\n'

#endif