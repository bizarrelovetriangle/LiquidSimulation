#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <fstream>
#include <sstream>

class GPUProgramBase {
protected:
    uint32_t CreateShader(const std::string& path, GLenum type) {
        std::string shader_code = readFile(path);
        auto shader_code_cstr = shader_code.c_str();
        auto shader = glCreateShader(type);
        glShaderSource(shader, 1, &shader_code_cstr, NULL);
        glCompileShader(shader);
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        LogErrors(success);
        return shader;
    }

    std::string readFile(const std::string& path) {
        std::stringstream ss;
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(path);
        ss << file.rdbuf();
        return ss.str();
    }

    void LogErrors(bool success) {
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(program_id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::\n" << infoLog << std::endl;
        }
    }

    uint32_t program_id;
};