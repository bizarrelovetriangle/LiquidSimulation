#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <fstream>
#include <sstream>

class DeviceProgram {
public:
	void InitProgram(std::initializer_list<std::pair<GLenum, std::string>> shader_infos) {
		std::vector<uint32_t> shaders;
		for (auto& [type, path] : shader_infos) {
			uint32_t shader = CreateShader(type, path);
			shaders.push_back(shader);
		}

		int success;
		program_id = glCreateProgram();
		for (uint32_t shader : shaders)
			glAttachShader(program_id, shader);
		glLinkProgram(program_id);
		glGetProgramiv(program_id, GL_LINK_STATUS, &success);
		LogErrors(success);
		for (uint32_t shader : shaders)
			glDeleteShader(shader);
	}

	uint32_t program_id;

protected:
	uint32_t CreateShader(GLenum type, const std::string& path) {
		std::string shader_code = readFile(path);
		IncludeFiles(path, shader_code);
		auto shader_code_cstr = shader_code.c_str();
		auto shader = glCreateShader(type);
		glShaderSource(shader, 1, &shader_code_cstr, NULL);
		glCompileShader(shader);
		int success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		LogErrors(success, type);
		return shader;
	}

	void IncludeFiles(const std::string& path, std::string& shader_code) {
		std::string include_start = "#include \"";
		std::string include_end = "\"";

		while (true) {
			size_t start = shader_code.find(include_start);
			if (start == -1) break;
			size_t path_start = start + include_start.length();
			size_t end = shader_code.find(include_end, path_start);
			if (end == -1) break;
			std::string include_path = shader_code.substr(path_start, end - path_start);
			size_t slash_index = path.find_last_of("/");
			if (slash_index >= 0) include_path = std::string(path).erase(slash_index + 1) + include_path;
			std::string include_code = readFile(include_path);
			shader_code = shader_code.erase(start) + include_code + shader_code.substr(end + include_end.length());
		}
	}

	std::string readFile(const std::string& path) {
		std::stringstream ss;
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		file.open(path);
		ss << file.rdbuf();
		return ss.str();
	}

	void LogErrors(bool success, GLenum type = 0) {
		if (success) return;
		char infoLog[512];
		glGetProgramInfoLog(program_id, 512, NULL, infoLog);

		std::string type_str;
		switch (type) {
		case GL_VERTEX_SHADER: type_str = "GL_VERTEX_SHADER"; break;
		case GL_FRAGMENT_SHADER: type_str = "GL_FRAGMENT_SHADER"; break;
		case GL_COMPUTE_SHADER: type_str = "GL_COMPUTE_SHADER"; break;
		}

		std::cout << "ERROR::SHADER::PROGRAM::" << type_str << "\n" << infoLog << std::endl;
	}
};