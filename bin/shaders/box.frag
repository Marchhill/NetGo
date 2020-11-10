#version 460 core

out vec4 FragColor;

in vec2 texCoord;

uniform vec4 col;
uniform sampler2D tex;

void main() {
    FragColor = (col.w == 0.0) ? texture(tex, texCoord) : col;
}