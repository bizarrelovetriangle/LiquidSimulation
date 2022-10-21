#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <fstream>
#include <sstream>

class GPUProgramBase {
protected:
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
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    }

    uint32_t program_id;
};