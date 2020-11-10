#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <pthread.h>
#include "gui.h"
#include "display.h"
#include "net.h"
#include "board.h"
#include "settings.h"
#include "utils.h"

#include <stdio.h>
#include <unistd.h>

#define NORMALIZED_X 1080
#define NORMALIZED_Y 720
#define MAX_WIDGETS 20
#define DEFAULT_KOMI "7.5"

#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

typedef struct widget {
    char type, action;
    double x, y, w, h;
    char *data;
    int vao, texID, activeTex;
    char active, option;
    Word *word;
} Widget;

//Widget types
#define EMPTY_WIDGET 0
#define TEXT_BUTTON 1
#define PIC_BUTTON 2
#define TEXT_WIDGET 3
#define TEXT_BOX 4
#define TEXT_BUTTON_VERIFY 5

//Action types
#define ACTION_DUMMY 0
#define ACTION_TO_MAIN 1
#define ACTION_TO_OFFLINE 2
#define ACTION_TO_JOIN 3
#define ACTION_TO_HOST 4
#define ACTION_TO_SPLASH 5
#define ACTION_TO_SETTINGS 6
#define ACTION_START_GAME 7
#define ACTION_TO_WAIT 8
#define ACTION_JOIN_SERVER 9
#define ACTION_CANCEL_JOIN 10
#define ACTION_CANCEL_HOST 11
#define ACTION_GROUP0 12

//options to select
#define OPTION_NULL 0
#define OPTION_9x9 1
#define OPTION_13x13 2
#define OPTION_19x19 3
#define OPTION_WHITE 4
#define OPTION_BLACK 5
#define OPTION_BASIC 6
#define OPTION_POSI 7
#define OPTION_SITU 8

//Text box verification types
#define VERIFY_FLOAT 0
#define VERIFY_IP 1

