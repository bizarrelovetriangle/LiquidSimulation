#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class DeviceProgram {
public:
	void InitProgram(std::initializer_list<std::pair<GLenum, std::string>> shader_infos);

	uint32_t program_id;

protected:
	uint32_t CreateShader(GLenum type, const std::string& path);
	void IncludeFiles(const std::string& path, std::string& shader_code);
	std::string ReadFile(const std::string& path);
	void LogErrors(bool success, GLenum type = 0);
};