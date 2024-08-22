#version 460 core

in vec3 vertexPosition;
in vec4 vertexColor;

uniform mat4 mvp;

out vec4 vs_color;

void main() {
    vs_color = vertexColor;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
