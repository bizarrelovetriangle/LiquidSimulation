#pragma once
#include "GPUProgramBase.h"

template <typename T>
class RenderProgram : private GPUProgramBase {
public:
	void Init(const std::string& vertex_path, const std::string& fragment_path)
	{
		auto vertex_shader = CreateShader(vertex_path, GL_VERTEX_SHADER);
		auto fragment_shader = CreateShader(fragment_path, GL_FRAGMENT_SHADER);

		int success;
		program_id = glCreateProgram();
		glAttachShader(program_id, vertex_shader);
		glAttachShader(program_id, fragment_shader);
		glLinkProgram(program_id);
		glGetProgramiv(program_id, GL_LINK_STATUS, &success);
		LogErrors(success);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

	void Use() {
		glUseProgram(program_id);
		glBindVertexArray(vao_buffer_id);
	}

	void GenerateVAO(const std::vector<uint32_t>& indexes) {
		glGenVertexArrays(1, &vao_buffer_id);
		glGenBuffers(1, &vbo_buffer_id);
		glGenBuffers(1, &ebo_buffer_id);

		glBindVertexArray(vao_buffer_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_buffer_id);
		glNamedBufferData(ebo_buffer_id, indexes.size() * sizeof(uint32_t), &indexes[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_buffer_id);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
	}

	void UpdateVerteces(const std::vector<vector2>& points) {
		glNamedBufferData(vbo_buffer_id, points.size() * sizeof(vector2), &points[0], GL_DYNAMIC_DRAW);
	}

	void SetViewMatrix(const matrix3x3& matrix) {
		glUniformMatrix3fv(view_matrix_index, 1, GL_FALSE, &matrix.i.x);
	}

	void SetColor(const vector3& color) {
		glUniform4fv(color_index, 1, &color.x);
	}

	void Dispose() {
		glDeleteVertexArrays(1, &vao_buffer_id);
		glDeleteBuffers(1, &vbo_buffer_id);
		glDeleteBuffers(1, &ebo_buffer_id);
	}

private:
	uint32_t vao_buffer_id = 0;
	uint32_t vbo_buffer_id = 0;
	uint32_t ebo_buffer_id = 0;

	uint32_t view_matrix_index = 0;
	uint32_t color_index = 2;
};