#include "DeviceProgram.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ParticleGrid.h>
#include <DeviceComputation/CommonBuffers.h>

void DeviceProgram::InitProgram(std::initializer_list<std::pair<GLenum, std::string>> shader_infos) {
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

ParticleGrid* DeviceProgram::_particle_grid = nullptr;

void DeviceProgram::Use() const
{
	glUseProgram(program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, CommonBuffers::GetInstance().grid.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, CommonBuffers::GetInstance().particle_indexes.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, CommonBuffers::GetInstance().particles.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, CommonBuffers::GetInstance().particle_threads.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, CommonBuffers::GetInstance().threads_count.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, CommonBuffers::GetInstance().threads.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, CommonBuffers::GetInstance().threads_count_temp.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, CommonBuffers::GetInstance().threads_temp.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, CommonBuffers::GetInstance().threads_torn.GetBufferId());

	int parallel = 200;
	glBindBufferBase(GL_UNIFORM_BUFFER, 10, CommonBuffers::GetInstance().config.GetBufferId());
	if (_particle_grid) glUniform2i(11, _particle_grid->size.x, _particle_grid->size.y);
	glUniform1i(12, parallel);
}


uint32_t DeviceProgram::CreateShader(GLenum type, const std::string& path) {
	std::string shader_code = ReadFile(path);
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

void DeviceProgram::IncludeFiles(const std::string& path, std::string& shader_code) {
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
		std::string include_code = ReadFile(include_path);
		shader_code = shader_code.erase(start) + include_code + shader_code.substr(end + include_end.length());
	}
}

std::string DeviceProgram::ReadFile(const std::string& path) {
	std::stringstream ss;
	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	file.open(path);
	ss << file.rdbuf();
	return ss.str();
}

void DeviceProgram::LogErrors(bool success, GLenum type) {
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