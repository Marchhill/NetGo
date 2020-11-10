#ifndef DISPLAY_H
#define DISPLAY_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define MAX_WORD_LEN 25

struct character {
    int c, vao;
};

typedef struct word {
    float x, y;
    struct character chars[MAX_WORD_LEN];
} Word;

struct colour {
    float r, g, b;
};

//window
int createWindow(int x, int y, char *name, char fullscreen);
void toggleFullscreen(void);
//shader
int addShader(char *name);
//drawing
int createSquare(float x, float y, float w, float h);
//textures
int newTexture(const char *fp, char alpha);
void destroyTextures(void);
//text rendering
int initTextRender(int textShader);
Word *createWord(float x, float y, char *s);
void renderWord(Word*, struct colour);
void editWord(Word *w, char *s);

GLFWwindow* window;
int windowX, windowY;
int activeTexture;

#endif