#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "FreeImage.h"

#define debug(a) std::cerr << #a << " = " << a << std::endl;

std::vector<uint32_t> vertex_ind, texture_ind, normal_ind;
std::vector<float> vertices, normals, textures;
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
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

        } else if (type == "vn") {
            float x, y, z;
            obj_file >> x >> y >> z;
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);

        } else if (type == "vt") {
            float x, y, z;
            obj_file >> x >> y >> z;
            textures.push_back(x);
            textures.push_back(y);

        } else if (type == "f") {
            std::string raw_data;
            for(int i = 0 ; i < 4; i++) {
                obj_file >> raw_data;
                auto vertex = atoi(raw_data.substr(0, raw_data.find('/')).c_str());
                faces.push_back(vertex - 1);
            }
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

int main(void) {
    GLFWwindow* window;
    if (!glfwInit()) exit(EXIT_FAILURE);

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

    // Setup VAO
    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Setup VBO 
    GLuint vbo_vertices;
    glGenBuffers(1, &vbo_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glEnableVertexAttribArray(0);

    GLuint vbo_textures;
    glGenBuffers(1, &vbo_textures);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_textures);
    glBufferData(GL_ARRAY_BUFFER, textures.size() * sizeof(float), &textures[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    glEnableVertexAttribArray(1);

    GLuint vbo_normals;
    glGenBuffers(1, &vbo_normals);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glEnableVertexAttribArray(2);

    // Setup texture image
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    std::string texture_path = "rock.jpg";
    FIBITMAP *texImage = FreeImage_ConvertTo24Bits(FreeImage_Load(FIF_JPEG, texture_path.c_str()));
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FreeImage_GetWidth(texImage), FreeImage_GetHeight(texImage), 0, GL_RGB, GL_UNSIGNED_BYTE, (void *)FreeImage_GetBits(texImage));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    FreeImage_Unload(texImage);

    GLuint index;
    glGenBuffers(1, &index);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(GLuint), &faces[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);

    // Setup transformation matrix
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 30),  glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f);    
    glUniformMatrix4fv(model_shader, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(view_shader, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projection_shader, 1, GL_FALSE, &projection[0][0]);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_TRIANGLE_FAN, faces.size(), GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}