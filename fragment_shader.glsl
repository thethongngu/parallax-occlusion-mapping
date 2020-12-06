#version 330 core

in vec3 out_vertex;
in vec3 out_normal;
in vec2 out_texture;

uniform sampler2D texture_data;

out vec4 color;

void main() {
    vec3 color_frag = texture2D(texture_data, out_texture.st).rgb;
    color = vec4(color_frag, 1.0);
}