#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "board.h"
#include "history.h"
#include "net.h"
#include "settings.h"
#include "display.h"
#include "gui.h"
#include "utils.h"

#define NAME "NetGo"
#define MOVE_LEN 2

#define WIN_X 1920
#define WIN_Y 1080

void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* win, int button, int action, int mods);

//DRAW COOL BACKGROUND??
int main (int argc, char** argv) {
    printf("--- " NAME " ---\n\n");

    struct settings s = {0, 9, 0, 7.5};

    initBoard(s);

    createWindow(WIN_X, WIN_Y, NAME, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //board shader
    int boardShader = addShader("board");
    if (boardShader == -1)
        return 1;
    glUseProgram(boardShader);

    //wood texture
    glActiveTexture(GL_TEXTURE0 + activeTexture);
    int tex = newTexture("wood.png", 0);
    if (tex == -1)
        return 1;
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(glGetUniformLocation(boardShader, "tex"), 0);
    
    //board data texture
    int dataTex, dataActiveTexture = activeTexture;
    glActiveTexture(GL_TEXTURE0 + activeTexture);
    glGenTextures(1, &dataTex);
    glBindTexture(GL_TEXTURE_1D, dataTex);
    glUniform1i(glGetUniformLocation(boardShader, "board"), activeTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, 19 * 19, 0, GL_RED, GL_UNSIGNED_BYTE, board[0]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    activeTexture++;

    //board size
    glUniform1i(glGetUniformLocation(boardShader, "boardSize"), s.boardSize);

    //window size
    int windowUniform = glGetUniformLocation(boardShader, "window");

    //cursor loc
    int cursorUniform = glGetUniformLocation(boardShader, "cursor");
    double cursorX, cursorY;
    
    int boardBox = createSquare((1080. - 720.) * .5, 0, 720, 720);

    //text shader
    int textShader = addShader("text");
    if (textShader == -1)
        return 1;
    glUseProgram(textShader);

    //box shader
    int boxShader = addShader("box");
    if (boxShader == -1)
        return 1;

    if (!initGui(textShader, boxShader, boardShader))
        return 1;
    
    if (!initTextRender(textShader))
        return 1;
    
    setState(STATE_SPASH);
    const struct colour blackCol = {0.0f, 0.0f, 0.0f};
    const struct colour whiteCol = {1.0f, 1.0f, 1.0f};
    char counterText[] = "   ";

    Word *textTurn = createWord(910., 0., "Turn:");    
    Word *turnDisplay = createWord(900., 70., counterText);
    Word *blackDisplay = createWord(40., 500., counterText);
    Word *whiteDisplay = createWord(910., 500., counterText);

    glfwSetCharCallback(window, textInput);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.8f, 0.85f, 0.85f, 1.0f);
        
        glUseProgram(boardShader);

        //update board info
        glActiveTexture(GL_TEXTURE1);
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 19 * 19, GL_RED, GL_UNSIGNED_BYTE, board[0]);

        //send cursor location
        if (state == STATE_INGAME) {
            glfwGetCursorPos(window, &cursorX, &cursorY);
            glUniform2f(cursorUniform, (GLfloat) cursorX, (GLfloat) windowY - cursorY);
        }
        else {
            glUniform2f(cursorUniform, (GLfloat) 0.0, (GLfloat) 0.0);
        }

        //send current window size
        glUniform2i(windowUniform, windowX, windowY);

        glBindVertexArray(boardBox);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //update turn counter display
        if (state == STATE_INGAME || state == STATE_THEIR_TURN) {
            glUseProgram(textShader);
            renderWord(textTurn, blackCol);
            editWord(turnDisplay, updateCounter(counterText, turnCounter + 1, 0));
            renderWord(turnDisplay, (currentTurn == BLACK) ? blackCol : whiteCol);
            editWord(blackDisplay, updateCounter(counterText, blackPoints, 1));
            renderWord(blackDisplay, blackCol);
            editWord(whiteDisplay, updateCounter(counterText, whitePoints, 1));
            renderWord(whiteDisplay, whiteCol);
        }

        drawState();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(textTurn);
    free(turnDisplay);
    free(blackDisplay);
    free(whiteDisplay);

    disconnect();
    destroyTurnsFrom(turnHistory);
    destroyTextures();
    destroyWidgets();
    glfwTerminate();
    return 0;
}

void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    static int controlPressed = 0;
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
        if (action == GLFW_PRESS)
            controlPressed = 1;
        else
            controlPressed = 0;
    }
    if (action != GLFW_PRESS) //not interested in releases
        return;
    
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(win, 1); //CHANGE TO EXITING FULLSCREEN
    }
    else if (key == GLFW_KEY_Z && controlPressed) {
        playMove("<");
    }
    else if (key == GLFW_KEY_P) {
        if (!playMove("x")) {
            printf("End of game!\n");
            setState(STATE_MAIN_MENU);
        }
    }
    else if (key == GLFW_KEY_F11) {
        toggleFullscreen();
    }
    else if (key == GLFW_KEY_BACKSPACE) {
        textInput(win, BACKSPACE);
    }
    else if (key == GLFW_KEY_ENTER) {
        textInput(win, ENTER);
    }
}

void mouseCallback(GLFWwindow* win, int button, int action, int mods) {
    double x, y;
    char move[2];
    glfwGetCursorPos(win, &x, &y);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (state == STATE_INGAME && x > windowX/4. && x < 3. * windowX/4.) {
            x -= (windowX - windowY) * .5; //x offset
            double spacing = (double) windowY / (double) (boardSize + 1);
            move[0] = round((x / spacing) - 1);
            move[1] = round((y / spacing) - 1);
            if (verifyMove(move, isOnline)) {
                if (!playMove(move)) {
                    perror("ERROR!\n");
                    return;
                }
                sendMove(move);
                if (isOnline)
                    waitForMove();
            }
            else {
                printf("\a\n");
            }
        }
        else {
            processMouseClick(x, y);
        }
    }
}