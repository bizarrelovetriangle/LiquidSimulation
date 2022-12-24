#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class ParticleGrid;

class DeviceProgram {
public:
	void InitProgram(std::initializer_list<std::pair<GLenum, std::string>> shader_infos);
	void Use() const;

	uint32_t program_id;

	static ParticleGrid* _particle_grid;

protected:
	uint32_t CreateShader(GLenum type, const std::string& path);
	void IncludeFiles(const std::string& path, std::string& shader_code);
	std::string ReadFile(const std::string& path);
	void LogErrors(bool success, GLenum type = 0);
};