#include "RenderProgram.h"
#include <DeviceComputation/CommonBuffers.h>
#include <Elements/Wall.h>
#include <Elements/Particle.h>

template <typename T>
void RenderProgram<T>::Use() {
	glUseProgram(program_id);
	glBindVertexArray(vao_buffer_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, CommonBuffers::GetInstance().config->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().pairs->GetBufferId());
}

template <typename T>
void RenderProgram<T>::GenerateVAO(const std::vector<uint32_t>& indexes) {
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

template <typename T>
void RenderProgram<T>::UpdateVerteces(const std::vector<vector2>& points) {
	glNamedBufferData(vbo_buffer_id, points.size() * sizeof(vector2), &points[0], GL_DYNAMIC_DRAW);
}

template <typename T>
void RenderProgram<T>::SetViewMatrix(const matrix3x3& matrix) {
	glUniformMatrix3fv(view_matrix_index, 1, GL_FALSE, (float*)&matrix);
}

template <typename T>
void RenderProgram<T>::SetColor(const vector3& color) {
	glUniform4fv(color_index, 1, (float*)&color);
}

template <typename T>
void RenderProgram<T>::SetIndex(int index) {
	glUniform1i(index_index, index);
}

template <typename T>
void RenderProgram<T>::Dispose() {
	glDeleteVertexArrays(1, &vao_buffer_id);
	glDeleteBuffers(1, &vbo_buffer_id);
	glDeleteBuffers(1, &ebo_buffer_id);
}

template class RenderProgram<Wall>;
template class RenderProgram<Particle>;