Widget widgets[TOTAL_STATES][MAX_WIDGETS + 1] = {
    { //SPLASH
        {TEXT_WIDGET, ACTION_DUMMY, 415., 50., 0., 0., "NetGo", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON, ACTION_TO_MAIN, 365., 200., 350., 72., "Play", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON, ACTION_DUMMY, 365., 300., 350., 72., "Settings", -1, -1, -1, 0, OPTION_NULL, NULL},
        {EMPTY_WIDGET, ACTION_DUMMY, 0., 0., 0., 0., "", -1, -1, -1, 0, OPTION_NULL, NULL}
    },
    { //MAIN_MENU
        {TEXT_BUTTON, ACTION_TO_OFFLINE, 365., 150., 350., 72., "Offline", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON, ACTION_TO_JOIN, 365., 250., 350., 72., "Join", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON, ACTION_TO_HOST, 365., 350., 350., 72., "Host", -1, -1, -1, 0, OPTION_NULL, NULL},
        {PIC_BUTTON, ACTION_TO_SPLASH, 200., 50., 80., 90., "back.png", -1, -1, -1, 0, OPTION_NULL, NULL},
        {EMPTY_WIDGET, ACTION_DUMMY, 0., 0., 0., 0., "", -1, -1, -1, 0, OPTION_NULL, NULL}
    },
    { //GAME_OPS
        {TEXT_WIDGET, ACTION_DUMMY, 415., 50., 0., 0., "Game Options:", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_WIDGET, ACTION_DUMMY, 100., 200., 0., 0., "Size:", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON, ACTION_GROUP0, 350., 200., 200., 72., " 9x9", -1, -1, -1, 1, OPTION_9x9, NULL},
        {TEXT_BUTTON, ACTION_GROUP0, 600., 200., 200., 72., "13x13", -1, -1, -1, 0, OPTION_13x13, NULL},
        {TEXT_BUTTON, ACTION_GROUP0, 850., 200., 200., 72., "19x19", -1, -1, -1, 0, OPTION_19x19, NULL},
        {TEXT_WIDGET, ACTION_DUMMY, 100., 300., 0., 0., "Ko:", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON, ACTION_GROUP0 + 1, 350., 300., 200., 72., "Basic", -1, -1, -1, 1, OPTION_BASIC, NULL},
        {TEXT_BUTTON, ACTION_GROUP0 + 1, 600., 300., 200., 72., "Posi", -1, -1, -1, 0, OPTION_POSI, NULL},
        {TEXT_BUTTON, ACTION_GROUP0 + 1, 850., 300., 200., 72., "Situ", -1, -1, -1, 0, OPTION_SITU, NULL},
        {TEXT_WIDGET, ACTION_DUMMY, 100., 400., 0., 0., "Komi:", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BOX, VERIFY_FLOAT, 300., 400., 200., 72., DEFAULT_KOMI, -1, 5, 1, 0, OPTION_NULL, NULL},
        {PIC_BUTTON, ACTION_TO_MAIN, 200., 50., 80., 90., "back.png", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON_VERIFY, ACTION_START_GAME, 850., 500., 200., 72., "Start", -1, -1, -1, 0, OPTION_NULL, NULL},
        {EMPTY_WIDGET, ACTION_DUMMY, 0., 0., 0., 0., "", -1, -1, -1, 0, OPTION_NULL, NULL}
    },
    { //GAME_OPS_ONLINE
        {TEXT_WIDGET, ACTION_DUMMY, 415., 50., 0., 0., "Game Options:", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_WIDGET, ACTION_DUMMY, 100., 200., 0., 0., "Colour:", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON, ACTION_GROUP0, 400., 200., 200., 72., "Black", -1, -1, -1, 1, OPTION_BLACK, NULL},
        {TEXT_BUTTON, ACTION_GROUP0, 650., 200., 200., 72., "White", -1, -1, -1, 0, OPTION_WHITE, NULL},
        {TEXT_WIDGET, ACTION_DUMMY, 100., 300., 0., 0., "Size:", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON, ACTION_GROUP0 + 1, 350., 300., 200., 72., " 9x9", -1, -1, -1, 1, OPTION_9x9, NULL},
        {TEXT_BUTTON, ACTION_GROUP0 + 1, 600., 300., 200., 72., "13x13", -1, -1, -1, 0, OPTION_13x13, NULL},
        {TEXT_BUTTON, ACTION_GROUP0 + 1, 850., 300., 200., 72., "19x19", -1, -1, -1, 0, OPTION_19x19, NULL},
        {TEXT_WIDGET, ACTION_DUMMY, 100., 400., 0., 0., "Ko:", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON, ACTION_GROUP0 + 2, 350., 400., 200., 72., "Basic", -1, -1, -1, 1, OPTION_BASIC, NULL},
        {TEXT_BUTTON, ACTION_GROUP0 + 2, 600., 400., 200., 72., "Posi", -1, -1, -1, 0, OPTION_POSI, NULL},
        {TEXT_BUTTON, ACTION_GROUP0 + 2, 850., 400., 200., 72., "Situ", -1, -1, -1, 0, OPTION_SITU, NULL},
        {TEXT_WIDGET, ACTION_DUMMY, 100., 500., 0., 0., "Komi:", -1, -1, -1, 1, OPTION_NULL, NULL},
        {TEXT_BOX, VERIFY_FLOAT, 300., 500., 200., 72., DEFAULT_KOMI, -1, 5, 1, 0, OPTION_NULL, NULL},
        {PIC_BUTTON, ACTION_TO_MAIN, 200., 50., 80., 90., "back.png", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON_VERIFY, ACTION_TO_WAIT, 850., 600., 200., 72., "Start", -1, -1, -1, 0, OPTION_NULL, NULL},
        {EMPTY_WIDGET, ACTION_DUMMY, 0., 0., 0., 0., "", -1, -1, -1, 0, OPTION_NULL, NULL}
    },
    { //WAIT
        {TEXT_WIDGET, ACTION_DUMMY, 100., 200., 0., 0., "Waiting for player...", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_WIDGET, ACTION_DUMMY, 100., 300., 0., 0., "IP: 255.255.255.255", -1, -1, -1, 0, OPTION_NULL, NULL},
        {PIC_BUTTON, ACTION_CANCEL_HOST, 200., 50., 80., 90., "back.png", -1, -1, -1, 0, OPTION_NULL, NULL}, //Add waiting animation
        {EMPTY_WIDGET, ACTION_DUMMY, 0., 0., 0., 0., "", -1, -1, -1, 0, OPTION_NULL, NULL}
    },
    { //JOIN
        {TEXT_WIDGET, ACTION_DUMMY, 415., 50., 0., 0., "Join Game:", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_WIDGET, ACTION_DUMMY, 180., 200., 0., 0., "IP:", -1, -1, -1, 0, OPTION_NULL, NULL},
        {PIC_BUTTON, ACTION_CANCEL_JOIN, 200., 50., 80., 90., "back.png", -1, -1, -1, 0, OPTION_NULL, NULL},
        {TEXT_BOX, VERIFY_IP, 300., 200., 600., 72., "", -1, 15, 0, 0, OPTION_NULL, NULL},
        {TEXT_BUTTON_VERIFY, ACTION_JOIN_SERVER, 740., 300., 160., 72., "Join", -1, -1, -1, 0, OPTION_NULL, NULL},
        {EMPTY_WIDGET, ACTION_DUMMY, 0., 0., 0., 0., "", -1, -1, -1, 0, OPTION_NULL, NULL}
    },
    { //INGAME
        {EMPTY_WIDGET, ACTION_DUMMY, 0., 0., 0., 0., "", -1, -1, -1, 0, OPTION_NULL, NULL}
    },
    { //THEIR_TURN
        {TEXT_WIDGET, ACTION_DUMMY, 100., 50., 0., 0., "Waiting for move...", -1, -1, -1, 0, OPTION_NULL, NULL},
        {EMPTY_WIDGET, ACTION_DUMMY, 0., 0., 0., 0., "", -1, -1, -1, 0, OPTION_NULL, NULL}
    }
};

