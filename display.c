#include "display.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lodepng.h>

#define MAX_FILENAME_LEN 50
#define MAX_FILE_LEN 3000
#define TEX_PATH "bin/textures/"
#define SHADER_PATH "bin/shaders/"
#define FRAG_EXT ".frag"
#define VERT_EXT ".vert"
#define FONT_WIDTH 39.
#define FONT_HEIGHT 71.

struct tex {
    char *fp;
    int tex;
    struct tex *next;
};

int activeTexture = 0;
static char _isFullscreen;
static struct tex *loadedTextures = NULL;

static void windowSize(GLFWwindow* win, int x, int y) {
    glViewport(0, 0, x, y);
    windowX = x;
    windowY = y;
}

int createWindow(int x, int y, char *name, char fullscreen) {
    if (!glfwInit()) {
        perror("Could not initialize glfw.");
        return 0;
    }

    window = glfwCreateWindow(x, y, name, fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
    windowX = x;
    windowY = y;
    _isFullscreen = fullscreen;

    if (!window) {
        perror("Could not create window.");
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        perror("Could not load OpenGL.");
        glfwTerminate();
        return 0;
    }

    printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);

    glViewport(0, 0, x, y);
    glfwSetFramebufferSizeCallback(window, windowSize);

    return 1;
}

void toggleFullscreen(void) {
    _isFullscreen = !_isFullscreen;
    glfwSetWindowMonitor(window, _isFullscreen ? glfwGetPrimaryMonitor() : NULL, 0, 0, windowX, windowY, GLFW_DONT_CARE);
}

int addShader(char *name) {
    char fragProgram[MAX_FILE_LEN];
    char vertProgram[MAX_FILE_LEN];
    char fragName[MAX_FILENAME_LEN] = SHADER_PATH;
    char vertName[MAX_FILENAME_LEN] = SHADER_PATH;
    const char *fp = fragProgram;
    const char *vp = vertProgram;
    strcat(fragName, name);
    strcat(fragName, FRAG_EXT);
    strcat(vertName, name);
    strcat(vertName, VERT_EXT);

    if (!readFile(fragName, fragProgram))
        return -1;

    if (!readFile(vertName, vertProgram))
        return -1;

    int frag, vert, prog, success;
    char infoLog[512];

    frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fp, NULL);
    glCompileShader(frag);
    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(frag, 512, NULL, infoLog);
        fprintf(stderr, "Error compiling fragment shader %s", infoLog);
        return -1;
    }

    vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vp, NULL);
    glCompileShader(vert);
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vert, 512, NULL, infoLog);
        fprintf(stderr, "Error compiling vertex shader %s", infoLog);
        return -1;
    }

    prog = glCreateProgram();

    glAttachShader(prog, frag);
    glAttachShader(prog, vert);
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(prog, 512, NULL, infoLog);
        fprintf(stderr, "ERROR LINKING SHADERS! %s", infoLog);
        return -1;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    return prog;
}

int createSquare(float x, float y, float w, float h) {
    int vbo, vao;
    /*float r = (float)windowY / (float)windowX;
    float vertices[] = {
        -r, -1.0, 0.0, 0.0,
        -r, 1.0, 0.0, 1.0,
        r, -1.0, 1.0, 0.0,
        r, -1.0, 1.0, 0.0,
        r, 1.0, 1.0, 1.0,
        -r, 1.0, 0.0, 1.0
    };*/
    float vertices[] = {
        x, y, 0.0, 0.0,
        x, y + h, 0.0, 1.0,
        x + w, y, 1.0, 0.0,
        x + w, y, 1.0, 0.0,
        x + w, y + h, 1.0, 1.0,
        x, y + h, 0.0, 1.0
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vao;
}

int newTexture(const char *fp, char alpha) {
    //check if already loaded
    struct tex *t, *last = NULL;
    for (t = loadedTextures; t; t = t->next) {
        if (strcmp(t->fp, fp) == 0)
            return t->tex;
        
        last = t;
    }

    //save texture to list
    if (last) {
        last->next = (struct tex*) malloc(sizeof(struct tex));
        last = last->next;
    }
    else {
        loadedTextures = (struct tex*) malloc(sizeof(struct tex));
        last = loadedTextures;
    }
    last->fp = (char *) malloc(strlen(fp) + 1);
    strcpy(last->fp, fp);
    last->next = NULL;

    char path[MAX_FILENAME_LEN] = TEX_PATH;
    strcat(path, fp);
    int tex;
    unsigned int width, height;
    glActiveTexture(activeTexture++);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    last->tex = tex;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *data;
    if (alpha)
        lodepng_decode32_file(&data, &width, &height, path);
    else
        lodepng_decode24_file(&data, &width, &height, path);

    if (!data) {
        last->tex = -1;
        fprintf(stderr, "Could not load texture \"%s\".", path);
        return -1;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(tex);

    free(data);
    data = NULL;
    return tex;
}

void destroyTextures(void) {
    struct tex *t, *next;
    for (t = loadedTextures; t; t = next) {
        next = t->next;
        free(t->fp);
        free(t);
    }
}

//text shader uniforms
static int _chUniform, _textColUniform;
//font texture
static int _fontTex, _fontActive;

int initTextRender(int textShader) {
    _chUniform = glGetUniformLocation(textShader, "ch");
    _textColUniform = glGetUniformLocation(textShader, "col");

    _fontActive = activeTexture;
    glUniform1i(glGetUniformLocation(textShader, "font"), activeTexture);
    glActiveTexture(GL_TEXTURE0 + activeTexture);
    _fontTex = newTexture("roboto_mono_regular_48.png", 1);
    if (_fontTex == -1)
        return 0;
}

Word *createWord(float x, float y, char *s) {
    Word *w = (Word *) malloc(sizeof(Word));
    w->x = x;
    w->y = y;
    int len = strlen(s);
    for (int i = 0; i < MAX_WORD_LEN; i++) {
        if (i < len) {
            w->chars[i].vao = createSquare(x + i * FONT_WIDTH, y, FONT_WIDTH, FONT_HEIGHT);
            w->chars[i].c = s[i];
        }
        else {
            w->chars[i].c = '\0';
        }
    }
    return w;
}

void renderWord(Word *w, struct colour c) {
    glActiveTexture(GL_TEXTURE0 + _fontActive);
    glBindTexture(GL_TEXTURE_2D, _fontTex);
    glUniform3f(_textColUniform, c.r, c.g, c.b);
    int i;
    for (i = 0; i < MAX_WORD_LEN && w->chars[i].c != '\0'; i++) {
        glUniform1i(_chUniform, w->chars[i].c);
        glBindVertexArray(w->chars[i].vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void editWord(Word *w, char *s) {
    int len = strlen(s);
    for (int i = 0; i < MAX_WORD_LEN; i++) {
        if (i < len) {
            //create new character displayer
            if (!w->chars[i].c) {
                w->chars[i].vao = createSquare(w->x + i * FONT_WIDTH, w->y, FONT_WIDTH, FONT_HEIGHT);
            }
            //copy across char
            w->chars[i].c = s[i];
        }
        else {
            w->chars[i].c = '\0';
        }
    }
}