#version 460 core
#define MAX_SIZE 19
#define MIN_SIZE 9
#define MAX_PIECE_RADIUS 20.0
#define MIN_PIECE_RADIUS 10.0
#define LINE_WIDTH 1.0

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D tex;
uniform sampler1D board;
uniform int boardSize;
uniform vec2 cursor;
uniform ivec2 window;

const float xOffset = float(window.x - window.y) / 2.;
const float spacing = float(window.y) / float(boardSize + 1) + 0.001;
const float pieceRadius = MIN_PIECE_RADIUS + (1.0 - smoothstep(float(MIN_SIZE), float(MAX_SIZE), float(boardSize))) * (MAX_PIECE_RADIUS - MIN_PIECE_RADIUS);

int boardAt(ivec2 pos) {
    if (pos.x < 0 || pos.x >= boardSize || pos.y < 0 || pos.y >= boardSize)
        return 0;
    return int(texelFetch(board, pos.x + 19 * pos.y, 0).x * 255);
}

ivec2 closest(void) {
    ivec2 r;
    r.x = int(round((gl_FragCoord.x - xOffset) / spacing)) - 1;
    r.y = boardSize - int(round(gl_FragCoord.y / spacing));

    return r;
}

int roundNearest(float x, float to) {
    return int (round(x / to) * to);
}

void main() {
    vec3 col;
    float xDist = mod(gl_FragCoord.x - xOffset, spacing);
    float yDist = mod(gl_FragCoord.y, spacing);
    xDist = min(xDist, spacing - xDist);
    yDist = min(yDist, spacing - yDist);
    bool onBoard = (gl_FragCoord.x - xOffset) > (spacing - LINE_WIDTH) && (gl_FragCoord.x - xOffset) < (window.y + LINE_WIDTH - spacing) && gl_FragCoord.y > (spacing - LINE_WIDTH) && gl_FragCoord.y < (window.y + LINE_WIDTH - spacing);
    bool onLine = xDist < (LINE_WIDTH / 2.) || yDist < (LINE_WIDTH / 2.);
    int piece = boardAt(closest());
    bool onPiece = pow(xDist, 2) + pow(yDist, 2) < (pieceRadius * pieceRadius) && (piece != '-') && (piece != '\0');
    if (onBoard && onLine) {
        if (roundNearest(gl_FragCoord.x - xOffset, spacing / floor(spacing / LINE_WIDTH)) == roundNearest(cursor.x - xOffset, spacing) || roundNearest(gl_FragCoord.y, spacing / floor(spacing / LINE_WIDTH)) == roundNearest(cursor.y, spacing))
            col = vec3(1.0, 0.0, 0.0);
        else
            col = vec3(1.0, 1.0, 1.0);
    }
    else {
        col = vec3(texture(tex, texCoord));
    }
    
    if (onPiece)
        col = (piece == 'w') ? vec3(1.0) : ((piece == 'b') ? vec3(0.0) : vec3(1.0, 0.0, 0.0));
    
    FragColor = vec4(col, 1.0);
}