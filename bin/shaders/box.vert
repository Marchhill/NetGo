#version 460 core
#define winX 1080
#define winY 720

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;

void main() {
    gl_Position = vec4(2. * vec2(aPos.x / winX, (winY - aPos.y) / winY) - vec2(1.0, 1.0), 0.0, 1.0);
    texCoord = aTexCoord;
}