#version 460 core
#define SYMBOLS 95.
#define WIDTH 3706.

uniform sampler2D font;
uniform int ch;
uniform vec3 col;

in vec2 texCoord;
out vec4 FragColor;

void main() {
    vec2 charCoord = vec2((ch - ' ') / SYMBOLS + (texCoord.x / SYMBOLS), texCoord.y);
    FragColor = vec4(col, texture(font, charCoord).w);
}