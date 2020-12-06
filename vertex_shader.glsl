#version 330 core

layout (location = 0) in vec3 in_vertex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 out_vertex;


void main() {
    out_vertex = in_vertex;
    gl_Position = projection * view * model * vec4(in_vertex, 1.0);
}