char state, isOnline = 0;
static Widget *_inputTarget = NULL;
//box shader uniforms
static int _boxColUniform, _texUniform;
//board shader uniforms
static int _boardSizeUniform;
//shader ids
static int _textShader, _boxShader, _boardShader;

static int _activeThread = 0;
static struct settings _settings;
static struct serverInfo _server;
static pthread_t _netThread;
static char _move[2];

static void cleanupState(void);

int initGui(int textShader, int boxShader, int boardShader) {
    _texUniform = glGetUniformLocation(boxShader, "tex");
    _boxColUniform = glGetUniformLocation(boxShader, "col");
    _boardSizeUniform = glGetUniformLocation(boardShader, "boardSize");
    _textShader = textShader;
    _boxShader = boxShader;
    _boardShader = boardShader;

    Widget *b;
    char *new;
    for (int i = 0; i < TOTAL_STATES; i++) {
        for (b = widgets[i]; b->type != EMPTY_WIDGET; b++) {
            //load textures
            if (b->type == PIC_BUTTON) {
                b->activeTex = activeTexture;
                b->texID = newTexture(b->data, 1);
                if (b->texID == -1)
                    return 0;
            } //allocate memory for text boxes
            else if (b->type == TEXT_BOX) {
                new = (char *) malloc(MAX_WORD_LEN + 1);
                strcpy(new, b->data);
                b->data = new;
            }
        }
    }

    return 1;
}

void setState(char newState) {
    if (newState >= TOTAL_STATES || newState < 0)
        return;
    state = newState;

    glUseProgram(_textShader);

    Widget *b;
    for (b = widgets[state]; b->type != EMPTY_WIDGET; b++) {
        //switch b->type ?
        if (b->vao == -1 && b->type == TEXT_BUTTON || b->type == PIC_BUTTON || b->type == TEXT_BOX || b->type == TEXT_BUTTON_VERIFY)
            b->vao = createSquare(b->x, b->y, b->w, b->h);
        
        if (b->word == NULL && b->type == TEXT_BUTTON || b->type == TEXT_WIDGET || b->type == TEXT_BOX || b->type == TEXT_BUTTON_VERIFY)
            b->word = createWord(b->x, b->y, b->data);
    }
}

static void getSettings(void) {
    for (Widget *b = widgets[state]; b->type != EMPTY_WIDGET; b++) {
        if (b->option != OPTION_NULL && b->active) {
            switch (b->option) {
                case OPTION_9x9:
                    _settings.boardSize = 9;
                    break;
                case OPTION_13x13:
                    _settings.boardSize = 13;
                    break;
                case OPTION_19x19:
                    _settings.boardSize = 19;
                    break;
                case OPTION_WHITE:
                    _settings.hostIsWhite = 1;
                    break;
                case OPTION_BLACK:
                    _settings.hostIsWhite = 0;
                    break;
                case OPTION_BASIC:
                    _settings.ko = NO_SUPERKO;
                    break;
                case OPTION_POSI:
                    _settings.ko = POSI_SUPERKO;
                    break;
                case OPTION_SITU:
                    _settings.ko = SITU_SUPERKO;
                    break;
                }
            }
            else if (b->type == TEXT_BOX) {
                _settings.komi = (float) atof(b->data);
            }
    }

    //redraw board with new size
    glUseProgram(_boardShader);
    glUniform1i(_boardSizeUniform, _settings.boardSize);

    //update board
    initBoard(_settings);
}

