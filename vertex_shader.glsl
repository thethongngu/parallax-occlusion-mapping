#version 400 core

layout (location = 0) in vec3 in_vertex;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_pos;
uniform vec3 eye_pos;

out vec3 out_vertex;
out vec3 out_normal;
out vec2 out_texture;

void main() {
    vs_out.FragPos = vec3(model * vec4(in_vertex, 1.0));   
    vs_out.TexCoords = in_texture;   
    
    // vec3 T = normalize(mat3(model) * in_tangent);
    // vec3 B = normalize(mat3(model) * in_bitangent);
    // vec3 N = normalize(mat3(model) * in_normal);
    // mat3 TBN = transpose(mat3(T, B, N));

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * in_tangent);
    vec3 N = normalize(normalMatrix * in_normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * light_pos;
    vs_out.TangentViewPos  = TBN * eye_pos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    
    gl_Position = projection * view * model * vec4(in_vertex, 1.0);
}