#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "stb_image.h"
#include "FreeImage.h"



#define PI 3.14159

#define debug(a) std::cerr << #a << " = " << a << std::endl;
#define stop1() std::string a; std::getline(std::cin, a)

std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<glm::vec2> textures;

std::vector<float> vertex_coords, normal_coords, texture_coords, tangent_coords, bitangent_coords;

std::vector<unsigned int> faces;

void load_obj() {
    std::ifstream obj_file;
    obj_file.open("sphere1.obj");

    std::string type;
    while (!obj_file.eof()) {
        obj_file >> type;
        
        if (type == "#") {
            continue;
        }
        else if (type == "v") {
            float x, y, z;
            obj_file >> x >> y >> z;
            vertices.push_back(glm::vec3(x, y, z));

        } else if (type == "vn") {
            float x, y, z;
            obj_file >> x >> y >> z;
            normals.push_back(glm::vec3(x, y, z));

        } else if (type == "vt") {
            float x, y, z;
            obj_file >> x >> y;
            textures.push_back(glm::vec2(x, y));

        } else if (type == "f") {
            std::string raw_data;
            int vertex_index[3], texture_index[3], normal_index[3];
            for(int i = 0 ; i < 3; i++) {
                obj_file >> raw_data;
                auto slash_st = raw_data.find('/');
                auto slash_nd = raw_data.find('/', slash_st + 1);

                vertex_index[i] = atoi(raw_data.substr(0, slash_st).c_str());
                vertex_coords.push_back(vertices[vertex_index[i] - 1].x);
                vertex_coords.push_back(vertices[vertex_index[i] - 1].y);
                vertex_coords.push_back(vertices[vertex_index[i] - 1].z);

                texture_index[i] = atoi(raw_data.substr(slash_st + 1, slash_nd - slash_st - 1).c_str());
                texture_coords.push_back(textures[texture_index[i] - 1].x);
                texture_coords.push_back(textures[texture_index[i] - 1].y);

                normal_index[i] = atoi(raw_data.substr(slash_nd + 1).c_str());
                normal_coords.push_back(normals[normal_index[i] - 1].x);
                normal_coords.push_back(normals[normal_index[i] - 1].y);
                normal_coords.push_back(normals[normal_index[i] - 1].x);

            }

            glm::vec3 tangent1, bitangent1;

            glm::vec3 edge1 = vertices[vertex_index[1] - 1] - vertices[vertex_index[0] - 1];
            glm::vec3 edge2 = vertices[vertex_index[2] - 1] - vertices[vertex_index[0] - 1];
            glm::vec2 deltaUV1 = textures[texture_index[1] - 1] - textures[texture_index[0] - 1];
            glm::vec2 deltaUV2 = textures[texture_index[2] - 1] - textures[texture_index[0] - 1];

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
            tangent1 = glm::normalize(tangent1);

            bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
            bitangent1 = glm::normalize(bitangent1);

            tangent_coords.push_back(tangent1.x);
            tangent_coords.push_back(tangent1.y);
            tangent_coords.push_back(tangent1.z);
            tangent_coords.push_back(tangent1.x);
            tangent_coords.push_back(tangent1.y);
            tangent_coords.push_back(tangent1.z);
            tangent_coords.push_back(tangent1.x);
            tangent_coords.push_back(tangent1.y);
            tangent_coords.push_back(tangent1.z);

            bitangent_coords.push_back(bitangent1.x);
            bitangent_coords.push_back(bitangent1.y);
            bitangent_coords.push_back(bitangent1.z);
            bitangent_coords.push_back(bitangent1.x);
            bitangent_coords.push_back(bitangent1.y);
            bitangent_coords.push_back(bitangent1.z);
            bitangent_coords.push_back(bitangent1.x);
            bitangent_coords.push_back(bitangent1.y);
            bitangent_coords.push_back(bitangent1.z);
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

unsigned int load_texture_stb(char const * path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

int main(void) {
    GLFWwindow* window;
    if (!glfwInit()) exit(EXIT_FAILURE);
    FreeImage_Initialise(true);

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );

    window = glfwCreateWindow(1024, 768, "Project 02", NULL, NULL);
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

    GLint eye_point_shader = glGetUniformLocation(shader_program, "eye_pos");
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

    GLuint vbo_tangent;
    glGenBuffers(1, &vbo_tangent);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_tangent);
    glBufferData(GL_ARRAY_BUFFER, tangent_coords.size() * sizeof(float), tangent_coords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLuint vbo_bitangent;
    glGenBuffers(1, &vbo_bitangent);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_bitangent);
    glBufferData(GL_ARRAY_BUFFER, bitangent_coords.size() * sizeof(float), bitangent_coords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    
    // Setup texture
    GLuint texture_id, normal_id, heightmap_id;
    load_texture(normal_id, 1, "rock_normal.jpg", FIF_JPEG);
    load_texture(heightmap_id, 2, "rock_height.png", FIF_PNG);
    load_texture(texture_id, 3, "rock.jpg", FIF_JPEG);

    // unsigned int normal_id = load_texture_stb("rock_normal.jpg");
    // unsigned int heightmap_id = load_texture_stb("rock_height.png");
    // unsigned int texture_id = load_texture_stb("rock.jpg");
    // glUniform1i(glGetUniformLocation(shader_program, "texture_map"), 0); 
    // glUniform1i(glGetUniformLocation(shader_program, "normal_map"), 1); 
    // glUniform1i(glGetUniformLocation(shader_program, "height_map"), 2); 

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
    glFrontFace(GL_CCW);
    
    // Main loop
    float degree = 0;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        degree = fmod(degree + 0.5, 360);
        eye = glm::vec3(cos(glm::radians(degree)) * 20, sin(glm::radians(degree)) * 20, 0);
        glm::mat4 view = glm::lookAt(eye,  glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

        glUniformMatrix4fv(view_shader, 1, GL_FALSE, &view[0][0]);
        glUniform3fv(eye_point_shader, 1, &eye[0]);

        // stop1();

        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, texture_id);
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, normal_id);
        // glActiveTexture(GL_TEXTURE2);
        // glBindTexture(GL_TEXTURE_2D, heightmap_id);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertex_coords.size());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}