void processMouseClick(float x, float y) {
   //normalize click coords
    x = (x / windowX) * NORMALIZED_X;
    y = (y / windowY) * NORMALIZED_Y;
    float xDist;
    float yDist;

    Widget *b;
    char action = ACTION_DUMMY;
    for (b = widgets[state]; b->type != EMPTY_WIDGET; b++) {
        xDist = x - b->x;
        yDist = y - b->y;
        if (xDist < b->w && xDist > 0. && yDist > 0. && yDist < b->h) {
            action = b->action;
            break;
        }
    }

    _inputTarget = NULL;

    if (b->type == TEXT_BOX) {
        _inputTarget = b;
    }
    else {
        switch(action) {
            case ACTION_TO_MAIN:
                setState(STATE_MAIN_MENU);
                break;
            case ACTION_TO_OFFLINE:
                setState(STATE_GAME_OPS);
                break;
            case ACTION_TO_JOIN:
                setState(STATE_JOIN);
                break;
            case ACTION_TO_HOST:
                setState(STATE_GAME_OPS_ONLINE);
                break;
            case ACTION_TO_SPLASH:
                setState(STATE_SPASH);
                break;
            case ACTION_TO_SETTINGS:
                break;
            case ACTION_START_GAME:
                getSettings();
                isOnline = 0;
                setState(STATE_INGAME);
                break;
            case ACTION_TO_WAIT:
                getSettings();
                _server.settings = &_settings;
                _server.complete = 0;
                _activeThread = 1;
                pthread_create(&_netThread, NULL, (void * (*)(void *))startServer, &_server);
                setState(STATE_WAIT);
                break;
            case ACTION_JOIN_SERVER:
                memset(&_settings, 0, sizeof(_settings));
                for (b = widgets[state]; b->type != TEXT_BOX; b++)
                    ;
                _server.address = b->data;
                _server.settings = &_settings;
                _server.complete = 0;
                _activeThread = 1;
                pthread_create(&_netThread, NULL, (void * (*)(void *))connectToServer, (void *) &_server);
                break;
            case ACTION_CANCEL_JOIN:
                pthread_cancel(_netThread);
                disconnect();
                _activeThread = 0;
                setState(STATE_MAIN_MENU);
                break;
            case ACTION_CANCEL_HOST:
                pthread_cancel(_netThread);
                disconnect();
                _activeThread = 0;
                setState(STATE_GAME_OPS_ONLINE);
                break;
            default: //button toggle groups
                b->active = 1;
                Widget *w;
                for (w = widgets[state]; w->type != EMPTY_WIDGET; w++) {
                    if (w != b && w->action == b->action)
                        w->active = 0;
                }
                break;
        }
    }
}

void waitForMove(void) {
    _move[0] = 127;
    _move[1] = 127;
    pthread_create(&_netThread, NULL, (void * (*)(void *))receiveMove, (void *) _move);
    _activeThread = 2;
    setState(STATE_THEIR_TURN);
}

static void checkNetwork(void) {
    if (_activeThread == 1) {
        if (_server.complete == 1) {
            _activeThread = 0;
            isOnline = 1;

            if (state == STATE_JOIN) {
                //redraw board with new size
                glUseProgram(_boardShader);
                glUniform1i(_boardSizeUniform, _settings.boardSize);
            }
            initBoard(_settings);
            if ((state == STATE_WAIT) ^ _settings.hostIsWhite)
                setState(STATE_INGAME);
            else
                waitForMove();
        }
        else if (_server.complete == -1) {
            setState(STATE_MAIN_MENU);
            _activeThread = 0;
        }
    }
    else if (_activeThread == 2 && _move[0] != 127 && _move[1] != 127) {
        _activeThread = 0;
        if (!verifyMove(_move, isOnline)) {
            fprintf(stderr, "Invalid move %d %d, other user may be cheating...\n", _move[0], _move[1]);
            return;
        }
        playMove(_move);
        setState(STATE_INGAME);
    }
}

