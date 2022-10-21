#pragma once
#include "GPUProgramBase.h"

class RenderProgram : private GPUProgramBase {
public:
    void Init(const std::string& vertex_path, const std::string& fragment_path)
    {
        int success;
        std::string vertex_shader_code = readFile(vertex_path);
        auto vertex_shader_code_cstr = vertex_shader_code.c_str();
        auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_code_cstr, NULL);
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
        LogErrors(success);

        std::string fragment_shader_code = readFile(vertex_path);
        auto fragment_shader_code_cstr = fragment_shader_code.c_str();
        auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_code_cstr, NULL);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
        LogErrors(success);

        program_id = glCreateProgram();
        glAttachShader(program_id, vertex_shader);
        glAttachShader(program_id, fragment_shader);
        glLinkProgram(program_id);
        glGetProgramiv(program_id, GL_LINK_STATUS, &success);
        LogErrors(success);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }


};