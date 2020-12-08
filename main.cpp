#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "FreeImage.h"

#define PI 3.14159

#define debug(a) std::cerr << #a << " = " << a << std::endl;
#define stop1() std::string a; std::cin >> a;

struct Vertex {
    float x, y, z;
    Vertex(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct Normal {
    float x, y, z;
    Normal(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct Texture {
    float x, y;
    Texture(float x, float y) : x(x), y(y) {}
};

std::vector<Vertex> vertices;
std::vector<Normal> normals;
std::vector<Texture> textures;

std::vector<float> vertex_coords, normal_coords, texture_coords;

std::vector<unsigned int> faces;

void load_obj() {
    std::ifstream obj_file;
    obj_file.open("Sphere.obj");

    std::string type;
    while (!obj_file.eof()) {
        obj_file >> type;
        if (type == "v") {
            float x, y, z;
            obj_file >> x >> y >> z;
            vertices.emplace_back(x, y, z);

        } else if (type == "vn") {
            float x, y, z;
            obj_file >> x >> y >> z;
            normals.emplace_back(x, y, z);

        } else if (type == "vt") {
            float x, y, z;
            obj_file >> x >> y >> z;
            textures.emplace_back(x, y);

        } else if (type == "f") {
            std::string raw_data;
            int vertex_index[4], texture_index[4], normal_index[4];
            for(int i = 0 ; i < 4; i++) {
                obj_file >> raw_data;
                auto slash_st = raw_data.find('/');
                auto slash_nd = raw_data.find('/', slash_st + 1);
                vertex_index[i] = atoi(raw_data.substr(0, slash_st).c_str());
                texture_index[i] = atoi(raw_data.substr(slash_st + 1, slash_nd - slash_st - 1).c_str());
                normal_index[i] = atoi(raw_data.substr(slash_nd + 1).c_str());
            }

            vertex_coords.push_back(vertices[vertex_index[0] - 1].x);
            vertex_coords.push_back(vertices[vertex_index[0] - 1].y);
            vertex_coords.push_back(vertices[vertex_index[0] - 1].z);
            vertex_coords.push_back(vertices[vertex_index[1] - 1].x);
            vertex_coords.push_back(vertices[vertex_index[1] - 1].y);
            vertex_coords.push_back(vertices[vertex_index[1] - 1].z);
            vertex_coords.push_back(vertices[vertex_index[2] - 1].x);
            vertex_coords.push_back(vertices[vertex_index[2] - 1].y);
            vertex_coords.push_back(vertices[vertex_index[2] - 1].z);

            vertex_coords.push_back(vertices[vertex_index[0] - 1].x);
            vertex_coords.push_back(vertices[vertex_index[0] - 1].y);
            vertex_coords.push_back(vertices[vertex_index[0] - 1].z);
            vertex_coords.push_back(vertices[vertex_index[2] - 1].x);
            vertex_coords.push_back(vertices[vertex_index[2] - 1].y);
            vertex_coords.push_back(vertices[vertex_index[2] - 1].z);
            vertex_coords.push_back(vertices[vertex_index[3] - 1].x);
            vertex_coords.push_back(vertices[vertex_index[3] - 1].y);
            vertex_coords.push_back(vertices[vertex_index[3] - 1].z);

            normal_coords.push_back(normals[normal_index[0] - 1].y);
            normal_coords.push_back(normals[normal_index[0] - 1].x);
            normal_coords.push_back(normals[normal_index[0] - 1].z);
            normal_coords.push_back(normals[normal_index[1] - 1].x);
            normal_coords.push_back(normals[normal_index[1] - 1].y);
            normal_coords.push_back(normals[normal_index[1] - 1].z);
            normal_coords.push_back(normals[normal_index[2] - 1].x);
            normal_coords.push_back(normals[normal_index[2] - 1].y);
            normal_coords.push_back(normals[normal_index[2] - 1].z);

            normal_coords.push_back(normals[normal_index[0] - 1].y);
            normal_coords.push_back(normals[normal_index[0] - 1].x);
            normal_coords.push_back(normals[normal_index[0] - 1].z);
            normal_coords.push_back(normals[normal_index[2] - 1].x);
            normal_coords.push_back(normals[normal_index[2] - 1].y);
            normal_coords.push_back(normals[normal_index[2] - 1].z);
            normal_coords.push_back(normals[normal_index[3] - 1].x);
            normal_coords.push_back(normals[normal_index[3] - 1].y);
            normal_coords.push_back(normals[normal_index[3] - 1].z);

            texture_coords.push_back(textures[texture_index[0] - 1].x);
            texture_coords.push_back(textures[texture_index[0] - 1].y);
            texture_coords.push_back(textures[texture_index[1] - 1].x);
            texture_coords.push_back(textures[texture_index[1] - 1].y);
            texture_coords.push_back(textures[texture_index[2] - 1].x);
            texture_coords.push_back(textures[texture_index[2] - 1].y);

            texture_coords.push_back(textures[texture_index[0] - 1].x);
            texture_coords.push_back(textures[texture_index[0] - 1].y);
            texture_coords.push_back(textures[texture_index[2] - 1].x);
            texture_coords.push_back(textures[texture_index[2] - 1].y);
            texture_coords.push_back(textures[texture_index[3] - 1].x);
            texture_coords.push_back(textures[texture_index[3] - 1].y);

            // faces.push_back(vertex_index[0] - 1);
            // faces.push_back(vertex_index[1] - 1);
            // faces.push_back(vertex_index[2] - 1);

            // faces.push_back(texture_index[0] - 1);
            // faces.push_back(texture_index[1] - 1);
            // faces.push_back(texture_index[2] - 1);

            // faces.push_back(normal_index[0] - 1);
            // faces.push_back(normal_index[1] - 1);
            // faces.push_back(normal_index[2] - 1);

            // faces.push_back(vertex_index[0] - 1);
            // faces.push_back(vertex_index[2] - 1);
            // faces.push_back(vertex_index[3] - 1);

            // faces.push_back(texture_index[0] - 1);
            // faces.push_back(texture_index[2] - 1);
            // faces.push_back(texture_index[3] - 1);

            // faces.push_back(normal_index[0] - 1);
            // faces.push_back(normal_index[2] - 1);
            // faces.push_back(normal_index[3] - 1);
        }
    }

    obj_file.close();
}

void compile_shader(GLuint type, std::string path, GLuint& shader_program) {
    GLuint shader_id = glCreateShader(type);
    std::ifstream file(path);
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    const char* source_c_str = source.c_str();
    glShaderSource(shader_id, 1, &source_c_str, nullptr);
    glCompileShader(shader_id);

    int status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        int log_length;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
        char* error = (char*) alloca(log_length * sizeof(char));

        glGetShaderInfoLog(shader_id, log_length, &log_length, error);
        std::cout << "Failed to compile " << ( type == GL_VERTEX_SHADER ? "vertex" : "fragment" ) << "shader" << std::endl;
        std::cout << error << std::endl;
        glDeleteShader(shader_id);
    }

    glAttachShader(shader_program, shader_id);
    glLinkProgram(shader_program);
    glValidateProgram(shader_program);
    glDeleteShader(shader_id);
}

void load_texture(GLuint &tbo, int tex_unit, const std::string tex_path, FREE_IMAGE_FORMAT img_type) {
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    FIBITMAP *tex_img = FreeImage_ConvertTo24Bits(FreeImage_Load(img_type, tex_path.c_str()));

    glGenTextures(1, &tbo);
    glBindTexture(GL_TEXTURE_2D, tbo);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, FreeImage_GetWidth(tex_img),
        FreeImage_GetHeight(tex_img), 0, GL_BGR, GL_UNSIGNED_BYTE,
        (void *)FreeImage_GetBits(tex_img)
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    FreeImage_Unload(tex_img);
}

int main(void) {
    GLFWwindow* window;
    if (!glfwInit()) exit(EXIT_FAILURE);
    FreeImage_Initialise(true);

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );

    window = glfwCreateWindow(800, 600, "Project 02", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glewInit();
    
    load_obj();

    // Compile shader
    GLuint shader_program = glCreateProgram();
    compile_shader(GL_VERTEX_SHADER, "vertex_shader.glsl", shader_program);
    compile_shader(GL_FRAGMENT_SHADER, "fragment_shader.glsl", shader_program);
    glUseProgram(shader_program);

    GLint model_shader = glGetUniformLocation(shader_program, "model");
    GLint view_shader = glGetUniformLocation(shader_program, "view");
    GLint projection_shader = glGetUniformLocation(shader_program, "projection");

    GLint texture_shader = glGetUniformLocation(shader_program, "texture_data");
    GLint normal_shader = glGetUniformLocation(shader_program, "normal_data");
    GLint heightmap_shader = glGetUniformLocation(shader_program, "heightmap_data");

    GLint eye_point_shader = glGetUniformLocation(shader_program, "eye_point");
    GLint light_pos_shader = glGetUniformLocation(shader_program, "light_pos");

    // Setup VAO
    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Setup VBO 
    GLuint vbo_vertices;
    glGenBuffers(1, &vbo_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, vertex_coords.size() * sizeof(GLfloat), vertex_coords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLuint vbo_normals;
    glGenBuffers(1, &vbo_normals);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
    glBufferData(GL_ARRAY_BUFFER, normal_coords.size() * sizeof(float), normal_coords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLuint vbo_textures;
    glGenBuffers(1, &vbo_textures);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_textures);
    glBufferData(GL_ARRAY_BUFFER, texture_coords.size() * sizeof(float), texture_coords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    
    // Setup texture
    GLuint texture_id, normal_id, heightmap_id;
    load_texture(normal_id, 1, "rock_normal.jpg", FIF_JPEG);
    load_texture(heightmap_id, 2, "rock_height.png", FIF_PNG);
    load_texture(texture_id, 3, "rock.jpg", FIF_JPEG);

    // Setup transformation matrix
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 eye = glm::vec3(0, 20, 20);
    glm::mat4 view = glm::lookAt(eye,  glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f);    
    glm::vec3 light_pos = glm::vec3(0, 0, 50);

    // Init data for shader
    glUniformMatrix4fv(model_shader, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(projection_shader, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(view_shader, 1, GL_FALSE, &view[0][0]);
    glUniform1i(normal_shader, normal_id);
    glUniform1i(heightmap_shader, heightmap_id);
    glUniform1i(texture_shader, texture_id);
    glUniform3fv(eye_point_shader, 1, &eye[0]);
    glUniform3fv(light_pos_shader, 1, &light_pos[0]);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);  
    
    // Main loop
    float degree = 0.5;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        degree = fmod(degree, 360);
        model = glm::rotate(model, glm::radians(degree), glm::vec3(1, 0, 0));

        glUniformMatrix4fv(model_shader, 1, GL_FALSE, &model[0][0]);

        // stop1();

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertex_coords.size());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}