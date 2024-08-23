#version 460 core

in vec3 vertexPosition;
in vec4 vertexColor;

uniform mat4 matModel;
uniform mat4 matView;

uniform mat4 u_mat_proj;

out vec4 vs_color;

void main() {
    vs_color = vertexColor;

    vec4 world_pos = matModel * vec4(vertexPosition, 1.0);

    gl_Position = u_mat_proj * matView * world_pos;
}
