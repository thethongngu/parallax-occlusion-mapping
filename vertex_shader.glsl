#version 330 core

layout (location = 0) in vec3 in_vertex;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 out_vertex;
out vec3 out_normal;
out vec2 out_texture;


void main() {
    gl_Position = projection * view * model * vec4(in_vertex, 1.0);

    out_vertex = (model * vec4(in_vertex, 1.0)).xyz;

    out_normal = (vec4(in_normal, 1.0) * inverse(model)).xyz;
    out_normal = normalize(out_normal);

    out_texture = in_texture;
}