//processes keys typed when there is an active text box
void textInput(GLFWwindow* w, unsigned int c) {
    //checks char is valid and targeted text box still has space (texID used as maximum cutoff)
    if (c < 0 || c >= 128 || _inputTarget == NULL)
        return;
    
    if (c != BACKSPACE && c != ENTER) {
        switch (_inputTarget->action) {
            case VERIFY_FLOAT:
            case VERIFY_IP:
                //ignore if invalid
                if (!isdigit(c) && c != '.')
                    return;
                break;
        }
    }

    char *sp, *last = NULL;
    for (sp = _inputTarget->data; *sp != '\0'; sp++)
        last = sp;

    switch (c) {
        case BACKSPACE:
            if (last)
                *last = '\0';
            break;
        case ENTER:
            _inputTarget = NULL;
            return;
        default:
            //text box at max capacity
            if (strlen(_inputTarget->data) >= _inputTarget->texID)
                return;
            //add char
            *sp = (char) c;
            *(sp + 1) = '\0';
            break;
    }

    switch (_inputTarget->action) {
            case VERIFY_FLOAT:
                //count if number of full-stops > 1
                ; //empty statement
                
                _inputTarget->activeTex = verifyFloat(_inputTarget->data);
                break;
            case VERIFY_IP:
                _inputTarget->activeTex = verifyIP(_inputTarget->data);
                break;
        }

    editWord(_inputTarget->word, _inputTarget->data);
}

//temp!
const struct colour blackCol = {0.0f, 0.0f, 0.0f};
const struct colour whiteCol = {1.0f, 1.0f, 1.0f};

void drawState(void) {
    if (_activeThread)
        checkNetwork();

    Widget *w;
    glUseProgram(_boxShader);
    char valid = 1;
    for (w = widgets[state]; w->type != EMPTY_WIDGET; w++) {
        if (w->type == TEXT_BOX && w->activeTex == 0) {
            valid = 0;
            break;
        }
    }

    //draw each widget box
    for (w = widgets[state]; w->type != EMPTY_WIDGET; w++) {
        if (w->type == TEXT_BUTTON) {
            glUniform1i(_texUniform, 0);
            if (w->action >= ACTION_GROUP0 && !w->active)
                glUniform4f(_boxColUniform, 0.7, 0.7, 0.7, 1.0);
            else
                glUniform4f(_boxColUniform, 0.0, 0.0, 1.0, 1.0);

            glBindVertexArray(w->vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        else if (w->type == TEXT_BOX) {
            glUniform1i(_texUniform, 0);
            if (_inputTarget == w)
                glUniform4f(_boxColUniform, 0.0, 0.0, 1.0, 1.0);
            else
                glUniform4f(_boxColUniform, 0.5, 0.5, 0.5, 1.0);

            glBindVertexArray(w->vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        else if (w->type == TEXT_BUTTON_VERIFY) {
            glUniform1i(_texUniform, 0);
            if (valid)
                glUniform4f(_boxColUniform, 0.0, 0.0, 1.0, 1.0);
            else
                glUniform4f(_boxColUniform, 0.7, 0.7, 0.7, 1.0);

            glBindVertexArray(w->vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        else if (w->type == PIC_BUTTON) {
            glActiveTexture(GL_TEXTURE0 + w->activeTex);
            glBindTexture(GL_TEXTURE_2D, w->texID);
            glUniform1i(_texUniform, w->activeTex);
            glUniform4f(_boxColUniform, 0.0, 0.0, 0.0, 0.0);

            glBindVertexArray(w->vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    //draw text on widgets
    glUseProgram(_textShader);
    for (w = widgets[state]; w->type != EMPTY_WIDGET; w++) {
        if (w->type == TEXT_BUTTON || w->type == TEXT_WIDGET || w->type == TEXT_BOX || w->type == TEXT_BUTTON_VERIFY) {
            renderWord(w->word, whiteCol);
        }
    }
}

void destroyWidgets(void) {
    Widget *w;
    for (int i = 0; i < TOTAL_STATES; i++) {
        for (w = widgets[i]; w->type != EMPTY_WIDGET; w++) {
            if (w->type == TEXT_BUTTON || w->type == TEXT_WIDGET || w->type == TEXT_BOX || w->type == TEXT_BUTTON_VERIFY)
                free(w->word);
            if (w->type == TEXT_BOX)
                free(w->data);
        }
    }
}